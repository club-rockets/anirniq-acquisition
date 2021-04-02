#include "mti.h"
#include "main.h"
#include "board.h"
#include "mtssp_interface.h"
#include "xbusmessageid.h"
#include "wait.h"
#include "xbusdef.h"
#include "xbustostring.h"
#include "xbushelpers.h"

#include <string.h>

static XbusMessage msg;
static uint8_t xbusMessage[2056];

static void mti_error(){

#if (configTRANSCRIPT_ENABLED)
	transcript(APP_MTI_NAME, "Something went wrong - Suspend task", CERROR);
#endif

	while(1); //Stop the task
}

static void mti_waitForBoot(void){

	//Wait for the MTI too boot
	while(HAL_GPIO_ReadPin(DATA_READY_PORT, DATA_READY_PIN) == GPIO_PIN_SET);
	while(HAL_GPIO_ReadPin(DATA_READY_PORT, DATA_READY_PIN) != GPIO_PIN_SET);

}

static void mti_send(uint8_t msgId){

	XbusMessageCreate(&msg,msgId);
	MtsspInterface_sendXbusMessage(&msg);
	wait_us(5); //Wait for next transfert

}

/*!	\brief Resets the MTi
*/
void resetDevice(void)
{
	HAL_GPIO_WritePin(RESET_PORT, RESET_PIN, GPIO_PIN_SET);
	wait_us(5000);
	HAL_GPIO_WritePin(RESET_PORT, RESET_PIN, GPIO_PIN_RESET);
}

void config_mti(void){

	uint8_t* xbusMsg = &xbusMessage[2];
	uint16_t notificationMessageSize;
	uint16_t measurementMessageSize;

	resetDevice();

	//Wait for data ready
	mti_waitForBoot();

	MtsspInterface_readPipeStatus(&notificationMessageSize, &measurementMessageSize); //Read pipe status
	wait_us(5); //Wait for next transfert

	if(notificationMessageSize > 0){

		MtsspInterface_readFromPipe(xbusMsg, notificationMessageSize, XBUS_NOTIFICATION_PIPE);

		if(getMessageId(xbusMessage) == XMID_Wakeup){

			mti_send(XMID_WakeupAck);

		}else{

#if (configTRANSCRIPT_ENABLED)
			transcript(APP_MTI_NAME,"Message received was not XMID_Wakeup",CERROR);
			transcript(APP_MTI_NAME,xbusToString(xbusMessage),CERROR);
#endif
			mti_error();

		}
	}

	//Check if in config state
	mti_send(XMID_GotoConfig);

	//Wait for dataReady
	while(HAL_GPIO_ReadPin(DATA_READY_PORT, DATA_READY_PIN) != GPIO_PIN_SET);

	//Read the pipe status
	MtsspInterface_readPipeStatus(&notificationMessageSize, &measurementMessageSize);
	wait_us(5); //Wait for next transfert

	if(notificationMessageSize > 0){

				//Read the wake up message (XMID_Wakeup)
				MtsspInterface_readFromPipe(xbusMessage+2, notificationMessageSize, XBUS_NOTIFICATION_PIPE);
				wait_us(5); //Wait for next transfert

				if(getMessageId(xbusMessage) != XMID_GotoConfigAck){

	#if (configTRANSCRIPT_ENABLED)
					transcript(APP_MTI_NAME,"Message received was not XMID_GotoConfigAck",CERROR);
					transcript(APP_MTI_NAME,xbusToString(xbusMessage),CERROR);
	#endif
					mti_error();
				}

			}else{
	#if (configTRANSCRIPT_ENABLED)
				transcript(APP_MTI_NAME,"No notification message",CERROR);
	#endif
				mti_error();
			}


}

void task_mti(void * pvParameters){

	for(;;){



	}


}

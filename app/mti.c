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
static uint8_t xbusMessage[500];
static mti_device_state mtiState = 0;

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

void mti_receive(uint8_t msgId){

	uint16_t notificationMessageSize;
	uint16_t measurementMessageSize;

	//Wait for message acknowledge
	//Wait for dataReady
	while(HAL_GPIO_ReadPin(DATA_READY_PORT, DATA_READY_PIN) != GPIO_PIN_SET);


	//Read the pipe status
	MtsspInterface_readPipeStatus(&notificationMessageSize, &measurementMessageSize);
	wait_us(5); //Wait for next transfert

	if(notificationMessageSize > 0){

				//Read the wake up message (XMID_Wakeup)
				MtsspInterface_readFromPipe(xbusMessage+2, notificationMessageSize, XBUS_NOTIFICATION_PIPE);
				wait_us(5); //Wait for next transfert

				if(getMessageId(xbusMessage) != msgId){

	#if (configTRANSCRIPT_ENABLED)
					transcript(APP_MTI_NAME,"Message received was not expected",CERROR);
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

void mti_send(uint8_t msgId){

	XbusMessageCreate(&msg,msgId);
	MtsspInterface_sendXbusMessage(&msg);
	wait_us(5); //Wait for next transfert

}

/*!	\brief Resets the MTi
*/
void resetDevice(void)
{
	HAL_GPIO_WritePin(RESET_PORT, RESET_PIN, GPIO_PIN_RESET);
	wait_us(5000);
	HAL_GPIO_WritePin(RESET_PORT, RESET_PIN, GPIO_PIN_SET);
	mtiState = 0; //Reset device state
}

void config_mti(void){

	//uint8_t* xbusMsg = &xbusMessage[2];
	mtiConfiguration mtiConfig;

	resetDevice();

	//Wait for data ready
	mti_waitForBoot();

	/* RECEIVE DEVICE ACKNOWLEDGE */

	mti_receive(XMID_Wakeup);
	mti_send(XMID_WakeupAck);

	/* GO TO CONFIG STATE */

	//Check if in config state
	mti_send(XMID_GotoConfig);
	mti_receive(XMID_GotoConfigAck);

	mtiState = mti_configuration; //mti is now in configuration mode

	/* FETCH MTI CONFIGURATION */

	mti_send(XMID_ReqConfiguration);
	mti_receive(XMID_Configuration); //Expected 118 bytes

	memcpy(&mtiConfig, getPointerToPayload(xbusMessage), getPayloadLength(xbusMessage)); //Copy configuration of xbusMessage

#if (configTRANSCRIPT_ENABLED)
			transcript(APP_MTI_NAME,"Mti is in configuration mode",0);
#endif

	//Setup OutputConfiguration

}

void task_mti(void * pvParameters){

	uint16_t notificationMessageSize;
	uint16_t measurementMessageSize;

	//Go to measurement mode
	mti_send(XMID_GotoMeasurement);
	mti_receive(XMID_GotoMeasurementAck);

	mtiState = mti_measurement; //If all is ok

#if (configTRANSCRIPT_ENABLED)
			transcript(APP_MTI_NAME,"Mti is in measurement mode",0);
#endif

	//Create binary semaphore for DRDY interrupt request
	xSemaphoreDRDY = xSemaphoreCreateBinaryStatic( &xSemaphoreDRDYBuffer );
	configASSERT( xSemaphoreDRDY );

	//Enable the interrupt service request from DRDY
	HAL_NVIC_EnableIRQ(EXTI4_IRQn);

	for(;;){

		//Check Rocket Status

		//Wait for DRDY go high
		if( xSemaphoreTake( xSemaphoreDRDY, portMAX_DELAY ) == pdTRUE ){

			//Read notification and measurement data
			MtsspInterface_readPipeStatus(&notificationMessageSize, &measurementMessageSize);

			//Check pipe
			if( (!notificationMessageSize) && (measurementMessageSize > 0) ){
				MtsspInterface_readFromPipe(xbusMessage+2, measurementMessageSize, XBUS_MEASUREMENT_PIPE);
			} //else create a handle for notificationMessage
		}
	}
}

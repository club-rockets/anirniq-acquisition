#include "mti.h"

#include <string.h>
#include <stdio.h>

#define APP_MTI_NAME "MTI"

static uint8_t xbusMessage[500];
static mti_device_state mtiState = 0;
static volatile mtData2 mtiData;
//static char g_textBuffer[256];
//static uint8_t test = 0;

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

	XbusMessage msg;

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
	while(HAL_GPIO_ReadPin(DATA_READY_PORT, DATA_READY_PIN) != GPIO_PIN_SET);
	mti_receive(XMID_GotoConfigAck);

	mtiState = mti_configuration; //mti is now in configuration mode

	/* FETCH MTI CONFIGURATION */

	mti_send(XMID_ReqConfiguration);
	while(HAL_GPIO_ReadPin(DATA_READY_PORT, DATA_READY_PIN) != GPIO_PIN_SET);
	mti_receive(XMID_Configuration); //Expected 118 bytes

	memcpy(&mtiConfig, getPointerToPayload(xbusMessage), getPayloadLength(xbusMessage)); //Copy configuration of xbusMessage

#if (configTRANSCRIPT_ENABLED)
			transcript(APP_MTI_NAME,"Mti is in configuration mode",0);
#endif

	//Setup OutputConfiguration


}

uint8_t mti_mtData2_parse(XbusMessage msg){

	uint16_t dataId;
	uint8_t dataSize;
	uint8_t dataIndex = 0;

	//Check if it is from MTData 2
	if(msg.m_mid != XMID_MtData2){
#if (configTRANSCRIPT_ENABLED)
		transcript(APP_MTI_NAME,"Not MtData2 id",CERROR);
#endif
		return 0;
	}

	//Loop
	while(dataIndex < msg.m_length){

		//Read message id from pointer
		dataId = readUint16(msg.m_data, &dataIndex);
		dataSize = readUint8(msg.m_data, &dataIndex);

		switch(dataId){

			case XDI_PacketCounter:

				if(dataSize != sizeof(mtiData.xdiPacketCounter)) break; //Check datasize

				mtiData.xdiPacketCounter = readUint16(msg.m_data, &dataIndex);

				break;

			case XDI_Acceleration:

				if(dataSize != sizeof(mtiData.xdiAcceleration)) break;

				mtiData.xdiAcceleration.x = readFloat(msg.m_data, &dataIndex);
				mtiData.xdiAcceleration.y = readFloat(msg.m_data, &dataIndex);
				mtiData.xdiAcceleration.z = readFloat(msg.m_data, &dataIndex);

				sd_writeFloat("Acc X",mtiData.xdiAcceleration.x);
				sd_writeFloat("Acc Y",mtiData.xdiAcceleration.y);
				sd_writeFloat("Acc Z",mtiData.xdiAcceleration.z);

				break;

			case XDI_Temperature:

				if(dataSize != sizeof(mtiData.xdiTemperature)) break;

				mtiData.xdiTemperature = readFloat(msg.m_data, &dataIndex);

				sd_writeFloat("Temp",mtiData.xdiTemperature);

				break;

			default:

				dataIndex += dataSize;
				break;
		}
	}
	return 1;
}

void task_mti(void * pvParameters){

	XbusMessage msg;
	uint16_t notificationMessageSize;
	uint16_t measurementMessageSize;

	//Enable the interrupt service request from DRDY
	HAL_NVIC_EnableIRQ(EXTI4_IRQn);

	//Go to measurement mode
	mti_send(XMID_GotoMeasurement);
	mtiState = mti_measurement; //If all is ok

#if (configTRANSCRIPT_ENABLED)
			transcript(APP_MTI_NAME,"Mti is in measurement mode",0);
			transcript(APP_MTI_NAME,"Launching MTI task",0);
#endif

	for(;;){

		//Check Rocket Status
		HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin,0);

		//Wait for DRDY go high
		if( xSemaphoreTake( xSemaphoreDRDY, portMAX_DELAY ) == pdTRUE ){

			HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin,1);
			//Read notification and measurement data
			MtsspInterface_readPipeStatus(&notificationMessageSize, &measurementMessageSize);

			if( (notificationMessageSize > 0) ){

				MtsspInterface_readFromPipe(xbusMessage+2, notificationMessageSize, XBUS_NOTIFICATION_PIPE);

				//Parse message information
				msg.m_mid = getMessageId(xbusMessage);
				msg.m_length = getPayloadLength(xbusMessage);
				msg.m_data = getPointerToPayload(xbusMessage);

				//Use case from error that can occur

			}

			//Check pipe
			if( (measurementMessageSize > 0) ){

				MtsspInterface_readFromPipe(xbusMessage+2, measurementMessageSize, XBUS_MEASUREMENT_PIPE); //Read from the pipe

				//Parse message information
				msg.m_mid = getMessageId(xbusMessage);
				msg.m_length = getPayloadLength(xbusMessage);
				msg.m_data = getPointerToPayload(xbusMessage);

				//Send Data to sd card


				//Parse data
				mti_mtData2_parse(msg);

			}

			} //else create a handle for notificationMessage
		}
	}

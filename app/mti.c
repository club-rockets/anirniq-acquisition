#include "mti.h"

#include <string.h>
#include <stdio.h>

#define APP_MTI_NAME "MTI"

static uint8_t xbusMessage[500];
static mti_device_state mtiState = 0;
static volatile mtData2 mtiData;
//static char g_textBuffer[256];
static volatile uint8_t test = 0;
static volatile uint8_t dataNumber = 0;

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

	xbusMessage[1] = 0xff;
	//uint8_t* xbusMsg = &xbusMessage[2];
	mtiConfiguration mtiConfig;

	resetDevice();

	//Wait for data ready
	mti_waitForBoot();

	/* RECEIVE DEVICE ACKNOWLEDGE */

	mti_receive(XMID_Wakeup);

	if(verifyChecksum(xbusMessage))
		mti_send(XMID_WakeupAck);
	else mti_error();


	/* GO TO CONFIG STATE */

	//Check if in config state
	mti_send(XMID_GotoConfig);
	while(HAL_GPIO_ReadPin(DATA_READY_PORT, DATA_READY_PIN) != GPIO_PIN_SET);
	mti_receive(XMID_GotoConfigAck);

	if(!verifyChecksum(xbusMessage)) mti_error();

	mtiState = mti_configuration; //mti is now in configuration mode

	/* FETCH MTI CONFIGURATION */

	mti_send(XMID_ReqConfiguration);
	while(HAL_GPIO_ReadPin(DATA_READY_PORT, DATA_READY_PIN) != GPIO_PIN_SET);
	mti_receive(XMID_Configuration); //Expected 118 bytes

	if(!verifyChecksum(xbusMessage)) mti_error();

	memcpy(&mtiConfig, getPointerToPayload(xbusMessage), getPayloadLength(xbusMessage)); //Copy configuration of xbusMessage

#if (configTRANSCRIPT_ENABLED)
			transcript(APP_MTI_NAME,"Mti is in configuration mode",0);
#endif

	//Setup OutputConfiguration
	if(!mti_mtData2_configure()) return;

}

/***********************
 * MTI API
 ***********************/

/* FUNCTION TO CREATE */

static void read_packet(uint8_t size, uint8_t* ptr, uint8_t* prm){

	if(size != sizeof(mtiData.xdiPacketCounter)) return; //Check datasize
	mtiData.xdiPacketCounter = readUint16(ptr, prm);

	sd_writeUint("packet",mtiData.xdiPacketCounter);

}

static void read_acceleration(uint8_t size, uint8_t* ptr, uint8_t* prm){

	if(size != sizeof(mtiData.xdiAcceleration)) return;
	mtiData.xdiAcceleration.x = readFloat(ptr, prm);
	mtiData.xdiAcceleration.y = readFloat(ptr, prm);
	mtiData.xdiAcceleration.z = readFloat(ptr, prm);

	sd_writeFloat("x",mtiData.xdiAcceleration.x);
	sd_writeFloat("y",mtiData.xdiAcceleration.y);
	sd_writeFloat("z",mtiData.xdiAcceleration.z);

}

static void read_temperature(uint8_t size, uint8_t* ptr, uint8_t* prm){

	if(size != sizeof(mtiData.xdiTemperature)) return;
	mtiData.xdiTemperature = readFloat(ptr, prm);

	sd_writeFloat("temp",mtiData.xdiTemperature);

}

/* DATA STRUCTURE TO FILL */

mti_api mti_data[] = {

		XDI_PacketCounter, 0xffff , "NPacket", read_packet,
		XDI_Acceleration, 20 , "Accel", read_acceleration,
		XDI_Temperature, 20 , "Temperature", read_temperature
};

/***********************
 * END OF MTI API
 ***********************/

uint8_t mti_mtData2_configure(void){

	uint8_t i = 0;
	uint8_t nb = 0;

	//See how many data we have
	nb = sizeof(mti_data)/28;

	uint8_t* payload = getPointerToPayload(xbusMessage);

	//Add first row
	for(i = 0; i< nb; i++){

		payload[4*i] = mti_data[i].id >> 8;
		payload[(4*i)+1] = mti_data[i].id;
		payload[(4*i)+2] = mti_data[i].frequency >> 8;
		payload[(4*i)+3] = mti_data[i].frequency;

	}

	XbusMessage msg;

	msg.m_mid = XMID_SetOutputConfiguration;
	msg.m_length = 4*nb;
	msg.m_data = payload;

	MtsspInterface_sendXbusMessage(&msg);
	while(HAL_GPIO_ReadPin(DATA_READY_PORT, DATA_READY_PIN) != GPIO_PIN_SET);
	mti_receive(XMID_SetOutputConfigurationAck);

#if (configTRANSCRIPT_ENABLED)
			transcript(APP_MTI_NAME,"Output configuration sent",0);
#endif

	return 1;
}

uint8_t mti_mtData2_parse(XbusMessage msg){

	uint16_t dataId;
	uint8_t dataSize;
	uint8_t dataIndex = 0;
	uint8_t i = 0;

	//Check if it is from MTData 2
	if(msg.m_mid != XMID_MtData2){
#if (configTRANSCRIPT_ENABLED)
		transcript(APP_MTI_NAME,"Not MtData2 id",CERROR);
#endif
		return 0;
	}

	//Loop to check all data
	while(dataIndex < msg.m_length){

		//Read message id from pointer
		dataId = readUint16(msg.m_data, &dataIndex);
		dataSize = readUint8(msg.m_data, &dataIndex);

		//Check all data
		if(mti_data[i].id == dataId){

			mti_data[i].callback(dataSize, msg.m_data, &dataIndex);

		}else{

			dataIndex += dataSize; //Skip the data

		}

		i++; //Increment index
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
				if(verifyChecksum(xbusMessage));

			}

			//Check pipe
			if( (measurementMessageSize > 0) ){

				MtsspInterface_readFromPipe(xbusMessage+2, measurementMessageSize, XBUS_MEASUREMENT_PIPE); //Read from the pipe

					//Parse message information
					msg.m_mid = getMessageId(xbusMessage);
					msg.m_length = getPayloadLength(xbusMessage);
					msg.m_data = getPointerToPayload(xbusMessage);

					if(verifyChecksum(xbusMessage))
						mti_mtData2_parse(msg);
			}

			} //else create a handle for notificationMessage
		}
	}

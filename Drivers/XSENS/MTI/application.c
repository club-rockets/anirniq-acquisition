
//  Copyright (c) 2003-2020 Xsens Technologies B.V. or subsidiaries worldwide.
//  All rights reserved.
//  
//  Redistribution and use in source and binary forms, with or without modification,
//  are permitted provided that the following conditions are met:
//  
//  1.	Redistributions of source code must retain the above copyright notice,
//  	this list of conditions, and the following disclaimer.
//  
//  2.	Redistributions in binary form must reproduce the above copyright notice,
//  	this list of conditions, and the following disclaimer in the documentation
//  	and/or other materials provided with the distribution.
//  
//  3.	Neither the names of the copyright holders nor the names of their contributors
//  	may be used to endorse or promote products derived from this software without
//  	specific prior written permission.
//  
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
//  EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
//  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
//  THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
//  SPECIAL, EXEMPLARY OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 
//  OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
//  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR
//  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.THE LAWS OF THE NETHERLANDS 
//  SHALL BE EXCLUSIVELY APPLICABLE AND ANY DISPUTES SHALL BE FINALLY SETTLED UNDER THE RULES 
//  OF ARBITRATION OF THE INTERNATIONAL CHAMBER OF COMMERCE IN THE HAGUE BY ONE OR MORE 
//  ARBITRATORS APPOINTED IN ACCORDANCE WITH SAID RULES.
//  

#include "application.h"
#include "board.h"
#include "mtssp_interface.h"
#include "xbusmessageid.h"
#include "wait.h"
#include "xbusdef.h"
#include "xbustostring.h"
#include "xbushelpers.h"

#include <string.h>

typedef struct t_config{

	uint8_t deviceID[4];
	uint8_t samplingP[2];
	uint8_t OutputSkipfactor[2];
	uint8_t SsettingsMode[2];
	uint8_t SsettingsOffset[4];
	uint8_t YYYYMMDD[8];
	uint8_t HHMMSSHH[8];
	uint8_t numberDevice[2];
	uint8_t DeviceID[4];
	uint8_t MTData2Datalength[2];
	uint8_t outputMode[2];
	uint8_t outputSettings[4];

}config;

//Transcript enable define
#define configTRANSCRIPT_ENABLED 	1

//Transcript message
#define configTRANSCRIPT_WARNING 	1
#define configTRANSCRIPT_ERROR 		1
#define configTRANSCRIPT_DEFAULT 	1

enum{

	CWARNING = 1,
	CERROR

};

typedef enum t_mti_state{

	mti_boot = 0,
	mti_config,
	mti_measurement

}mti_state;

static uint8_t xbusMessage[2056];

void transcript(const char* module, const char* pData, uint8_t code);
void mti_readPipe();
void mti_error();

void transcript(const char* module, const char* pData, uint8_t code){

#if (configTRANSCRIPT_ENABLED)

	UART_HandleTypeDef* usrPeripheral = &huart2;

	switch(code){

		case CWARNING:
#if (configTRANSCRIPT_WARNING)
			HAL_UART_Transmit(usrPeripheral,(uint8_t*)("*WARNING* "),10,100);

			HAL_UART_Transmit(usrPeripheral,(uint8_t*)("["),1,100);
			HAL_UART_Transmit(usrPeripheral,(uint8_t*)module,strlen(module),100);
			HAL_UART_Transmit(usrPeripheral,(uint8_t*)("] "),2,100);
			HAL_UART_Transmit(usrPeripheral,(uint8_t*)pData,strlen(pData),100);
			HAL_UART_Transmit(usrPeripheral,(uint8_t*)("\n\r"),2,100);
#endif
			break;
		case CERROR:
#if (configTRANSCRIPT_ERROR)
			HAL_UART_Transmit(usrPeripheral,(uint8_t*)("*ERROR* "),8,100);

			HAL_UART_Transmit(usrPeripheral,(uint8_t*)("["),1,100);
			HAL_UART_Transmit(usrPeripheral,(uint8_t*)module,strlen(module),100);
			HAL_UART_Transmit(usrPeripheral,(uint8_t*)("] "),2,100);
			HAL_UART_Transmit(usrPeripheral,(uint8_t*)pData,strlen(pData),100);
			HAL_UART_Transmit(usrPeripheral,(uint8_t*)("\n\r"),2,100);
#endif
			break;
		default:
#if (configTRANSCRIPT_DEFAULT)
			HAL_UART_Transmit(usrPeripheral,(uint8_t*)("["),1,100);
			HAL_UART_Transmit(usrPeripheral,(uint8_t*)module,strlen(module),100);
			HAL_UART_Transmit(usrPeripheral,(uint8_t*)("] "),2,100);
			HAL_UART_Transmit(usrPeripheral,(uint8_t*)pData,strlen(pData),100);
			HAL_UART_Transmit(usrPeripheral,(uint8_t*)("\n\r"),2,100);
#endif
			break;
	}
#endif

}

void mti_readPipe(){

	uint16_t notificationMessageSize;
	uint16_t measurementMessageSize;

	transcript("mti","Reading pipe status",0);
	MtsspInterface_readPipeStatus(&notificationMessageSize, &measurementMessageSize);

	//Check notification pipe
	if(notificationMessageSize > 0){

		transcript("mti","Reading notification message",0);
		MtsspInterface_readFromPipe(xbusMessage, notificationMessageSize, XBUS_NOTIFICATION_PIPE);

	}else{

		transcript("mti","No notification message",0);

	}

	//Check measurement pipe
	if(measurementMessageSize > 0){

		transcript("mti","Reading measurement message",0);
		MtsspInterface_readFromPipe(xbusMessage, measurementMessageSize, XBUS_MEASUREMENT_PIPE);

	}else{

		transcript("mti","No measurement message",0);
	}
}

void mti_error(){

	transcript("mti", "Something went wrong - Suspend task", CERROR);
	while(1); //Stop the task
}

/*!	\brief Defines the main loop of the program which handles user commands
*/
void run()
{

	XbusMessage msg;
	uint16_t notificationMessageSize;
	uint16_t measurementMessageSize;

	//Fastest data frequency : 100 Hz
	//500ms watchdog

	//Reset the device
	transcript("mti","Reset MTI module",0);
	resetDevice();

	transcript("mti","Wait for data ready pin",0);

	//Wait for the MTI too boot
	while(HAL_GPIO_ReadPin(DATA_READY_PORT, DATA_READY_PIN) == GPIO_PIN_SET){

	}

	while(HAL_GPIO_ReadPin(DATA_READY_PORT, DATA_READY_PIN) != GPIO_PIN_SET){

	}

	transcript("mti","Data is ready",0);

	//Read the pipe status
	transcript("mti","Reading pipe status",0);
	MtsspInterface_readPipeStatus(&notificationMessageSize, &measurementMessageSize);
	wait_us(5); //Wait for next transfert

	if(notificationMessageSize > 0){

		//Read the wake up message (XMID_Wakeup)
		transcript("mti","Reading notification message",0);
		MtsspInterface_readFromPipe(xbusMessage+2, notificationMessageSize, XBUS_NOTIFICATION_PIPE);
		wait_us(5); //Wait for next transfert

		if(getMessageId(xbusMessage) == XMID_Wakeup){

			//Respond with WakeUpAck message (XMID_WakeupAck)
			transcript("mti","Send XMID_WakeupAck message",0);
			XbusMessageCreate(&msg,XMID_WakeupAck);
			MtsspInterface_sendXbusMessage(&msg);
			wait_us(5); //Wait for next transfert

			//Verifier que je fais la bonne chose


		}else{

			transcript("mti","Message received was not XMID_Wakeup",CERROR);

			//Send the message received
			transcript("mti",xbusToString(xbusMessage),CERROR);
			mti_error();

		}

	}else{
		transcript("mti","No notification message",CERROR);
		mti_error();
	}

	//Check if in config state
	transcript("mti","Go to configuration mode",0);
	XbusMessageCreate(&msg,XMID_GotoConfig);
	MtsspInterface_sendXbusMessage(&msg);
	wait_us(5); //Wait for next transfert

	transcript("mti","Wait for data ready pin",0);

	//Wait for dataReady
	while(HAL_GPIO_ReadPin(DATA_READY_PORT, DATA_READY_PIN) != GPIO_PIN_SET){

	}

	transcript("mti","Data is ready",0);

	//Read the pipe status
	transcript("mti","Reading pipe status",0);
	MtsspInterface_readPipeStatus(&notificationMessageSize, &measurementMessageSize);
	wait_us(5); //Wait for next transfert

	if(notificationMessageSize > 0){

			//Read the wake up message (XMID_Wakeup)
			transcript("mti","Reading notification message",0);
			MtsspInterface_readFromPipe(xbusMessage+2, notificationMessageSize, XBUS_NOTIFICATION_PIPE);
			wait_us(5); //Wait for next transfert

			if(getMessageId(xbusMessage) == XMID_GotoConfigAck){

				transcript("mti","Module is in configuration mode",0);

			}else{

				transcript("mti","Message received was not XMID_GotoConfigAck",CERROR);

				//Send the message received
				transcript("mti",xbusToString(xbusMessage),CERROR);
				mti_error();

			}

		}else{
			transcript("mti","No notification message",CERROR);
			mti_error();
		}

		//Request MTi-7 configuration
		transcript("mti","Request Mti-7 ID",0);
		XbusMessageCreate(&msg,0x00);
		MtsspInterface_sendXbusMessage(&msg);
		wait_us(5); //Wait for next transfert

		transcript("mti","Wait for data ready pin",0);

		//Wait for dataReady
		while(HAL_GPIO_ReadPin(DATA_READY_PORT, DATA_READY_PIN) != GPIO_PIN_SET){

		}

		transcript("mti","Data is ready",0);

		//Read the pipe status
		transcript("mti","Reading pipe status",0);
		MtsspInterface_readPipeStatus(&notificationMessageSize, &measurementMessageSize);
		wait_us(5); //Wait for next transfert

		if(notificationMessageSize > 0){

				//Read the wake up message (XMID_Wakeup)
				transcript("mti","Reading notification message",0);
				MtsspInterface_readFromPipe(xbusMessage+2, notificationMessageSize, XBUS_NOTIFICATION_PIPE);
				wait_us(5); //Wait for next transfert

				if(getMessageId(xbusMessage) == 0x01){

					xbusMessage[0] = XBUS_PREAMBLE;
					transcript("mti",xbusToString(xbusMessage),0);


				}else{

					transcript("mti","Message received was not 0x01",CERROR);

					//Send the message received
					transcript("mti",xbusToString(xbusMessage),CERROR);
					mti_error();

				}

			}else{
				transcript("mti","No notification message",CERROR);
				mti_error();
			}

}

/*!	\brief Resets the MTi
*/
void resetDevice()
{
	HAL_GPIO_WritePin(RESET_PORT, RESET_PIN, GPIO_PIN_SET);
	wait_us(5000);
	HAL_GPIO_WritePin(RESET_PORT, RESET_PIN, GPIO_PIN_RESET);
}

/*!	\brief Let the user interactively configure a new DataReady configuration
*/
/*bool enterDataReadyConfig(uint8_t* dataReadyConfig)
{
	*dataReadyConfig = 0;

	{
		uint8_t polarity;
		polarity = readInteger();
		*dataReadyConfig |= (polarity << DRDY_CONFIG_POL_POS);
	}

	{
		uint8_t outputType;
		outputType = readInteger();
		*dataReadyConfig |= (outputType << DRDY_CONFIG_OTYPE_POS);
	}

	{
		uint8_t enableNotificationPipeEvent;
		enableNotificationPipeEvent = readInteger();
		*dataReadyConfig |= (enableNotificationPipeEvent << DRDY_CONFIG_NEVENT_POS);
	}

	{
		uint8_t enableMeasurementPipeEvent;
		enableMeasurementPipeEvent = readInteger();
		*dataReadyConfig |= (enableMeasurementPipeEvent << DRDY_CONFIG_MEVENT_POS);
	}

	return 1;
}*/


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



/*!	\brief Defines the main loop of the program which handles user commands
*/
void run()
{

	//Wait for the MTI too boot
	while(HAL_GPIO_ReadPin(DATA_READY_PORT, DATA_READY_PIN) != GPIO_PIN_SET){

		//10ms led blinking
		HAL_GPIO_TogglePin(GPIOD, LD6_Pin);
		wait_us(80000);

	}

	//The device is now ready

		HAL_GPIO_WritePin(GPIOD, LD6_Pin, GPIO_PIN_SET);

		//Check if there is something in both pipe
		uint16_t notificationMessageSize, measurementMessageSize;
		MtsspInterface_readPipeStatus(&notificationMessageSize,&measurementMessageSize);

		XbusMessage msg;
		msg.m_mid = XMID_ReqFirmwareRevision;
		MtsspInterface_sendXbusMessage(&msg);

		MtsspInterface_readPipeStatus(&notificationMessageSize,&measurementMessageSize);

		wait_us(5);

}


/*!	\brief Resets the MTi
*/
void resetDevice()
{
	HAL_GPIO_WritePin(RESET_PORT, RESET_PIN, GPIO_PIN_SET);
	wait_us(1000);
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

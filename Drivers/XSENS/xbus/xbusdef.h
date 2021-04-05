
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

#ifndef XBUSDEF_H
#define XBUSDEF_H

/*! \brief Xbus message preamble byte. */
#define XBUS_PREAMBLE (0xFA)
/*! \brief Xbus message bus ID for master devices. */
#define XBUS_MASTERDEVICE (0xFF)
/*! \brief Xbus length byte for messages without payload. */
#define XBUS_NO_PAYLOAD (0x00)
/*! \brief Xbus length byte for message with an extended payload. */
#define XBUS_EXTENDED_LENGTH (0xFF)

/*! \brief Opcode to write to control pipe in I2C/SPI mode */
#define XBUS_PROTOCOL_INFO (0x01)
#define XBUS_CONFIGURE_PROTOCOL (0x02)
#define XBUS_CONTROL_PIPE (0x03)
#define XBUS_PIPE_STATUS (0x04)
#define XBUS_NOTIFICATION_PIPE (0x05)
#define XBUS_MEASUREMENT_PIPE (0x06)

#define OFFSET_TO_PREAMBLE		0
#define OFFSET_TO_BID			1
#define OFFSET_TO_MID			2
#define OFFSET_TO_LEN			3
#define OFFSET_TO_LEN_EXT_HI	4
#define OFFSET_TO_LEN_EXT_LO	5
#define OFFSET_TO_PAYLOAD		4
#define OFFSET_TO_PAYLOAD_EXT	6
#define LENGTH_EXTENDER_BYTE	0xFF
#define XBUS_CHECKSUM_SIZE		1

/* CONFIGURATION */

#define CONFIG_SIZE 118

//MTI-7 config defines
#define CONFIG_OFFSET_MASTER_DEVICE_ID 			0
#define CONFIG_OFFSET_SAMPLING_PERIOD 			4
#define CONFIG_OFFSET_OUTPUT_SKIP_FACTOR 		6
#define CONFIG_OFFSET_SETTINGS_MODE				8
#define CONFIG_OFFSET_SETTINGS_SKIP_FACTOR		10
#define CONFIG_OFFSET_SETTINGS_OFFSET			12
#define CONFIG_OFFSET_YYYYMMDD					16
#define CONFIG_OFFSET_HHMMSS					24
#define CONFIG_OFFSET_NUMBER_OF_DEVICE			96
#define CONFIG_OFFSET_DEVICE_ID					98
#define CONFIG_OFFSET_MTDATA2_LENGTH			102
#define CONFIG_OFFSET_OUTPUT_MODE				104
#define CONFIG_OFFSET_OUTPUT_SETTINGS			106

//MTI-7 config defines
#define CONFIG_SIZE_MASTER_DEVICE_ID 		4
#define CONFIG_SIZE_SAMPLING_PERIOD 		2
#define CONFIG_SIZE_OUTPUT_SKIP_FACTOR 		2
#define CONFIG_SIZE_SETTINGS_MODE			2
#define CONFIG_SIZE_SETTINGS_SKIP_FACTOR	2
#define CONFIG_SIZE_SETTINGS_OFFSET			4
#define CONFIG_SIZE_YYYYMMDD				8
#define CONFIG_SIZE_HHMMSS					8
#define CONFIG_SIZE_NUMBER_OF_DEVICE		2
#define CONFIG_SIZE_DEVICE_ID				4
#define CONFIG_SIZE_MTDATA2_LENGTH			2
#define CONFIG_SIZE_OUTPUT_MODE				2
#define CONFIG_SIZE_OUTPUT_SETTINGS			4

typedef struct __attribute__ ((__packed__)){

	uint8_t masterDeviceID[4];
	uint8_t samplingPeriod[2];
	uint8_t outputSkipfactor[2];
	uint8_t syncinSettingsMode[2];
	uint8_t syncinSkipFactor[2];
	uint8_t syncinSettingsOffset[4];
	uint8_t YYYYMMDD[8];
	uint8_t HHMMSSHH[8];
	uint8_t reserved1[64];
	uint8_t numberDevice[2];
	uint8_t deviceID[4];
	uint8_t mtData2Datalength[2];
	uint8_t outputMode[2];
	uint8_t outputSettings[4];
	uint8_t reserved2[8];

}mtiConfiguration;

/* MTDATA2 */

typedef struct t_utcTime{

	uint32_t ns;
	uint16_t year;
	uint8_t month;
	uint8_t day;
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
	uint8_t flags;

}utcTime;

typedef struct t_quaterion{

	float q0;
	float q1;
	float q2;
	float q3;

}quaterion;

typedef struct t_eulerAngle{

	float roll;
	float pitch;
	float yaw;

}eulerAngle;

typedef struct t_rotationMatrix{

	float a;
	float b;
	float c;
	float d;
	float e;
	float f;
	float g;
	float h;
	float i;

}rotationMatrix;

typedef struct t_tridimentional{

	float x;
	float y;
	float z;

}tridimentional;

typedef struct t_mtData2{

	float xdiTemperature;
	utcTime xdiUtcTime;
	uint16_t xdiPacketCounter;
	uint32_t xdiSampleTimeFine;
	uint32_t xdiSampleTimeCoarse;
	quaterion xdiQuaterion;
	eulerAngle xdiEulerAngle;
	rotationMatrix xdiRotationMatrix;
	uint32_t xdiBaroPressure;
	tridimentional xdiDeltaV;
	quaterion xdiDeltaQ;
	tridimentional xdiAcceleration;
	tridimentional xdiFreeAcceleration;
	tridimentional xdiAccelerationHR;
	tridimentional xdiRateOfTurn;
	tridimentional xdiRateOfTurnHR;

	/* Completer */

	uint32_t xdiGnssPvtPulse;
	uint16_t xdiRawAccGyrMagTemp[10];
	uint16_t xdiRawGyroTemp[3];
	tridimentional xdiMagneticField;
	uint8_t xdiStatusByte;
	uint32_t xdiStatusWord;
	uint32_t xdiDeviceId;
	uint16_t xdiLocationId;
	tridimentional xdiPositionEcef;
	float xdiLatLon[2];
	float xdiAltitudeEllipsoid;
	tridimentional xdiVelocityXYZ;

}mtData2;

#endif

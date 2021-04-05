
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

#include "xbustostring.h"
#include "xbusmessageid.h"
#include <stdio.h>
#include <string.h>
#include "xbushelpers.h"


/*!	\brief Helper function for reading a uint8_t
*/
uint8_t readUint8(const uint8_t* data, uint8_t* index)
{
	uint8_t result = data[(*index)++];
	return result;
}


/*!	\brief Helper function for reading a uint16_t
*/
uint16_t readUint16(const uint8_t* data, uint8_t* index)
{
	uint16_t result = 0;
	result |= data[(*index)++] << 8;
	result |= data[(*index)++] << 0;
	return result;
}


/*!	\brief Helper function for reading a uint32_t
*/
uint32_t readUint32(const uint8_t* data, uint8_t* index)
{
	uint32_t result = 0;
	result |= data[(*index)++] << 24;
	result |= data[(*index)++] << 16;
	result |= data[(*index)++] << 8;
	result |= data[(*index)++] << 0;
	return result;
}


/*!	\brief Helper function for reading a float
*/
float readFloat(const uint8_t* data, uint8_t* index)
{
	uint32_t temp = readUint32(data, index);
	float result;
	memcpy(&result, &temp, 4);
	return result;
}

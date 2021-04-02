#ifndef TRANSCRIPT_H
#define TRANSCRIPT_H

#include "main.h"
#include "usart.h"
#include "stdint.h"

//Transcript enable define
#define configTRANSCRIPT_ENABLED 	0

//Transcript message
#define configTRANSCRIPT_WARNING 	0
#define configTRANSCRIPT_ERROR 		0
#define configTRANSCRIPT_DEFAULT 	0

#define configTRANSCRIPT_UART_PERIPHERAL &huart1

enum{

	CWARNING = 1,
	CERROR

};

void transcript(const char* module, const char* pData, uint8_t code);

#endif

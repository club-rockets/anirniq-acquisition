

#ifndef APP_SD_H
#define APP_SD_H

#include "stdint.h"

#define SD_QUEUE_SIZE 30
#define SD_QUEUE_BLOCK_SIZE sizeof(struct sd_log)

enum sd_dataType{
	type_UINT,
	type_INT,
	type_FLOAT
};

union sd_data{
	uint32_t UINT;
	int32_t INT;
	float FLOAT;
};

struct sd_log{
	char * title;
	enum sd_dataType type;
	uint32_t time;
	union sd_data data;
};

void task_sd(void * pvParameters);

//add the value to the sd queue, name : data
uint32_t sd_writeInt(char* name,int32_t data);

uint32_t sd_writeUint(char* name,uint32_t data);

uint32_t sd_writeFloat(char* name,float data);





#endif //APP_SD_H

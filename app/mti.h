#ifndef MTI_H
#define MTI_H

#include "stdint.h"
#include "main.h"

#define SAMPLING_PERIOD 50 //10-500 Hz

typedef enum {

	mti_measurement = 0,
	mti_configuration

}mti_device_state;

extern SemaphoreHandle_t xSemaphoreDRDY;
StaticSemaphore_t xSemaphoreDRDYBuffer;

void resetDevice(void);
void config_mti(void);
void mti_send(uint8_t msgId);
void mti_receive(uint8_t msgId);
void task_mti(void * pvParameters);

#endif

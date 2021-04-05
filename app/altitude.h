#ifndef ALTITUDE_H_
#define ALTITUDE_H_

#include "main.h"

extern SemaphoreHandle_t xSemaphoreSPI;

void config_altitude(void);
void task_altitude(void * pvParameters);

#endif

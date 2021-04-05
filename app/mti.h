#ifndef MTI_H
#define MTI_H

#include "stdint.h"
#include "main.h"
#include "main.h"
#include "board.h"
#include "mtssp_interface.h"
#include "xbusmessageid.h"
#include "wait.h"
#include "xbusdef.h"
#include "xbustostring.h"
#include "xbushelpers.h"
#include "xsdataidentifier.h"
#include "transcript.h"
#include "../../shared/app/sd.h"

#define SAMPLING_PERIOD 50 //10-500 Hz

typedef enum {

	mti_measurement = 0,
	mti_configuration

}mti_device_state;

typedef struct {

	uint16_t id;
	uint16_t frequency;
	char name[20];
	void (*callback)(uint8_t,uint8_t*,uint8_t*);

}mti_api;

/** 3-float tuple */
typedef struct {
  float x, y, z;
} vec3f_t;

extern SemaphoreHandle_t xSemaphoreDRDY;
extern SemaphoreHandle_t xSemaphoreSPI;

void resetDevice(void);
void config_mti(void);
void mti_send(uint8_t msgId);
void mti_receive(uint8_t msgId);
uint8_t mti_mtData2_parse(XbusMessage msg);
uint8_t mti_mtData2_configure(void);
void task_mti(void * pvParameters);

#endif

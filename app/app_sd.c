#include "app_sd.h"

#include <stdio.h>
#include <string.h>

#include "cmsis_os.h"
#include "main.h"
#include "fatfs.h"
#include "ff.h"

static FATFS sdfs;
static FIL logfile;

static osMessageQId sd_events;

void app_sd();
osThreadDef(sd, app_sd, osPriorityLow, 1, 2048);

void app_sd_init()
{
    osMessageQDef(sdqueue, 10, uint16_t);
    sd_events = osMessageCreate(osMessageQ(sdqueue), NULL);

    osThreadCreate(osThread(sd), NULL);
}

void app_sd()
{
    osEvent event;

    while (1) {
        event = osMessageGet(sd_events, osWaitForever);

        if (event.status == osEventMessage) {
            switch(event.value.v) {
                case CARD_CONNECTED:
                    if (BSP_SD_Init() == MSD_OK) {
                        if (f_mount(&sdfs, SDPath, 1) != FR_OK) {
                            break;
                        }

                        if (f_open(&logfile, "journal.log", FA_OPEN_ALWAYS | FA_WRITE) != FR_OK) {
                            break;
                        }
                        f_lseek(&logfile, f_size(&logfile));
                        f_printf(&logfile, "%u SD Card mounted\n", HAL_GetTick());
                        f_sync(&logfile); 
                        HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET);
                    }
                    break;
                case CARD_DISCONNECTED:
                    HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET);
                    break;

                default:
                    break;
            }
        }
    }
}

void app_sd_detect_handler()
{
    if (!HAL_GPIO_ReadPin(SD_DETECT_GPIO_Port, SD_DETECT_Pin)) {
        osMessagePut(sd_events, CARD_CONNECTED, osWaitForever);
    }
    else {
        osMessagePut(sd_events, CARD_DISCONNECTED, osWaitForever);
    }
}

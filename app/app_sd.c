#include "app_sd.h"

#include <stdio.h>
#include <string.h>

#include "cmsis_os.h"
#include "fatfs.h"
#include "ff.h"
#include "main.h"

static FATFS sdfs;
static FIL logfile;
static FIL datafile;

static QueueHandle_t sd_events;

static char line[128];
static uint8_t buffer[512] = { 0 };
static int cursor = 0;
static int mounted = 0;

static void app_sd();
static void sd_mount();
static void sd_flush();
static void sd_write_data(sd_data_t* data);
osThreadDef(sd, app_sd, osPriorityLow, 1, 2048);

void app_sd_init()
{
    sd_events = xQueueCreate(10, sizeof(sd_event_t));
    osThreadCreate(osThread(sd), NULL);
}

void app_sd_write_data(sd_data_t* data)
{
    sd_event_t event = {
        .type = DATA_READY,
        .data = data
    };
    xQueueSend(sd_events, &event, osWaitForever);
}

static void app_sd()
{
    sd_event_t event;

    while (1) {
        xQueueReceive(sd_events, &event, osWaitForever);
        switch (event.type) {
            case CARD_CONNECTED:
                sd_mount();
                mounted = 1;
                break;

            case CARD_DISCONNECTED:
                HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET);
                mounted = 0;
                break;

            case DATA_READY:
                sd_write_data(event.data);
                break;

            default:
                break;
        }
    }
}

static void sd_mount()
{
    if (BSP_SD_Init() == MSD_OK) {
        if (f_mount(&sdfs, SDPath, 1) != FR_OK) {
            return;
        }

        if (f_open(&logfile, "journal.log", FA_OPEN_ALWAYS | FA_WRITE) != FR_OK) {
            return;
        }
        f_lseek(&logfile, f_size(&logfile));

        if (f_open(&datafile, "data.csv", FA_OPEN_ALWAYS | FA_WRITE) != FR_OK) {
            return;
        }
        f_lseek(&datafile, f_size(&datafile));

        f_printf(&logfile, "%u SD Card mounted\n", HAL_GetTick());
        f_sync(&logfile);
        HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET);
    }
}

static void sd_write_data(sd_data_t* data)
{
    int size, left;
    if (!mounted) {
        return;
    }

    sprintf(line, "%f\n", data->val);
    size = strlen(line);
    left = 512 - cursor;

    if (size > left) {
        memcpy(buffer + cursor, line, left);
        sd_flush();
        cursor = size - left;
        memcpy(buffer, line + left, cursor);
        buffer[cursor + 1] = 0;
    }
    else {
        memcpy(buffer + cursor, line, size);
        cursor += size;
    }

    return;
}

static void sd_flush()
{
    unsigned bw;
    f_write(&datafile, buffer, 512, &bw);
    f_sync(&datafile);
    cursor = 0;
    HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
}

void app_sd_detect_handler()
{
    sd_event_t event = { .data = NULL };
    long int pd;

    if (!HAL_GPIO_ReadPin(SD_DETECT_GPIO_Port, SD_DETECT_Pin)) {
        event.type = CARD_CONNECTED;
    } else {
        event.type = CARD_DISCONNECTED;
    }
    xQueueSendFromISR(sd_events, &event, &pd);
}

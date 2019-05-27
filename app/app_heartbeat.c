#include "app_heartbeat.h"

#include "cmsis_os.h"
#include "main.h"

void app_heartbeat();
osThreadDef(heartbeat, app_heartbeat, osPriorityLow, 1, 512);

void app_heartbeat_init()
{
    osThreadCreate(osThread(heartbeat), NULL);
}

void app_heartbeat()
{
    while (1) {
        osDelay(100);
        HAL_GPIO_TogglePin(LED4_GPIO_Port, LED4_Pin);
    }
}
#include "app_altitude.h"
#include "app_sd.h"

#include "cmsis_os.h"

#include "main.h"
#include "barometer/barometer.h"
#include "kalman/kalman.h"

void app_altitude();
osThreadDef(altitude, app_altitude, osPriorityNormal, 1, 512);

void app_altitude_init()
{
    osThreadCreate(osThread(altitude), NULL);
}

void app_altitude()
{
    barometer_t barometer;
    barometer_init(&barometer, BARO_CS_GPIO_Port, BARO_CS_Pin, &hspi1);

    kalman_t kalman;
    kalman_init(&kalman);
    sd_data_t data;

    while (1) {
        barometer_update(&barometer);
        kalman_update(&kalman, pressure_to_altitude(barometer.pressure), 0, 0.020);
        osDelay(20);
        data.val = kalman.altitude;
        app_sd_write_data(&data);
        HAL_GPIO_TogglePin(LED3_GPIO_Port, LED3_Pin);
    }
}
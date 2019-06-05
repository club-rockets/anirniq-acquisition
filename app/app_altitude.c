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
    sd_data_t* data;

    barometer_t barometer;
    barometer_init(&barometer, BARO_CS_GPIO_Port, BARO_CS_Pin, &hspi1);

    kalman_t kalman;
    kalman_init(&kalman);

    while (1) {
        barometer_update(&barometer);
        if ((data = app_sd_prepare_data()) != NULL) {
            data->generic.type = SD_DATA_BARO;
            data->baro.pressure = barometer.pressure;
            data->baro.temperature = barometer.temperature;
            app_sd_write_data(data);
        }

        kalman_update(&kalman, pressure_to_altitude(barometer.pressure), 0, 0.020f);
        if ((data = app_sd_prepare_data()) != NULL) {
            data->generic.type = SD_DATA_KALMAN;
            data->kalman.altitude = kalman.altitude;
            data->kalman.velocity = kalman.velocity;
            data->kalman.acceleration = kalman.acceleration;
            app_sd_write_data(data);
        }

        osDelay(20);
        HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);
    }
}
#include "app_altitude.h"
#include "app_sd.h"

#include "cmsis_os.h"

#include "main.h"
#include "barometer/barometer.h"
#include "kalman/kalman.h"
#include "bsp_can.h"
#include "can/can_driver.h"

void app_altitude();
osThreadDef(altitude, app_altitude, osPriorityNormal, 1, 512);

void app_altitude_init()
{
    osThreadCreate(osThread(altitude), NULL);
}

void app_altitude()
{
    sd_data_t* data;
    can_regData_u can_data = { 0 };
    int cycle = 0;

    barometer_t barometer;
    barometer_init(&barometer, BARO_CS_GPIO_Port, BARO_CS_Pin, &hspi1);

    kalman_t kalman;
    kalman_init(&kalman);


    while (1) {
        barometer_update(&barometer);
        if ((data = app_sd_prepare_data(SD_DATA_BARO)) != NULL) {
            data->baro.pressure = barometer.pressure;
            data->baro.temperature = barometer.temperature;
            app_sd_write_data(data);
        }
        if (cycle == 0) {
            can_data.FLOAT = barometer.pressure;
            can_canSetRegisterData(CAN_ACQUISITION_AIR_PRESSURE_INDEX, &can_data);
            can_data.FLOAT = barometer.temperature;
            can_canSetRegisterData(CAN_ACQUISITION_AIR_TEMPERATURE_INDEX, &can_data);
        }


        kalman_update(&kalman, pressure_to_altitude(barometer.pressure), 0, 0.020f);
        if ((data = app_sd_prepare_data(SD_DATA_KALMAN)) != NULL) {
            data->kalman.altitude = kalman.altitude;
            data->kalman.velocity = kalman.velocity;
            data->kalman.acceleration = kalman.acceleration;
            app_sd_write_data(data);
        }
        if (cycle != 0) {
            can_data.FLOAT = kalman.altitude;
            can_canSetRegisterData(CAN_ACQUISITION_ALTITUDE_INDEX, &can_data);
            can_data.FLOAT = kalman.velocity;
            can_canSetRegisterData(CAN_ACQUISITION_VERTICAL_VELOCITY_INDEX, &can_data);
        }

        osDelay(20);
        HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);
        cycle = !cycle;
    }
}
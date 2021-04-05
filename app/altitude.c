#include "altitude.h"

#include "../../shared/app/sd.h"

#include "main.h"
#include "../../shared/drivers/barometer/barometer.h"
#include "../../shared/middleware/kalman/kalman.h"
#include "../../shared/bsp/bsp_can.h"
#include "../../shared/drivers/can/can_driver.h"

static barometer_t barometer;

void config_altitude(void){

    barometer_init(&barometer, BARO_CS_GPIO_Port, BARO_CS_Pin, &hspi1);

}

void task_altitude(void * pvParameters){

    //sd_data_t* data;
    can_regData_u can_data = { 0 };
    int cycle = 0;

    kalman_t kalman;
    kalman_init(&kalman);

    while (1) {

		//Wait to take the semaphore
		while(xSemaphoreTake( xSemaphoreSPI, portMAX_DELAY) == pdFALSE);

        	barometer_update(&barometer);

        xSemaphoreGive( xSemaphoreSPI );
       /* if ((data = app_sd_prepare_data(SD_DATA_BARO)) != NULL) {
            data->baro.pressure = barometer.pressure;
            data->baro.temperature = barometer.temperature;
            app_sd_write_data(data);
        }*/

        sd_writeFloat("Pressure", barometer.pressure);
        sd_writeFloat("Baro_Temperature", barometer.temperature);

        if (cycle == 0) {
            can_data.FLOAT = barometer.pressure;
            can_canSetRegisterData(CAN_ACQUISITION_AIR_PRESSURE_INDEX, &can_data);
            can_data.FLOAT = barometer.temperature;
            can_canSetRegisterData(CAN_ACQUISITION_AIR_TEMPERATURE_INDEX, &can_data);
        }


        kalman_update(&kalman, pressure_to_altitude(barometer.pressure), 0, 0.020f);
        /*if ((data = app_sd_prepare_data(SD_DATA_KALMAN)) != NULL) {
            data->kalman.altitude = kalman.altitude;
            data->kalman.velocity = kalman.velocity;
            data->kalman.acceleration = kalman.acceleration;
            app_sd_write_data(data);
        }*/

        sd_writeFloat("Altitude", kalman.altitude);
        sd_writeFloat("Vertical_Velocity", kalman.velocity);
        sd_writeFloat("Vertical_Acceleration", kalman.acceleration);

        if (cycle != 0) {
            can_data.FLOAT = kalman.altitude;
            can_canSetRegisterData(CAN_ACQUISITION_ALTITUDE_INDEX, &can_data);
            can_data.FLOAT = kalman.velocity;
            can_canSetRegisterData(CAN_ACQUISITION_VERTICAL_VELOCITY_INDEX, &can_data);
        }

        vTaskDelay(50);
        HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);
        cycle = !cycle;
    }
}

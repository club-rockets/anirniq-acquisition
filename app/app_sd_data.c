#include "app_sd_data.h"
#include "stm32f4xx_hal.h"

#include <stdio.h>

void app_sd_format_data(sd_data_t* data, char* line, int len)
{
    switch (data->generic.type) {
        case SD_DATA_BARO:
            snprintf(line, len, "baro,%lu,%f,%f\n", HAL_GetTick(), data->baro.pressure, data->baro.temperature);
            break;
        case SD_DATA_KALMAN:
            snprintf(line, len, "kalman,%lu,%f,%f,%f\n", HAL_GetTick(), data->kalman.altitude, data->kalman.velocity, data->kalman.acceleration);
            break;
        case SD_DATA_GPS:
            snprintf(line, len, "gps,%lu,%f,%f,%f\n", HAL_GetTick(), data->gps.longitude, data->gps.latitude, data->gps.height);
            break;
        default:
            snprintf(line, len, "unhandled data type in app_sd_format_data()\n");
            break;
    }
}
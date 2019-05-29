#include "app_sd_data.h"

#include <stdio.h>

void app_sd_format_data(sd_data_t* data, char* line, int len)
{
    switch (data->generic.type) {
        case SD_DATA_BARO:
            snprintf(line, len, "baro,%f,%f\n", data->baro.pressure, data->baro.temperature);
            break;
        case SD_DATA_KALMAN:
            snprintf(line, len, "kalman,%f,%f,%f\n", data->kalman.altitude, data->kalman.velocity, data->kalman.acceleration);
            break;
        default:
            snprintf(line, len, "unhandled data type in app_sd_format_data()\n");
            break;
    }
}
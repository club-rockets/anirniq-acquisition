#ifndef APP_SD_DATA_H
#define APP_SD_DATA_H

#include <stdint.h>

#define SD_DATA_GENERIC 0x00
#define SD_DATA_BARO    0x01
#define SD_DATA_KALMAN  0x02
#define SD_DATA_GPS     0x03

typedef struct {
    int8_t type;
    int8_t complete;
    float pressure;
    float temperature;
} sd_baro_data_t;

typedef struct {
    int8_t type;
    int8_t complete;
    float altitude;
    float velocity;
    float acceleration;
} sd_kalman_data_t;

typedef struct {
    int8_t type;
    int8_t complete;
    float longitude;
    float latitude;
    float height;
} sd_gps_data_t;

typedef struct sd_generic_data {
    int8_t type;
    int8_t complete;
} sd_generic_data_t;

typedef union sd_data {
    sd_generic_data_t generic;
    sd_baro_data_t baro;
    sd_kalman_data_t kalman;
    sd_gps_data_t gps;
} sd_data_t;

void app_sd_format_data(sd_data_t* data, char* line, int len);

#endif
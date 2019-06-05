#ifndef APP_GPS_H_
#define APP_GPS_H_

#include <stdint.h>

#define GPS_EVENT_DATA  0x01
#define GPS_EVENT_SLEEP 0x02
#define GPS_EVENT_WAKE  0x03

#define UBX_CLASS_NAV 0x01

#define UBX_ID_POSLLH 0x02  // NAV-POSLLH Geodetic Position Solution


/* UBX Payload structs */

struct __attribute__((__packed__)) UBX_POSLLH_payload {
    uint32_t iTOW;   // GPS Millisecond Time of Week (ms)
    int32_t lon;     // Longitude (deg * 1e-7)
    int32_t lat;     // Latitude (deg * 1e-7)
    int32_t height;  // Height above Ellipsoid (mm)
    int32_t hMSL;    // Height MSL (mm)
    uint32_t hAcc;   // Horizontal Accuracy Estimate
    uint32_t vAcc;   // Vertical Accuracy Estimate
};

void app_gps_init();
void app_gps_rx_handler();

#endif
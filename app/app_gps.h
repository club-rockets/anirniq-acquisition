#ifndef APP_GPS_H_
#define APP_GPS_H_

#include <stdint.h>

#define GPS_EVENT_DATA  0x01
#define GPS_EVENT_SLEEP 0x02
#define GPS_EVENT_WAKE  0x03

#define UBX_CLASS_NAV 0x01

#define UBX_ID_POSLLH  0x02  // NAV-POSLLH Geodetic Position Solution
#define UBX_ID_STATUS  0x03  // NAV-STATUS Receiver Navigation Status
#define UBX_ID_TIMEUTC 0x21  // NAV-TIMEUTC UTC Time Solution

typedef enum __attribute__((__packed__)) {
    NO_FIX = 0x00,
    DEAD_RECKONING = 0x01,
    FIX_2D = 0x02,
    FIX_3D = 0x03,
    GPS_AND_DEAD_RECKONING = 0x04,
    TIME_ONLY = 0x05
} GPSfix;

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

struct __attribute__((__packed__)) UBX_TIMEUTC_payload {
    uint32_t iTOW;  // GPS Millisecond Time of Week (ms)
    int32_t nano;   // Nanoseconds of second, range -1e9..1e9
    uint16_t year;  // Year, range 1999..2099
    uint8_t month;  // Month, range 1..12
    uint8_t day;    // Day of Month, range 1..31
    uint8_t hour;   // Hour, range 0..23
    uint8_t min;    // Minutes, range 0..59
    uint8_t sec;    // Seconds, range 0..59
    uint8_t valid;  // Validity Flags
};

struct __attribute__((__packed__)) UBX_STATUS_payload {
    uint32_t iTOW;    // GPS Millisecond Time of Week (ms)
    GPSfix gpsFix;    // GPS fix type
    uint8_t flags;    // Navigation Status Flags
    uint8_t fixStat;  // Fix Status Information
    uint8_t flags2;   // further information about navigation output
    uint32_t ttff;    // Time to first fix (ms time tag)
    uint32_t msss;    // Milliseconds since Startup/Reset
};

void app_gps_init();
void app_gps_rx_handler();

#endif
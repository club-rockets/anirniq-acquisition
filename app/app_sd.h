#ifndef APP_SD_H_
#define APP_SD_H_

#define CARD_CONNECTED    0x00
#define CARD_DISCONNECTED 0x01
#define DATA_READY        0x02

typedef struct {
    float val;
} sd_data_t;

typedef struct {
    int type;
    sd_data_t* data;
} sd_event_t;

void app_sd_init();
void app_sd_write_data(sd_data_t* data);
void app_sd_detect_handler();

#endif
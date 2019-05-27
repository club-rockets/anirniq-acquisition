#ifndef APP_SD_H_
#define APP_SD_H_

#define CARD_CONNECTED    0x00
#define CARD_DISCONNECTED 0x01

void app_sd_init();
void app_sd_detect_handler();

#endif
#include "app_gps.h"
#include <stdint.h>

#include "cmsis_os.h"
#include "queue.h"
#include "message_buffer.h"

#include "main.h"

#include "app_sd.h"

extern UART_HandleTypeDef huart2;

static QueueHandle_t gps_events;
static MessageBufferHandle_t gps_messages;

static void app_gps();
osThreadDef(gps, app_gps, osPriorityNormal, 1, 1024);


enum PARSER_STATE {
    SYNC1,
    SYNC2,
    CLASS,
    ID,
    LENGTH,
    PAYLOAD,
    CK_A,
    CK_B
};
uint8_t rx_state = SYNC1;
uint8_t rx_buf[128];
uint8_t rx_buf_next = 0;
uint16_t rx_payload_len = 0;

static uint8_t message[128];
static uint8_t message_len = 0;

void app_gps_init()
{
    gps_events = xQueueCreate(5, sizeof(int));
    gps_messages = xMessageBufferCreate(512);
    osThreadCreate(osThread(gps), NULL);
}

void app_gps()
{
    int event;
    sd_data_t* sd_data;
    
    __HAL_UART_ENABLE_IT(&huart2, UART_IT_RXNE);

    while (1) {
        xQueueReceive(gps_events, &event, osWaitForever);

        switch (event) {
            case GPS_EVENT_DATA:
                message_len = xMessageBufferReceive(gps_messages, message, 128, 0);
                if (message[2] == UBX_CLASS_NAV && message[3] == UBX_ID_POSLLH) {
                    struct UBX_POSLLH_payload* data = (struct UBX_POSLLH_payload*) &message[6];

                    if ((sd_data = app_sd_prepare_data()) != NULL) {
                        sd_data->gps.longitude = data->lon / 1e-7f;
                        sd_data->gps.latitude = data->lat / 1e-7f;
                        sd_data->gps.height = data->height / 1000.0f;
                        app_sd_write_data(sd_data);
                        HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
                    }
                }
                break;
            default:
                /* Unhandled event */
                break;
        }
    }
}

void app_gps_rx_handler()
{
    if (USART2->SR & USART_SR_RXNE) {
        uint8_t byte = USART2->DR;
        uint8_t ck_a, ck_b;
        rx_buf[rx_buf_next] = byte;
        rx_buf_next++;


        switch (rx_state) {
            case SYNC1:
                if (byte == 0xb5) {
                    rx_state = SYNC2;
                } else {
                    rx_buf_next = 0;
                }
                break;

            case SYNC2:
                if (byte == 0x62) {
                    rx_state = CLASS;
                } else {
                    rx_buf_next = 0;
                    rx_state = SYNC1;
                }
                break;

            case CLASS:
            case ID:
                rx_state++;
                break;

            case LENGTH:
                if (rx_buf_next == 6) {
                    rx_payload_len = (rx_buf[5] << 8) | rx_buf[4];
                    rx_state++;
                }
                break;
            case PAYLOAD:
                if (--rx_payload_len == 0) {
                    rx_state++;
                }
                break;

            case CK_A:
                rx_state++;
                break;

            case CK_B:
                ck_a = 0;
                ck_b = 0;
                for (int i = 0; i < 4 + ((rx_buf[5] << 8) | rx_buf[4]); i++) {
                    ck_a += rx_buf[i + 2];
                    ck_b += ck_a;
                }
                if (ck_a == rx_buf[rx_buf_next - 2] && ck_b == rx_buf[rx_buf_next - 1]) {
                    long int pd;
                    int event = GPS_EVENT_DATA;
                    xMessageBufferSendFromISR(gps_messages, rx_buf, rx_buf_next, &pd);
                    xQueueSendFromISR(gps_events, &event, &pd);
                }
                rx_state = SYNC1;
                rx_buf_next = 0;
                break;
        }
    }
}

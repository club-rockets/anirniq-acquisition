#include "blink.h"

void task_blink(void * pvParameters){

	configASSERT( ( uint32_t ) pvParameters == 0UL );

    for( ;; )
    {
        vTaskDelay(1000); //1 second delay
        HAL_GPIO_TogglePin(LED4_GPIO_Port, LED4_Pin);
    }

}

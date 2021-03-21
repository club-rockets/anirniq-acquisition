#include "transcript.h"

void transcript(const char* module, const char* pData, uint8_t code){

	UART_HandleTypeDef* usrPeripheral = configTRANSCRIPT_UART_PERIPHERAL;

	switch(code){

		case CWARNING:
#if (configTRANSCRIPT_WARNING)
			HAL_UART_Transmit(usrPeripheral,(uint8_t*)("*WARNING* "),10,100);

			HAL_UART_Transmit(usrPeripheral,(uint8_t*)("["),1,100);
			HAL_UART_Transmit(usrPeripheral,(uint8_t*)module,strlen(module),100);
			HAL_UART_Transmit(usrPeripheral,(uint8_t*)("] "),2,100);
			HAL_UART_Transmit(usrPeripheral,(uint8_t*)pData,strlen(pData),100);
			HAL_UART_Transmit(usrPeripheral,(uint8_t*)("\n\r"),2,100);
#endif
			break;
		case CERROR:
#if (configTRANSCRIPT_ERROR)
			HAL_UART_Transmit(usrPeripheral,(uint8_t*)("*ERROR* "),8,100);

			HAL_UART_Transmit(usrPeripheral,(uint8_t*)("["),1,100);
			HAL_UART_Transmit(usrPeripheral,(uint8_t*)module,strlen(module),100);
			HAL_UART_Transmit(usrPeripheral,(uint8_t*)("] "),2,100);
			HAL_UART_Transmit(usrPeripheral,(uint8_t*)pData,strlen(pData),100);
			HAL_UART_Transmit(usrPeripheral,(uint8_t*)("\n\r"),2,100);
#endif
			break;
		default:
#if (configTRANSCRIPT_DEFAULT)
			HAL_UART_Transmit(usrPeripheral,(uint8_t*)("["),1,100);
			HAL_UART_Transmit(usrPeripheral,(uint8_t*)module,strlen(module),100);
			HAL_UART_Transmit(usrPeripheral,(uint8_t*)("] "),2,100);
			HAL_UART_Transmit(usrPeripheral,(uint8_t*)pData,strlen(pData),100);
			HAL_UART_Transmit(usrPeripheral,(uint8_t*)("\n\r"),2,100);
#endif
			break;
	}

}

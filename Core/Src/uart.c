#include "main.h"
/**************extern variables********************************/

extern UART_HandleTypeDef huart1;

/**************static variables********************************/
int8_t rcv_bytes[10]={0};

//Start Uart data receive
void Uart1_Start_Receive(void)
{	
	HAL_UART_Receive_IT(&huart1,(uint8_t*) rcv_bytes,1);
}

//Receive data interrapt
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{	
	HAL_UART_Receive_IT(&huart1,(uint8_t*) rcv_bytes,1);
}

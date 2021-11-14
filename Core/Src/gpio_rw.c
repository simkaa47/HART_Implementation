/*
Module description:
Reading and writing pin states on MCU
*/
#include "main.h"

 uint8_t analog_input;// if analog_input==1, MCU works as analog input, else output
 
 
 void ReadMode()// this function recognized MCU mode (input, output)
 {
		 if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1)==GPIO_PIN_SET)
		 {
				analog_input = 1;
		 }
 }
 
 void SendLedOn()// switch on send indicator
 {
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_SET);
 }
 void SendLedOff()//switch off send indicator
 {
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_RESET);
 }
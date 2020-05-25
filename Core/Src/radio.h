/**
 * radio.h
 * Allen Snook
 * May 21, 2020
 */

#ifndef __RADIO_H
#define __RADIO_H

#include "stm32f4xx_hal.h"
#include "cmsis_os.h"

void Radio_Set_SPI( SPI_HandleTypeDef *spi );
void Radio_Set_Reset_Pin( GPIO_TypeDef* gpio, uint16_t pin );
void Radio_Set_NCS_Pin( GPIO_TypeDef* gpio, uint16_t pin );

void Radio_Set_Message_Queue( osMessageQueueId_t hqueue );

uint8_t Radio_Init();
void Radio_Run();

#endif // __RADIO_H

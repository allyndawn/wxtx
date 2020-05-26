/**
 * gps.h
 * Allen Snook
 * May 26, 2020
 */

#ifndef __GPS_H
#define __GPS_H

#include "stm32f4xx_hal.h"
#include "cmsis_os.h"

void GPS_Set_UART( UART_HandleTypeDef *huart );
void GPS_Set_Message_Queue( osMessageQueueId_t hqueue );
void GPS_Run();

#endif // __GPS_H

/**
 * thp.h
 * Allen Snook
 * May 26, 2020
 */

#ifndef __THP_H
#define __THP_H

#include "stm32f4xx_hal.h"
#include "cmsis_os.h"

void THP_Set_I2C( I2C_HandleTypeDef *hi2c );
void THP_Set_Message_Queue( osMessageQueueId_t hqueue );
void THP_Run();

#endif // __THP_H

/**
 * core.h
 * Allen Snook
 * May 26, 2020
 */

#ifndef __CORE_H
#define __CORE_H

#include "cmsis_os.h"
#include "stm32f4xx_hal.h"

void Core_Set_RTC_Handle( RTC_HandleTypeDef *hrtc );
void Core_Set_THP_Message_Queue( osMessageQueueId_t hqueue );
void Core_Set_GPS_Message_Queue( osMessageQueueId_t hqueue );
void Core_Set_Radio_Message_Queue( osMessageQueueId_t hqueue );
void Core_Run();

#endif // __CORE_H

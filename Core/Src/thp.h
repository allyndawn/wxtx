/**
 * thp.h
 * Allen Snook
 * May 26, 2020
 */

#ifndef __THP_H
#define __THP_H

#include "stm32f4xx_hal.h"
#include "cmsis_os.h"

typedef struct {
	int16_t temperature; // deg C, 0.1 deg res, -90 (-900) to +140 (+1400) deg C
	uint16_t pressure; // mbar, 0.1 mbar res, 870 (8700) to 1100 (11000) mbar
	uint16_t humidity; // percent, 0.1 percent res, 0 to 100 (1000) perfect
} thp_data_type; // 6 bytes

void THP_Set_I2C( I2C_HandleTypeDef *hi2c );
void THP_Set_Message_Queue( osMessageQueueId_t hqueue );
void THP_Run();

#endif // __THP_H

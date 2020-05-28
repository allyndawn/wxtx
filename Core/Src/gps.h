/**
 * gps.h
 * Allen Snook
 * May 26, 2020
 */

#ifndef __GPS_H
#define __GPS_H

#include "stm32f4xx_hal.h"
#include "cmsis_os.h"

typedef struct {
	uint8_t year;				// Years since 2000
	uint8_t month;				// 1 to 12
	uint8_t day;				// 1 to 31
	uint8_t hour;				// 0 to 23
	uint8_t minutes;			// 0 to 59
	uint8_t seconds;			// 0 to 59
	uint8_t latitude_degrees;	// 0 to 90
	uint8_t latitude_minutes;	// 0 to 59
	uint8_t latitude_seconds;	// 0 to 59
	char latitude_hem;			// N or S
	uint8_t longitude_degrees;	// 0 to 180
	uint8_t longitude_minutes;	// 0 to 59
	uint8_t longitude_seconds;	// 0 to 59
	char longitude_hem;			// W or E
} gps_data_type; // 14 bytes

void GPS_Set_UART( UART_HandleTypeDef *huart );
void GPS_Set_Message_Queue( osMessageQueueId_t hqueue );
void GPS_Run();

#endif // __GPS_H

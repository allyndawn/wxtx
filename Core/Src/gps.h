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
	uint16_t year;
	uint8_t month;
	uint8_t day;
	uint8_t hour;
	uint8_t minutes;
	uint8_t seconds;
} gps_time_type;

typedef struct {
	uint8_t degrees;
	uint8_t minutes;
	uint8_t seconds;
	char hem;
} gps_angle_type;

typedef struct {
	gps_angle_type latitude;
	gps_angle_type longitude;
} gps_location_type;

void GPS_Set_UART( UART_HandleTypeDef *huart );
void GPS_Set_Message_Queue( osMessageQueueId_t hqueue );
void GPS_Run();

#endif // __GPS_H

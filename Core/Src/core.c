/**
 * core.c
 * Allen Snook
 * May 26, 2020
 */

#include "core.h"
#include "gps.h"
#include "thp.h"

#ifndef TRUE
#define TRUE UINT8_C(1)
#endif
#ifndef FALSE
#define FALSE UINT8_C(0)
#endif

#define CORE_LOOP_DELAY 250
#define CORE_TRANSMIT_INTERVAL 5000

#define CORE_RADIO_TX_PACKET_LENGTH 24

thp_data_type core_thp_data;
gps_data_type core_gps_data;

static RTC_HandleTypeDef *core_hrtc;
static osMessageQueueId_t core_thp_hqueue;
static osMessageQueueId_t core_gps_hqueue;
static osMessageQueueId_t core_radio_hqueue;
static HAL_StatusTypeDef core_hal_status;
static osStatus_t core_os_status;

static uint8_t core_has_thp_data = FALSE;
static uint8_t core_has_gps_data = FALSE;

static uint16_t core_loop_time = 0;

static RTC_DateTypeDef core_rtc_date;
static RTC_TimeTypeDef core_rtc_time;

static uint8_t core_radio_tx_packet[CORE_RADIO_TX_PACKET_LENGTH];

void Core_Set_RTC_Handle( RTC_HandleTypeDef *hrtc ) {
	core_hrtc = hrtc;
}

void Core_Set_THP_Message_Queue( osMessageQueueId_t hqueue ) {
	core_thp_hqueue = hqueue;
}

void Core_Set_GPS_Message_Queue( osMessageQueueId_t hqueue ) {
	core_gps_hqueue = hqueue;
}

void Core_Set_Radio_Message_Queue( osMessageQueueId_t hqueue ) {
	core_radio_hqueue = hqueue;
}

void _Core_Update_RTC() {
	RTC_TimeTypeDef rtc_time = {0};
	RTC_DateTypeDef rtc_date = {0};

	rtc_time.Hours = core_gps_data.hour;
	rtc_time.Minutes = core_gps_data.minutes;
	rtc_time.Seconds = core_gps_data.seconds;
	rtc_time.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
	rtc_time.StoreOperation = RTC_STOREOPERATION_RESET;
	core_hal_status = HAL_RTC_SetTime( core_hrtc, &rtc_time, RTC_FORMAT_BIN );
	if ( core_hal_status != HAL_OK ) {
		return;
	}
	rtc_date.WeekDay = RTC_WEEKDAY_MONDAY; // TODO
	rtc_date.Month = core_gps_data.month;
	rtc_date.Date = core_gps_data.day;
	rtc_date.Year = core_gps_data.year;

	core_hal_status = HAL_RTC_SetDate( core_hrtc, &rtc_date, RTC_FORMAT_BIN );
	if ( core_hal_status != HAL_OK ) {
		return;
	}
}

void _Core_Handle_GPS_Queue() {
	if ( ! core_gps_hqueue ) {
		return;
	}

	// Receive the gps_data_type structure (14 bytes)
	core_hal_status = osMessageQueueGet( core_gps_hqueue, (void *) &core_gps_data, NULL, 0U );
	if ( core_os_status == osOK ) {
		core_has_gps_data = TRUE;
		_Core_Update_RTC();
	}
}

void _Core_Handle_THP_Queue() {
	if ( ! core_gps_hqueue ) {
		return;
	}

	// Receive the thp_data_type structure (6 bytes)
	core_hal_status = osMessageQueueGet( core_thp_hqueue, (void *) &core_thp_data, NULL, 0U );
	if ( core_os_status == osOK ) {
		core_has_thp_data = TRUE;
	}
}

void _Core_Prepare_Packet() {
	// Do we have a core radio message queue handle?
	if ( ! core_radio_hqueue ) {
		return;
	}

	// Do we have THP data?
	if ( ! core_has_thp_data ) {
		return;
	}

	// Do we have GPS data?
	if ( ! core_has_gps_data ) {
		return;
	}

	// Size checks
	if ( 6 != sizeof( core_thp_data ) ) {
		return;
	}
	if ( 14 != sizeof( core_gps_data ) ) {
		return;
	}

	// Get the latest date and time from the RTC
	core_hal_status = HAL_RTC_GetDate( core_hrtc, &core_rtc_date, RTC_FORMAT_BIN );
	if ( core_hal_status != HAL_OK ) {
		return;
	}

	core_hal_status = HAL_RTC_GetTime( core_hrtc, &core_rtc_time, RTC_FORMAT_BIN );
	if ( core_hal_status != HAL_OK ) {
		return;
	}

	// Update the GPS data with the latest RTC results
	// Since the GPS may take 30 or more seconds to send a datetime update
	core_gps_data.year = core_rtc_date.Year;
	core_gps_data.month = core_rtc_date.Month;
	core_gps_data.day = core_rtc_date.Date;
	core_gps_data.hour = core_rtc_time.Hours;
	core_gps_data.minutes = core_rtc_time.Minutes;
	core_gps_data.seconds = core_rtc_time.Seconds;

	// Build the radio packet
	// Header
	core_radio_tx_packet[0] = CORE_RADIO_TX_PACKET_LENGTH;	// 24
	core_radio_tx_packet[1] = 0x0;							// dest addr
	core_radio_tx_packet[2] = 0x0;							// src addr
	core_radio_tx_packet[3] = 0x0;							// control byte

	// THP Data
	__builtin_memcpy( (void *) &(core_radio_tx_packet[4]), (void *) &core_thp_data, 6 );

	// GPS Data
	__builtin_memcpy( (void *) &(core_radio_tx_packet[10]), (void *) &core_gps_data, 14 );

	// Send it
	osMessageQueuePut( core_radio_hqueue, (void *) &(core_radio_tx_packet[0]), 0U, 0U );
}

void Core_Run() {
	_Core_Handle_GPS_Queue();
	_Core_Handle_THP_Queue();

	// Every CORE_TRANSMIT_INTERVAL build a buffer with all the data and send it to the radio to transmit
	core_loop_time += CORE_LOOP_DELAY;
	if ( CORE_TRANSMIT_INTERVAL <= core_loop_time ) {
		core_loop_time = 0;
		_Core_Prepare_Packet();
	}

	osDelay( CORE_LOOP_DELAY );
}

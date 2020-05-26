/**
 * core.c
 * Allen Snook
 * May 26, 2020
 */

#include "core.h"
#include "gps.h"

static RTC_HandleTypeDef *core_hrtc;
static osMessageQueueId_t core_thp_hqueue;
static osMessageQueueId_t core_gps_hqueue;
static osMessageQueueId_t core_radio_hqueue;
static HAL_StatusTypeDef core_hal_status;

static gps_time_type core_gps_time;

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

	rtc_time.Hours = core_gps_time.hour;
	rtc_time.Minutes = core_gps_time.minutes;
	rtc_time.Seconds = core_gps_time.seconds;
	rtc_time.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
	rtc_time.StoreOperation = RTC_STOREOPERATION_RESET;
	core_hal_status = HAL_RTC_SetTime( core_hrtc, &rtc_time, RTC_FORMAT_BIN );
	if ( core_hal_status != HAL_OK ) {
		return;
	}
	rtc_date.WeekDay = RTC_WEEKDAY_MONDAY; // TODO
	rtc_date.Month = core_gps_time.month;
	rtc_date.Date = core_gps_time.day;
	rtc_date.Year = core_gps_time.year;

	core_hal_status = HAL_RTC_SetDate( core_hrtc, &rtc_date, RTC_FORMAT_BIN );
	if ( core_hal_status != HAL_OK ) {
		return;
	}
}

void Core_Run() {
	// TODO - listen for GPS time messages, save them to core_gps_time and update the RTC
	// TODO - listen for GPS location messages and save state
	// TODO - listen for THP messages and save thp state
	// TODO - every 5 seconds build a buffer with all the data and send it to the radio to transmit

	osDelay( 200 ); // This task should sleep for 200 ms after running
}

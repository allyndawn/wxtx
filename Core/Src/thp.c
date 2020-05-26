/**
 * thp.c
 * Allen Snook
 * May 26, 2020
 *
 * Bosch BME280
 */

#include "thp.h"
#include "stm32f4xx_hal.h"
#include "bme280.h"

#define THP_STATE_UNKNOWN 0
#define THP_STATE_READY 1

static I2C_HandleTypeDef *thp_hi2c;
static osMessageQueueId_t thp_hqueue;
static struct bme280_dev thp_dev;
static struct bme280_data thp_data;

static uint8_t thp_settings = 0;
static uint8_t thp_result = 0;
static uint32_t thp_meas_delay = 0;
static uint8_t thp_state = THP_STATE_UNKNOWN;

void THP_Set_I2C( I2C_HandleTypeDef *hi2c ) {
	thp_hi2c = hi2c;
}

void THP_Set_Message_Queue( osMessageQueueId_t hqueue ) {
	thp_hqueue = hqueue;
}

void _THP_Init() {
	thp_result = bme280_init( &thp_dev );
	if ( BME280_OK == thp_result ) {
		thp_dev.settings.osr_h = BME280_OVERSAMPLING_1X;
		thp_dev.settings.osr_p = BME280_OVERSAMPLING_16X;
		thp_dev.settings.osr_t = BME280_OVERSAMPLING_2X;
		thp_dev.settings.filter = BME280_FILTER_COEFF_16;

		thp_settings = BME280_OSR_PRESS_SEL | BME280_OSR_TEMP_SEL | BME280_OSR_HUM_SEL | BME280_FILTER_SEL;

		thp_result = bme280_set_sensor_settings( thp_settings, &thp_dev );
		if ( BME280_OK == thp_result ) {
			thp_meas_delay = bme280_cal_meas_delay( &( thp_dev.settings ) );
			if ( thp_meas_delay < 100 ) {
				thp_meas_delay = 100;
			}
			thp_state = THP_STATE_READY;
		}
	}
}

void THP_Run() {
	if ( THP_STATE_UNKNOWN == thp_state ) {
		_THP_Init();
	}

	if ( THP_STATE_READY == thp_state ) {
		thp_result = bme280_set_sensor_mode( BME280_FORCED_MODE, &thp_dev );
		if ( BME280_OK == thp_result ) {
			HAL_Delay( thp_meas_delay );
			thp_result = bme280_get_sensor_data( BME280_ALL, &thp_data, &thp_dev );
			if ( BME280_OK == thp_result ) {
				// TODO send the data to core
			}
		}
	}

	osDelay( 1000 ); // This task should sleep for 1 second after running
}

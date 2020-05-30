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
static struct bme280_data thp_bme_data;

static thp_data_type thp_data;

static uint8_t thp_settings = 0;
static uint8_t thp_result = 0;
static uint32_t thp_meas_delay = 0;
static uint8_t thp_state = THP_STATE_UNKNOWN;

void _THP_Device_Delay_ms( uint32_t microsec ) {
	uint32_t millisec = microsec / 1000;
	if ( millisec < 1 ) {
		millisec = 1;
	}
	HAL_Delay( millisec );
}

int8_t _THP_Device_Read( uint8_t id, uint8_t reg_addr, uint8_t *data, uint16_t len ) {
	if ( id != thp_dev.dev_id ) {
		return BME280_E_COMM_FAIL;
	}

	HAL_StatusTypeDef halResult = HAL_I2C_Mem_Read(
		thp_hi2c,
		id << 1,
		reg_addr,
		I2C_MEMADD_SIZE_8BIT,
		data,
		len,
		100
	);

	return ( HAL_OK == halResult ? BME280_OK : BME280_E_COMM_FAIL);
}

int8_t _THP_Device_Write( uint8_t id, uint8_t reg_addr, uint8_t *data, uint16_t len ) {
	if ( id != thp_dev.dev_id ) {
		return BME280_E_COMM_FAIL;
	}

	HAL_StatusTypeDef halResult = HAL_I2C_Mem_Write(
		thp_hi2c,
		id << 1,
		reg_addr,
		I2C_MEMADD_SIZE_8BIT,
		data,
		len,
		100
	);

	return ( HAL_OK == halResult ? BME280_OK : BME280_E_COMM_FAIL);
}

void _THP_Init() {
	thp_dev.dev_id = BME280_I2C_ADDR_PRIM;
	thp_dev.intf = BME280_I2C_INTF;
	thp_dev.read = _THP_Device_Read;
	thp_dev.write = _THP_Device_Write;
	thp_dev.delay_ms = _THP_Device_Delay_ms;

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

void THP_Set_I2C( I2C_HandleTypeDef *hi2c ) {
	thp_hi2c = hi2c;
}

void THP_Set_Message_Queue( osMessageQueueId_t hqueue ) {
	thp_hqueue = hqueue;
}

void _THP_Enqueue_Data() {
	if ( ! thp_hqueue ) {
		return;
	}

	thp_data.pressure = (uint16_t) ( thp_bme_data.pressure / 10 );
	thp_data.temperature = (int16_t) ( thp_bme_data.temperature / 10 );
	thp_data.humidity = (uint16_t) ( thp_bme_data.humidity / 100 );

	osMessageQueuePut( thp_hqueue, (void *) &(thp_data), 0U, 0U );
}

void THP_Run() {
	if ( THP_STATE_UNKNOWN == thp_state ) {
		HAL_GPIO_TogglePin( GPIOB, GPIO_PIN_7 ); // Blue PB7 LD2
		_THP_Init();
	}

	if ( THP_STATE_READY == thp_state ) {
		thp_result = bme280_set_sensor_mode( BME280_FORCED_MODE, &thp_dev );
		if ( BME280_OK == thp_result ) {
			HAL_Delay( thp_meas_delay );
			thp_result = bme280_get_sensor_data( BME280_ALL, &thp_bme_data, &thp_dev );
			if ( BME280_OK == thp_result ) {
				HAL_GPIO_WritePin( GPIOB, GPIO_PIN_7, GPIO_PIN_SET ); // Blue PB7 LD2
				_THP_Enqueue_Data();
			}
		}
	}

	osDelay( 1000 ); // This task should sleep for 1 second after running
}

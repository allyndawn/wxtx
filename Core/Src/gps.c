/**
 * gps.c
 * Allen Snook
 * May 26, 2020
 *
 * Bosch BME280
 */

#include "thp.h"
#include "stm32f4xx_hal.h"
#include "string.h"
#include "stdlib.h" // for strtoul

#ifndef TRUE
#define TRUE UINT8_C(1)
#endif
#ifndef FALSE
#define FALSE UINT8_C(0)
#endif

#define GPS_STATE_UNKNOWN 0
#define GPS_STATE_READY 1

#define GPS_MAX_BUFFER_LENGTH 96
#define GPS_GPRMC_TOKENS 12
#define GPS_GPRMC_MAX_TOKEN_LENGTH 12

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

static UART_HandleTypeDef *gps_huart;
static osMessageQueueId_t gps_hqueue;

static uint8_t gps_char_rx;
static uint32_t gps_uart_flags;
static HAL_StatusTypeDef gps_hal_status;

static char gps_buffer[GPS_MAX_BUFFER_LENGTH];
static uint8_t gps_buffer_length = 0;

static char gps_scratchpad[GPS_GPRMC_TOKENS][GPS_GPRMC_MAX_TOKEN_LENGTH];
static gps_time_type gps_time;
static gps_location_type gps_location;

void GPS_Set_UART( UART_HandleTypeDef *huart ) {
	gps_huart = huart;
}

void GPS_Set_Message_Queue( osMessageQueueId_t hqueue ) {
	gps_hqueue = hqueue;
}

int8_t _GPS_Int_From_String( int8_t index, int8_t offset, int8_t length ) {
	int8_t value = 0;
	int8_t digit = 0;
	for ( int8_t i=0; i < length; i++ ) {
		value *= 10;
		digit = gps_scratchpad[index][offset + i];
		if ( digit >= '0' && digit <= '9' ) {
			value += digit - '0'; // Convert ASCII to number
		}
	}
	return value;
}

uint8_t _GPS_Process_Buffer() {
	// Is the buffer between 16 and 66 chars?
	if ( ( gps_buffer_length < 16 ) | ( gps_buffer_length > 66 ) ) {
		return FALSE;
	}

	// Is this a $GPRMRC sentence?
	if ( strncmp( "$GPRMC", gps_buffer, 6 ) != 0 ) {
		return FALSE;
	}

	// Does it have exactly 11 commas?
	uint8_t comma_count = 0;
	for ( uint8_t i=0; i < gps_buffer_length; i++ ) {
		if ( ',' == gps_buffer[i] ) {
			comma_count++;
		}
	}
	if ( 12 != comma_count ) {
		return FALSE;
	}

	// Does it have a checksum delimeter 3 characters from the end?
	if ( '*' != gps_buffer[ gps_buffer_length - 3 ] ) {
		return FALSE;
	}

	// Calculate the checksum
	// A NEMA checksum is all the characters between the $ and the *
	// XOR'd with each other and then turned into a two digit hex value
	uint8_t calculated_checksum = 0;
	for ( uint8_t i=1; i < gps_buffer_length - 3; i++ ) {
		calculated_checksum ^= (uint8_t) gps_buffer[i];
	}

	// Check the checksum
	unsigned long checksum = strtoul( gps_buffer + gps_buffer_length - 2, NULL, 16 );
	if ( checksum != calculated_checksum ) {
		return FALSE;
	}

	// If we have gotten this far, we're ready to parse the tokens
	// First, clear the tokens
	for ( int8_t i=0; i < GPS_GPRMC_TOKENS; i++ ) {
		gps_scratchpad[i][0] = 0;
	}

	int8_t tokenIndex = 0;

	// Now, tokenize into the scratchpad
	char dataChar[2];
	dataChar[0] = 0;
	dataChar[1] = 0;

	for ( uint8_t i=0; i < gps_buffer_length; i++ ) {
		if ( ',' == gps_buffer[i] ) {
			tokenIndex++;
		} else if (strlen( gps_scratchpad[tokenIndex] ) < GPS_GPRMC_MAX_TOKEN_LENGTH - 1 ) {
			dataChar[0] = gps_buffer[i];
			strcat( gps_scratchpad[tokenIndex], dataChar );
		}
	}

	// Lastly, extract all the values we want
	// Scratchpad Entry #2: Validity
	if ( 'A' != gps_scratchpad[2][0] ) {
		return FALSE;
	}

	gps_time_type new_time = { 0 };
	gps_location_type new_location = { 0 };

	// Scratchpad Entry #1: Time stamp (hhmmss.xx)
	if ( strlen( gps_scratchpad[1] ) < 6 ) {
		return FALSE;
	}
	new_time.hour = _GPS_Int_From_String( 1, 0, 2 );
	new_time.minutes = _GPS_Int_From_String( 1, 2, 2 );
	new_time.seconds = _GPS_Int_From_String( 1, 4, 2 );

	// Scratchpad Entry #9: Date stamp (ddmmyy)
	if ( strlen( gps_scratchpad[9] ) != 6 ) {
		return FALSE;
	}
	new_time.year = _GPS_Int_From_String( 9, 4, 2 );
	new_time.month = _GPS_Int_From_String( 9, 2, 2 );
	new_time.day = _GPS_Int_From_String( 9, 0, 2 );

	uint16_t seconds = 0;

	// Scratchpad Entry #3: Latitude (ddmm.xxxxx)
	if ( strlen( gps_scratchpad[3] ) < 7 ) {
		return FALSE;
	}
	if ( '.' != gps_scratchpad[3][4] ) {
		return FALSE;
	}
	new_location.latitude.degrees = _GPS_Int_From_String( 3, 0, 2 );
	new_location.latitude.minutes = _GPS_Int_From_String( 3, 2, 2 );
	seconds = _GPS_Int_From_String( 3, 5, 2 );
	seconds = seconds * 60 / 100;
	new_location.latitude.seconds = seconds;

	// Scratchpad Entry #4: North/South (N or S)
	if ( 'N' != gps_scratchpad[4][0] && 'S' != gps_scratchpad[4][0] ) {
		return FALSE;
	}
	new_location.latitude.hem = gps_scratchpad[4][0];

	// Scratchpad Entry #5: Longitude (dddmm.xxxxx)
	if ( strlen( gps_scratchpad[5] ) < 8 ) {
		return FALSE;
	}
	if ( '.' != gps_scratchpad[5][5] ) {
		return FALSE;
	}
	new_location.longitude.degrees = _GPS_Int_From_String( 5, 0, 3 );
	new_location.longitude.minutes = _GPS_Int_From_String( 5, 3, 2 );
	seconds = _GPS_Int_From_String( 5, 6, 2 );
	seconds = seconds * 60 / 100;
	new_location.longitude.seconds = seconds;

	// Scratchpad Entry #6: East/West (E or W)
	if ( 'E' != gps_scratchpad[6][0] && 'W' != gps_scratchpad[6][0] ) {
		return FALSE;
	}
	new_location.longitude.hem = gps_scratchpad[6][0];

	// All is well - persist the new time and location
	gps_time = new_time;
	gps_location = new_location;

	return TRUE;
}

void _GPS_Enqueue_Data() {
	// TODO
}

void GPS_Run() {
	do {
		// Read any error flags so reception doesn't stall
		// (Don't care if we get errors - we can just wait for the next line)
		gps_uart_flags = gps_huart->Instance->DR;
		gps_uart_flags = gps_huart->Instance->SR;

		// Receive any bits
		gps_hal_status = HAL_UART_Receive( gps_huart, &gps_char_rx, 1, 50 );
		if ( HAL_OK == gps_hal_status ) {

			// End of line? Try to process it
			if ( gps_buffer_length > 0 && ( 0x0A == gps_char_rx || 0x0D == gps_char_rx ) ) {
				if ( _GPS_Process_Buffer() ) {
					_GPS_Enqueue_Data();
				}
				gps_buffer_length = 0;
			} else {
				if ( '$' == gps_char_rx ) {
					// First character of any sentence is always $
					// If we receive a $, throw away anything we have and start over
					gps_buffer[ 0 ] = '$';
					gps_buffer_length = 1;
				} else {
					// If we have a $ already in the buffer, keep appending
					// while space permits
					if ( ( gps_buffer[ 0 ] == '$' ) && ( gps_buffer_length < GPS_MAX_BUFFER_LENGTH - 1 ) ) {
						gps_buffer[ gps_buffer_length ] = gps_char_rx;
						gps_buffer[ gps_buffer_length + 1 ] = 0;
						gps_buffer_length++;
					}
				}
			}
		}
	} while ( HAL_OK == gps_hal_status );

	osDelay( 1000 ); // This task should sleep for 1 second after running
}

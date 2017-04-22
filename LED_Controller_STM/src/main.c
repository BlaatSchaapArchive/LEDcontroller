/*
 * main.c
 *
 *  Created on: 19 apr. 2017
 *      Author: andre
 */

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "stm32f1xx.h"
#include "stm32f1xx_hal.h"

#include "stm32_common.h"

#include "usbd_core.h"
#include "usbd_led.h"
#include "usbd_desc.h"

#include "pwmdma.h"

USBD_HandleTypeDef USBD_Device;

uint8_t data_c0[3072*4]; // 4 Clockless channels of either 96 RGBW or 128 RGB leds
uint8_t data_c1[512];  // 1 Clocked channels of 96 LEDS

int8_t LED_Itf_Init(void) {
	return (USBD_OK);
}
int8_t LED_Itf_DeInit(void) {
	return (USBD_OK);
}
int8_t LED_Itf_Receive(uint8_t *buffer, uint32_t *length) {
	// First test
	// There are no sanity checks. This is ONLY A FIRST TEST.
	// Should not be used in production whatsoever!!!!!!!!!!
	uint16_t offset = *(uint16_t*) buffer;
	uint8_t  amount = *(uint8_t*) (buffer+2);
	memcpy(data_c1 + offset, buffer+3, amount);



	return (USBD_OK);

	// USBD_BUSY
}
;



USBD_LED_ItfTypeDef USBD_LED_fops = { LED_Itf_Init, LED_Itf_DeInit,
		LED_Itf_Receive };

int main() {


	HAL_Init(); // Reset of all peripherals, Initializes the Flash interface and the Systick.
	SystemClock_Config(); // Configure the system clock to 72 MHz


	usb_disconnect(); // Force re-enumeration

	USBD_Init(&USBD_Device, &VCP_Desc, 0); // Init Device Library
	USBD_RegisterClass(&USBD_Device, USBD_LED_CLASS); // Add Supported Class
	USBD_LED_RegisterInterface(&USBD_Device, &USBD_LED_fops); // Add LED Interface Class
	USBD_Start(&USBD_Device); // Start Device Process


	// Test Data
	int i = 0;
// green
	while (i<(4*24)){
		data_c0[i+0] = i < (4*8) ? 6 : 2;
		data_c0[i+1] = i < (4*8) ? 6 : 2;
		data_c0[i+2] = i < (4*8) ? 6 : 2;
		data_c0[i+3] = i < (4*8) ? 6 : 2;
		i+=4;;
	}

/*//red
	while (i<(4*24)){
		data_c0[i+0] = (i < (4*16)) && (i > (4*8) )? 6 : 2;
		data_c0[i+1] = (i < (4*16)) && (i > (4*8) )? 6 : 2;
		data_c0[i+2] = (i < (4*16)) && (i > (4*8) )? 6 : 2;
		data_c0[i+3] = (i < (4*16)) && (i > (4*8) ) ? 6 : 2;
		i+=4;;
	}

 // blue
	while (i<(4*24)){
		data_c0[i+0] = (i > (4*16) )? 6 : 2;
		data_c0[i+1] = (i > (4*16) )? 6 : 2;
		data_c0[i+2] = (i > (4*16)  )? 6 : 2;
		data_c0[i+3] = (i > (4*16) ) ? 6 : 2;
		i+=4;;
	}
*/
	while (i<(4*48)){
		data_c0[i+0] = 0;
		data_c0[i+1] = 0;
		data_c0[i+2] = 0;
		data_c0[i+3] = 0;
		i+=4;;
	}

	pwm_init();

	start_dma_transer(data_c0,4*48);
	/* Infinite loop */
	while (1) {

		while (is_busy());;
		start_dma_transer(data_c0,4*48);
	}
}


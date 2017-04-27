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

extern uint8_t data_c0[];

uint8_t data_usb_rx[64];  // USB Buffer data from pc
uint8_t data_usb_tx[64];  // USB Buffer data to pc

int8_t LED_Itf_Init(void) {
	return (USBD_OK);
}
int8_t LED_Itf_DeInit(void) {
	return (USBD_OK);
}


int8_t LED_Itf_Receive(uint8_t *buffer, uint32_t *length);

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

	pwm_init();

	//start_dma_transer(data_c0,sizeof(data_c0));
	/* Infinite loop */
	while (1) {

		//while (is_busy());;
		//start_dma_transer(data_c0,sizeof(data_c0));
	}
}


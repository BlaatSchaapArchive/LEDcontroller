/*
 * usb_led.c
 *
 *  Created on: 27 apr. 2017
 *      Author: andre
 */

#include "protocol.h"
#include "devinfo.h"
#include <string.h>



#include "usbd_core.h"


extern USBD_HandleTypeDef USBD_Device;
extern uint8_t data_usb_tx[64];

uint8_t data_c0[3072*4]; // 4 Clockless channels of either 96 RGBW or 128 RGB leds
uint8_t data_c1[512];  // 1 Clocked channels of 96 LEDS

bool buffer_state[2] = {false, false} ;
bool timer_state[3] = {false, false, false};

int8_t LED_Itf_Receive(uint8_t *buffer, uint32_t *length) {

	uint8_t command = buffer[0];

	switch (command) {
	case CMD_NONE:
		break;
	case CMD_PING:
		// todo send buffer back to host
		break;
	case CMD_DEVINFO: {

		devinfo_t info;
		info.architecture = EM_ARM;
		info.vendor       = VE_STM;
		info.device       = DBGMCU->IDCODE;


		UID_BASE; //read 96 bits


	}
		break;
	case CMD_CONFIG:
		break;
	case CMD_FILL_BUFFER: {
		uint8_t target = *(uint8_t*) (buffer+1);
		uint16_t offset = *(uint16_t*) (buffer+2);
		uint8_t  amount = *(uint8_t*) (buffer+4);

		switch (target) {
		case 0:
			if ((offset + amount) <  (sizeof(data_c0))) {
				memcpy(data_c0 + offset, buffer+5, amount);
			} else {
				// TODO signal error to host
				// invalid offset/amount, buffer overflow
			}
			break;
		case 1:
			if ((offset + amount) <  (sizeof(data_c1))) {
				memcpy(data_c1 + offset, buffer+5, amount);
			} else {
				// TODO signal error to host
				// invalid offset/amount, buffer overflow
			}

			break;
		default:
			// TODO signal error to host
			// Invalid buffer specified
			break;
		}
		}
		break;
	case CMD_APPLY_BUFFER: {
		uint8_t data_src = *(uint8_t*) (buffer+1);
		uint8_t data_dst = *(uint8_t*) (buffer+2);

		// TODO implement source and destination
		// For now, we only have one so ignore them

		start_dma_transer(data_c0,sizeof(data_c0));


		break;
	}
	case CMD_BUFFER_STATE: {
		// we need to reply something.....
		/// implement this
		data_usb_tx[0] = CMD_BUFFER_STATE;
		data_usb_tx[1] = is_busy();
		USBD_LED_TransmitPacket(&USBD_Device);
	}
	default:
		break;
	}



	return (USBD_OK);

	// USBD_BUSY
}
;

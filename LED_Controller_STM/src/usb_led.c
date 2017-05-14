/*
 * usb_led.c
 *
 *  Created on: 27 apr-> 2017
 *      Author: andre
 */

#include "protocol.h"
#include "devinfo.h"
#include <string.h>

#include "usbd_core.h"



void start_dma_transer2(char* memory, size_t size, DMA_Channel_TypeDef *dma,
		TIM_TypeDef *tim);

extern USBD_HandleTypeDef USBD_Device;
extern uint8_t data_usb_tx[64];

uint8_t data_c0[3072 * 4]; // 4 Clockless channels of either 96 RGBW or 128 RGB leds
uint8_t data_c1[512];  // 1 Clocked channels of 96 LEDS

bool buffer_state[2] = { false, false };
bool timer_state[3] = { false, false, false };


static int bits_per_colour = 8;
static int channels_per_timer = 4;


//For led strips
static int pwm_val_0 = 2;
static int pwm_val_1 = 6;



// For through hole leds??
//static int pwm_val_0 = 3;
//static int pwm_val_1 = 13;


bool set_leds(int offset, int channel_nr, int byteval) {
	for (int bit_nr = 0; bit_nr < bits_per_colour; bit_nr++) {
		int mask = 1 << bit_nr;
		uint8_t val = (byteval & mask) ? pwm_val_1 : pwm_val_0;
		uint32_t index = (offset * channels_per_timer)
				+ (((bits_per_colour - 1) - bit_nr) * channels_per_timer)
				+ channel_nr;
		if (index > sizeof(data_c0))
			return false;
		data_c0[index] = val;
	}
	return true;
}




bool set_led3(int led_nr, int channel_nr, int colour_index, int byteval) {
	int bits_per_led = 3 * bits_per_colour;
	for (int bit_nr = 0; bit_nr < bits_per_colour; bit_nr++) {
		int mask = 1 << bit_nr;
		uint8_t val = (byteval & mask) ? pwm_val_1 : pwm_val_0;
		uint32_t index = (led_nr * bits_per_led * channels_per_timer)
				+ (((bits_per_colour - 1) - bit_nr) * channels_per_timer)
				+ channel_nr
				+ ((colour_index * bits_per_colour) * channels_per_timer);
		if (index > sizeof(data_c0))
			return false;
		data_c0[index] = val;
	}
	return true;
}


bool set_led4(int led_nr, int channel_nr, int colour_index, int byteval) {
	int bits_per_led = 4 * bits_per_colour;
	for (int bit_nr = 0; bit_nr < bits_per_colour; bit_nr++) {
		int mask = 1 << bit_nr;
		uint8_t val = (byteval & mask) ? pwm_val_1 : pwm_val_0;
		uint32_t index = (led_nr * bits_per_led * channels_per_timer)
				+ (((bits_per_colour - 1) - bit_nr) * channels_per_timer)
				+ channel_nr
				+ ((colour_index * bits_per_colour) * channels_per_timer);
		if (index > sizeof(data_c0))
			return false;
		data_c0[index] = val;
	}
	return true;
}





int8_t LED_Itf_Receive(uint8_t *buffer, uint32_t *length) {

	uint8_t command = buffer[0];

	switch (command) {
	case CMD_NONE:
		break;
	case CMD_PING:
		// todo send buffer back to host
		break;
	case CMD_DEVINFO: {


		devinfo_response_t *r = (devinfo_response_t *)data_usb_tx;
		r->command = CMD_DEVINFO;
		r->size = sizeof(devinfo_response_t); //!!

		r->info.size = sizeof(devinfo_t);
		r->info.type = DEVINFO_MCU;
		r->info.architecture = EM_ARM;
		r->info.vendor = VE_STM;
		r->info.device = DBGMCU->IDCODE;

		r->led0.size = sizeof(led_dev_t);
		r->led0.type = DEVINFO_LED;
		r->led0.channels = 4;
		r->led0.implementation = LED_C0_TYPE_PWM_DMA;
		r->led0.count = 2;
		r->led0.buffer = 0;

		// APA102 style leds are not yet implemented
		r->led0.size = sizeof(led_dev_t);
		r->led0.type = DEVINFO_LED;
		r->led0.channels = 0;
		r->led0.implementation = LED_C1_TYPE_UNDEFINED;
		r->led0.count = 0;
		r->led0.buffer = 1;

		r->buf0.size = sizeof(buffer_t);
		r->buf0.type = DEVINFO_BUF;
		r->buf0.buffer_size = sizeof (data_c0);
		r->buf0.shared = 1;

		r->buf1.size = sizeof(buffer_t);
		r->buf1.type = DEVINFO_BUF;
		r->buf1.buffer_size = sizeof (data_c1);
		r->buf1.shared = 1;


		// Using LL it works
		//USBD_LL_Transmit(&USBD_Device, 0x81, r, sizeof(devinfo_response_t));

		// THe TransmitPacker it wrong, as it uses an internal buffer, which is *NOT*
		// the "extern uint8_t data_usb_tx[64];" declared above. As said, we need to
		// rewrite the class interface using the template. Nevertheless, sending works fine.

		//USBD_LED_TransmitPacket(&USBD_Device);
		// we need to indicate the buffer and sharing

		// Buffer + size

		// HW PWM Generator 4 channel
		// buffer layout / configuration
		// assigned buffer/sharing


		// Buffer + size

		// SPI / USART
		// configuration
		// assigned buffer/sharing
		// amount

		//UID_BASE; //read 96 bits

	}
		break;
	case CMD_CONFIG:
		break;
	case CMD_FILL_BUF_RAW: {
		uint8_t target = *(uint8_t*) (buffer + 1);
		uint16_t offset = *(uint16_t*) (buffer + 2);
		uint8_t amount = *(uint8_t*) (buffer + 4);

		switch (target) {
		case 0:
			if ((offset + amount) < (sizeof(data_c0))) {
				memcpy(data_c0 + offset, buffer + 5, amount);
			} else {
				// TODO signal error to host
				// invalid offset/amount, buffer overflow
			}
			break;
		case 1:
			if ((offset + amount) < (sizeof(data_c1))) {
				memcpy(data_c1 + offset, buffer + 5, amount);
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
		uint8_t data_src = *(uint8_t*) (buffer + 1);
		uint8_t data_dst = *(uint8_t*) (buffer + 2);

		if (data_src!=0) break;

		switch (data_dst) {
		case 0:
			if (!is_busy2(DMA1_Channel2))
				start_dma_transer2(data_c0, sizeof(data_c0), DMA1_Channel2, TIM2);
			break;
		case 1:
			if (!is_busy2(DMA1_Channel3))
				start_dma_transer2(data_c0, sizeof(data_c0), DMA1_Channel3, TIM3);
			break;
		case 2:
			if (is_busy2(DMA1_Channel7))
				start_dma_transer2(data_c0, sizeof(data_c0), DMA1_Channel7, TIM4);
			break;
		default: break;
		}
		//

		break;
	}
	case CMD_BUFFER_STATE: {
		// we need to reply something.....
		/// TODO implement this
		data_usb_tx[0] = CMD_BUFFER_STATE;
		data_usb_tx[1] = is_busy();
		USBD_LED_TransmitPacket(&USBD_Device);
		break;
	}
	case CMD_FILL_BUFFER3: {
		uint8_t target = buffer[1];
		uint8_t offset = buffer[2];
		uint8_t amount = buffer[3];

		if (target >= 4) break; // invalid channel id

		// buffers are verified in set_ledl* thus offset/amount
		// do not require verification
		int b = 4;
		for (int i = 0; i < amount; i++) {
			for (int colour = 0; colour < 3; colour++)
				set_led3(offset + i, target, colour, buffer[b++]);
		}
		break;
	case CMD_FILL_BUFFER4: {
		uint8_t target = buffer[1];
		uint8_t offset = buffer[2];
		uint8_t amount = buffer[3];

		if (target >= 4) break; // invalid channel id

		// offset and amount are verified in set_pixel4
		int b = 4;
		for (int i = 0; i < amount; i++) {
			for (int colour = 0; colour < 4; colour++)
				set_led4(offset + i, target, colour, buffer[b++]);
		}
		break;
	}
	case CMD_FILL_BUFFER: {
		uint8_t target = buffer[1];
		uint8_t offset = buffer[2];
		uint8_t amount = buffer[3];

		if (target >= 4) break; // invalid channel id

		// offset and amount are verified in set_pixel4
		for (int i = 0; i < amount; i++) {
				set_leds(offset + i, target, buffer[i]);
		}
		break;
	}




	}
	default:
		break;
	}

	return (USBD_OK);

	// USBD_BUSY
}
;

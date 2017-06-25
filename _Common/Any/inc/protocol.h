#ifndef _PROTOCOL_H_
#define _PROTOCOL_H_

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#pragma pack(push,1) // byte alignment


enum {
	CMD_NONE     = 0x00,
	CMD_PING     = 0x01,
	CMD_DEVINFO  = 0x02,
	CMD_CONFIG   = 0x03,

	CMD_FILL_BUF_RAW = 0x10,
	CMD_APPLY_BUFFER = 0x11,
	CMD_BUFFER_STATE = 0x12,


	CMD_FILL_BUFFER3 = 0x13,
	CMD_FILL_BUFFER4 = 0x14,
	CMD_FILL_BUFFER  = 0x15,

	CMD_FILL_BUFFER_SC = 0x16,
	CMD_APPLY_BUF_SC = 0x17,
};


enum {
	DEVINFO_MCU = 0x10,

	DEVINFO_BUF = 0x11,



	DEVINFO_LED = 0xA0,
	DEVINFO_CUR = 0xA1,



};




typedef struct {
	uint8_t size;
	uint8_t type;
	uint16_t architecture;
	uint16_t vendor;
	uint32_t device;
} devinfo_t;

typedef struct {
	uint8_t size;
	uint8_t type;
	uint8_t channels;
	uint8_t implementation;
	uint8_t count;
	uint8_t buffer;
} led_dev_t;

typedef struct {
	uint8_t size;
	uint8_t type;
	uint16_t buffer_size;
	uint8_t shared;
} buffer_t;

typedef struct {
	uint8_t command;
	uint8_t size;
	devinfo_t info;
	buffer_t  buf0;
	buffer_t  buf1;
	led_dev_t led0;
	led_dev_t led1;
	uint8_t pad;

} devinfo_response_t;


enum {
	DEV_LED_C0_CHANNEL = 0x20,
	DEV_LED_C1_CHANNEL = 0x21,
};

#define LED_C0_MASK 0x10
#define LED_C1_MASK 0x20

// Suppose we could make this different, set some bit masks rather
enum {
	LED_C0_TYPE_UNDEFINED = 0x10,
	LED_C0_TYPE_SW = 0x11,
	LED_C0_TYPE_TA = 0x12,
	LED_C0_TYPE_PWM_DMA = 0x13,
	LED_C0_TYPE_PWM_IT = 0x14,

	LED_C1_TYPE_UNDEFINED 	= 0x20,
	LED_C1_TYPE_SW 		= 0x21,
	LED_C1_TYPE_TA = 0x22,
	LED_C1_TYPE_SPI   = 0x23,
	LED_C1_TYPE_USART = 0x24,
};


typedef struct {
	uint8_t command;
	union {
		uint8_t raw_data[63];
	};
} packet_t;


#pragma pack(pop) // alignment whatever it was

#endif // _PROTOCOL_H_

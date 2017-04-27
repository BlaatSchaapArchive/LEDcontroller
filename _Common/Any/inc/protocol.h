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

	CMD_FILL_BUFFER = 0x10,
	CMD_APPLY_BUFFER = 0x11,
	CMD_BUFFER_STATE = 0x12,
};


typedef struct {
	uint16_t architecture;
	uint16_t vendor;
	uint32_t device;
} devinfo_t;


// define capabilities?


enum {
	DEV_LED_C0_CHANNEL = 0x20,
	DEV_LED_C1_CHANNEL = 0x21,
};

enum {
	C0_TYPE_BB_BW = 0x00,
	C0_TYPE_BB_TA = 0x01,
	C0_TYPE_PWM_DMA = 0x10,
	C0_TYPE_PWM_IT = 0x11,
};

enum {
	C1_TYPE_BB_BW = 0x00,
	C1_TYPE_BB_TA = 0x01,
	C1_TYPE_SPI   = 0x20,
	C1_TYPE_USART   = 0x30,
};

typedef struct {
	uint8_t dev_type;
	uint8_t channels;
	uint8_t data_width;
	uint8_t date_interleaved;
} c0_dev_t;

typedef struct {
	uint8_t command;
	union {
		uint8_t raw_data[63];
	};
} packet_t;


#pragma pack(pop) // alignment whatever it was

#endif // _PROTOCOL_H_

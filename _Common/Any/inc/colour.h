#ifndef _COLOUR_H_
#define _COLOUR_H_

typedef enum {
	rgb,
	rbg,
	grb,
	gbr,
	brg,
	bgr,
} rgb_permurations_t;


#pragma pack(push,1) // byte alignment
typedef struct {
	uint8_t r; // red
	uint8_t g; // green
	uint8_t b; // blue
} rgb_t;


typedef struct {
	uint8_t r; // red
	uint8_t g; // green
	uint8_t b; // blue
	uint8_t w;	// white
} rgbw_t;

typedef struct {
	uint8_t d; // dim (apa102 style)
	uint8_t r; // red
	uint8_t g; // green
	uint8_t b; // blue
} drgb_t;
#pragma pack(pop) // alignment whatever it was

#endif

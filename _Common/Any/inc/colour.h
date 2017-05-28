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


// Even though the SK6812RGBW says the sequence is RGBW, it turns out it is in fact GRBW.
// Therefore the same style permutation code as for RGB is to be implemented. As 4! is 24,
// I didn't feel like permuting them manually, thus I used an javascript tool
// 		http://users.telenet.be/vdmoortel/dirk/Maths/permutations.html
// This explains the odd sequence
typedef enum {
	bgrw,
	bgwr,
	brgw,
	brwg,
	bwgr,
	bwrg,
	gbrw,
	gbwr,
	grbw,
	grwb,
	gwbr,
	gwrb,
	rbgw,
	rbwg,
	rgbw,
	rgwb,
	rwbg,
	rwgb,
	wbgr,
	wbrg,
	wgbr,
	wgrb,
	wrbg,
	wrgb,
} rgbw_permurations_t;

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

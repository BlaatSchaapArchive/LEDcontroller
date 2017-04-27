#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "libusb.h"

/*

 int libusb_bulk_transfer 	( 	struct libusb_device_handle *  	dev_handle,
 unsigned char  	endpoint,
 unsigned char *  	data,
 int  	length,
 int *  	transferred,
 unsigned int  	timeout
 )
 */

typedef struct {
	uint8_t r;
	uint8_t g;
	uint8_t b;
} rgb_t;

// https://blog.adafruit.com/2012/03/14/constant-brightness-hsb-to-rgb-algorithm/
void hsb2rgbAN2(uint16_t index, uint8_t sat, uint8_t bright, rgb_t* color) {
	uint8_t temp[5], n = (index >> 8) % 3;
// %3 not needed if input is constrained, but may be useful for color cycling and/or if modulo constant is fast
	uint8_t x = ((((index & 255) * sat) >> 8) * bright) >> 8;
// shifts may be added for added speed and precision at the end if fast 32 bit calculation is available
	uint8_t s = ((256 - sat) * bright) >> 8;
	temp[0] = temp[3] = s;
	temp[1] = temp[4] = x + s;
	temp[2] = bright - x;
	color->r = temp[n + 2];
	color->g = temp[n + 1];
	color->b = temp[n];
}

void verify_data(uint8_t *data) {
	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++)
			printf("0x%02hhx ", data[j + (8 * i)]);
		printf("\n");
	}

}

int main() {
	int res = 0; /* return codes from libusb functions */
	libusb_device_handle* handle = 0; /* handle for USB device */
	int kernelDriverDetached = 0; /* Set to 1 if kernel driver detached */
	int numBytes = 0; /* Actual bytes transferred. */
	uint8_t buffer[64]; /* 64 byte transfer buffer */

	/* Initialise libusb. */
	res = libusb_init(0);
	if (res != 0) {
		fprintf(stderr, "Error initialising libusb.\n");
		return 1;
	}

	/* Get the first device with the matching Vendor ID and Product ID. If
	 * intending to allow multiple demo boards to be connected at once, you
	 * will need to use libusb_get_device_list() instead. Refer to the libusb
	 * documentation for details. */
	handle = libusb_open_device_with_vid_pid(0, 0x16c0, 0x05dc);
	if (!handle) {
		fprintf(stderr, "Unable to open device.\n");
		return 1;
	}

	/* Check whether a kernel driver is attached to interface #0. If so, we'll
	 * need to detach it.
	 */
	if (libusb_kernel_driver_active(handle, 0)) {
		res = libusb_detach_kernel_driver(handle, 0);
		if (res == 0) {
			kernelDriverDetached = 1;
		} else {
			fprintf(stderr, "Error detaching kernel driver.\n");
			return 1;
		}
	}

	/* Claim interface #0. */
	res = libusb_claim_interface(handle, 0);
	if (res != 0) {
		fprintf(stderr, "Error claiming interface.\n");
		return 1;
	}

//--

// insert message we want to send

	printf("Let's test something!!!\n");

	uint8_t data_c0[3072 * 4]; // 4 Clockless channels of either 96 RGBW or 128 RGB leds
	int index = 0;
	rgb_t rgb;
	int test = 0;
	for (int i = 0; i < 60; i++) {

		hsb2rgbAN2(index += 8 % 768, 255, 16, &rgb);
		//hsb2rgbAN2(index += 8 % 768, 255, 64, &rgb);
		for (int j = 0; j < 8; j++) {
			int mask = 1 << j;
			int valR = rgb.r & mask ? 6 : 2;
			int valG = rgb.g & mask ? 6 : 2;
			int valB = rgb.b & mask ? 2 : 2;

			for (int k = 0; k < 4; k++) {
				// 4 channels
				data_c0[(i * 24 * 4) + ((7 - j) * 4) + k] = valG;
				data_c0[(i * 24 * 4) + ((7 - j) * 4) + k + (8 * 4)] = valR;
				data_c0[(i * 24 * 4) + ((7 - j) * 4) + k + (16 * 4)] = valB;
			}
		}
	}

	// Test Data to indentify channels
	int i = 0;

	while (i < (4 * 24)) {
		data_c0[i + 0] = i < (4 * 8) ? 6 : 2;
		data_c0[i + 1] = (i < (4 * 16)) && (i > (4 * 8)) ? 6 : 2;
		data_c0[i + 2] = (i > (4 * 16)) ? 6 : 2;
		data_c0[i + 3] = 6;
		i += 4;
		;
	}

	int bytes_to_go = 60 * 4 * 24;
	printf("Bytes to go %d\n", bytes_to_go);
	char data_to_transmit[64];
	data_to_transmit[0] = 0x10; // FILL BUFFER
	data_to_transmit[1] = 0x00;  // TARGET BUFFER0
//  int bytes_done = 0;
	uint16_t *offset;
	offset = (uint16_t*) (data_to_transmit + 2);
	*offset = 0;
	uint8_t *bytes_of_data = (data_to_transmit + 4);
	*bytes_of_data = 64 - 5;
	while (*(data_c0 + *offset) && (bytes_to_go > *bytes_of_data)) {
		memcpy(data_to_transmit + 5, data_c0 + *offset, *bytes_of_data);

		verify_data(data_to_transmit);
		int transferred;
		int res = libusb_bulk_transfer(handle, 0x01, data_to_transmit, 64,
				&transferred, 1000);
		printf("Transferred %d Result %d\n", transferred, res);
		if (res) {
			printf("%s\n", libusb_strerror(res));
			break; // breaking out,
		}
		*offset += *bytes_of_data; // this goes wrong
		bytes_to_go -= *bytes_of_data;
		printf("Bytes to go %d\n", bytes_to_go);

	}


	 if (bytes_to_go) {
	 printf("Remaining data\n");
	 *bytes_of_data = bytes_to_go;
	 memcpy(data_to_transmit + 5, data_c0 + *offset, *bytes_of_data);

	 verify_data(data_to_transmit);
	 int transferred;
	 int res= libusb_bulk_transfer 	( handle, 0x01, data_to_transmit, 64,	&transferred, 1000 ) 	;
	 printf("Transferred %d Result %d\n",transferred, res);
	 if (res) {
	 printf("%s\n",libusb_strerror(res));
	 }
	 }


	int transferred;
	data_to_transmit[0] = 0x11; // APPLY BUFFER
	printf("Apply\n");
	res = libusb_bulk_transfer(handle, 0x01, data_to_transmit, 3, &transferred,
			1000);
	verify_data(data_to_transmit);



	/* Release interface #0. */
	res = libusb_release_interface(handle, 0);
	if (0 != res) {
		fprintf(stderr, "Error releasing interface.\n");
	}

	/* If we detached a kernel driver from interface #0 earlier, we'll now
	 * need to attach it again.  */
	if (kernelDriverDetached) {
		libusb_attach_kernel_driver(handle, 0);
	}

	/* Shutdown libusb. */
	libusb_exit(0);

	return 0;

}
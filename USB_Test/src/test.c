#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "libusb.h"

#include "permutations.h"

/*

 int libusb_bulk_transfer 	( 	struct libusb_device_handle *  	dev_handle,
 unsigned char  	endpoint,
 unsigned char *  	data,
 int  	length,
 int *  	transferred,
 unsigned int  	timeout
 )
 */


libusb_device_handle* handle = 0; /* handle for USB device */


#pragma pack(push,1) // byte alignment
typedef struct {
	uint8_t r;
	uint8_t g;
	uint8_t b;
} rgb_t;
#pragma pack(pop) // alignment whatever it was


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
	return;
	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++)
			printf("0x%02hhx ", data[j + (8 * i)]);
		printf("\n");
	}

}


bool is_supported_device(libusb_device_handle *handle) {

	libusb_device *dev = libusb_get_device(handle);
	struct libusb_device_descriptor desc;
	uint8_t string[256];
	int retval;
	retval = libusb_get_device_descriptor(dev, &desc);
	if (retval < 0) {
		fprintf(stderr, "failed to get device descriptor");
		return false;
	}

	retval = libusb_get_string_descriptor_ascii(handle, desc.iManufacturer, string, sizeof(string));
	printf("Got %s\n", string);

	if (strcmp("The IT Philosoher (https://www.philosopher.it)", string)) return false;

	retval = libusb_get_string_descriptor_ascii(handle, desc.iProduct, string, sizeof(string));
	printf("Got %s\n", string);

	uint8_t cmpName[256];
	for (int i = 0; i < 0xff; i++) {
		sprintf(cmpName,"LED Controller (variant %02x)",i);
		if (!strcmp(cmpName,string)) return true;
	}

	return false;

}


void permute_rgb_data(rgb_t* data, size_t size, rgb_permurations_t permutation) {
	if (permutation == rgb) return;

	rgb_t data_orig[size];
	memcpy(data_orig, data, sizeof(data_orig));
	for (int i = 0 ; i < size; i++) {
		switch (permutation) {
		case rbg:
			data[i].r = data_orig[i].r;
			data[i].g = data_orig[i].b;
			data[i].b = data_orig[i].g;
			break;
		case grb:
			data[i].r = data_orig[i].g;
			data[i].g = data_orig[i].r;
			data[i].b = data_orig[i].b;
			break;
		case gbr:
			data[i].r = data_orig[i].g;
			data[i].g = data_orig[i].b;
			data[i].b = data_orig[i].r;
			break;
		case brg:
			data[i].r = data_orig[i].b;
			data[i].g = data_orig[i].r;
			data[i].b = data_orig[i].g;
			break;
		case bgr:
			data[i].r = data_orig[i].b;
			data[i].g = data_orig[i].g;
			data[i].b = data_orig[i].r;
			break;
		}

	}

}


void send_rgb(rgb_t* data, size_t size, int offset) {
	// test compressed protocol
	permute_rgb_data(data,size,grb);
	char data_to_transmit[64];
	data_to_transmit[0] = 0x13; // FILL BUFFER, COMPRESSED DATA GRB
	data_to_transmit[1] = 0x00;  // TARGET CHANNEL 0
	data_to_transmit[2] = offset;// offset in leds
	data_to_transmit[3] = 20; // 20 LEDS per packet
	int res = 0;
	int to_go  = size;
	int transferred;
	rgb_t *rgb_data = (rgb_t *) data;
	while (to_go > 20) {
		memcpy(data_to_transmit + 4, rgb_data + data_to_transmit[2] - offset, 60);
		to_go -= 60;
		//offset += 60;
		data_to_transmit[2] += 20;
		res = libusb_bulk_transfer(handle, 0x01, data_to_transmit, 64,
				&transferred, 1000);
	}
	data_to_transmit[3] = to_go;
	memcpy(data_to_transmit + 4, rgb_data + data_to_transmit[2] - offset, to_go * sizeof (rgb_t));
	res = libusb_bulk_transfer(handle, 0x01, data_to_transmit, 64,
					&transferred, 1000);


	// Transmit remaining leds


	data_to_transmit[0] = 0x11; // APPLY BUFFER
	res = libusb_bulk_transfer(handle, 0x01, data_to_transmit, 3, &transferred,
			1000);



}

int main() {
	int res = 0; /* return codes from libusb functions */

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


	bool mayContinue = is_supported_device(handle);

	printf("May we continue: %d\n", mayContinue);




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

    struct timeval t1, t2;
    double elapsedTime;
    gettimeofday(&t1, NULL);

	int test1234 = 0;


	//while (1)
	{
		rgb_t rgb_data[20];
		rgb_data[0].r=0xff;

		rgb_data[1].g=0xff;

		rgb_data[2].b=0xff;

		send_rgb(rgb_data,4, 0);


	}

//

	gettimeofday(&t2, NULL);

    elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;      // sec to ms
    elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;   // us to ms
    printf ( "%f ms  / %f fps\n" ,elapsedTime, (1000 / elapsedTime)*75);


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

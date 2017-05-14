#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "libusb.h"

#include "colour.h"

// network stuff
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#include "protocol.h"

libusb_device_handle* handle = 0; /* handle for USB device */

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

	retval = libusb_get_string_descriptor_ascii(handle, desc.iManufacturer,
			string, sizeof(string));
	printf("Got %s\n", string);

	if (strcmp("The IT Philosoher (https://www.philosopher.it)", string))
		return false;

	retval = libusb_get_string_descriptor_ascii(handle, desc.iProduct, string,
			sizeof(string));
	printf("Got %s\n", string);

	uint8_t cmpName[256];
	for (int i = 0; i < 0xff; i++) {
		sprintf(cmpName, "LED Controller (variant %02x)", i);
		if (!strcmp(cmpName, string))
			return true;
	}

	return false;

}

void permute_rgb_data(rgb_t* data, size_t size, rgb_permurations_t permutation) {
	if (permutation == rgb)
		return;

	rgb_t data_orig[size];
	memcpy(data_orig, data, sizeof(data_orig));
	for (int i = 0; i < size; i++) {
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

void send_rgb(rgb_t* data, size_t size, int offset, int channel, int unit, rgb_permurations_t permutation) {
	// Create a local copy of the data, as we are going to modify the data
	// using permute_rgb_data. Using a copy, the same data buffer can be re-used.
	rgb_t rgb_data[size];
	memcpy(rgb_data, data, sizeof(rgb_data));

	permute_rgb_data(rgb_data, size, permutation);

	char send_buffer[64];
	send_buffer[0] = 0x13; // FILL BUFFER, COMPRESSED DATA GRB
	send_buffer[1] = channel;  // TARGET CHANNEL 0
	send_buffer[2] = offset;  // offset in leds
	send_buffer[3] = 20; // 20 LEDS per packet ( 60 / 3 )
	int retval = 0;
	int leds_remaining = size;
	int transferred;
	while (leds_remaining > 20) {
		memcpy(send_buffer + 4, rgb_data + send_buffer[2] - offset,
				60);
		leds_remaining -= 20;
		retval = libusb_bulk_transfer(handle, 0x01, send_buffer, 64,
				&transferred, 1000);
		send_buffer[2] += 20;
	}
	// Transmit remaining leds
	send_buffer[3] = leds_remaining;
	memcpy(send_buffer + 4, rgb_data + send_buffer[2] - offset,
			leds_remaining * sizeof(rgb_t));
	retval = libusb_bulk_transfer(handle, 0x01, send_buffer, 64, &transferred,
			1000);

	// TODO: buffered mode, separate apply

	send_buffer[0] = 0x11; // APPLY BUFFER
	send_buffer[1] = 0x00; // BUFFER 0
	send_buffer[2] = unit; // GENERATOR UNIT
	retval = libusb_bulk_transfer(handle, 0x01, send_buffer, 3, &transferred,
			1000);

}


void send_rgb2(rgb_t* data, size_t size, int offset, int channel, int unit, rgb_permurations_t permutation) {

	// todo: implement using command 0x15, send data in bytes, regardless of data layout
	//

	rgb_t rgb_data[size];
	memcpy(rgb_data, data, sizeof(rgb_data));

	permute_rgb_data(rgb_data, size, permutation);

	char send_buffer[64];
	send_buffer[0] = 0x13; // FILL BUFFER, COMPRESSED DATA GRB
	send_buffer[1] = channel;  // TARGET CHANNEL 0
	send_buffer[2] = offset;  // offset in leds
	send_buffer[3] = 20; // 20 LEDS per packet ( 60 / 3 )
	int retval = 0;
	int leds_remaining = size;
	int transferred;
	while (leds_remaining > 20) {
		memcpy(send_buffer + 4, rgb_data + send_buffer[2] - offset,
				60);
		leds_remaining -= 20;
		retval = libusb_bulk_transfer(handle, 0x01, send_buffer, 64,
				&transferred, 1000);
		send_buffer[2] += 20;
	}
	// Transmit remaining leds
	send_buffer[3] = leds_remaining;
	memcpy(send_buffer + 4, rgb_data + send_buffer[2] - offset,
			leds_remaining * sizeof(rgb_t));
	retval = libusb_bulk_transfer(handle, 0x01, send_buffer, 64, &transferred,
			1000);

	// TODO: buffered mode, separate apply

	send_buffer[0] = 0x11; // APPLY BUFFER
	send_buffer[1] = 0x00; // BUFFER 0
	send_buffer[2] = unit; // GENERATOR UNIT
	retval = libusb_bulk_transfer(handle, 0x01, send_buffer, 3, &transferred,
			1000);

}





void send_rgbw(rgbw_t* rgbw_data, size_t size, int offset, int channel, int unit) {


	char send_buffer[64];
	send_buffer[0] = 0x14; // FILL BUFFER, COMPRESSED DATA GRBW
	send_buffer[1] = channel;  // TARGET CHANNEL 0
	send_buffer[2] = offset;  // offset in leds
	send_buffer[3] = 15; // 15 LEDS per packet ( 60 / 4 )
	int retval = 0;
	int leds_remaining = size;
	int transferred;
	while (leds_remaining > 15) {
		memcpy(send_buffer + 4, rgbw_data + send_buffer[2] - offset,
				60);
		leds_remaining -= 15;
		retval = libusb_bulk_transfer(handle, 0x01, send_buffer, 64,
				&transferred, 1000);
		send_buffer[2] += 15;
	}
	// Transmit remaining leds
	send_buffer[3] = leds_remaining;
	memcpy(send_buffer + 4, rgbw_data + send_buffer[2] - offset,
			leds_remaining * sizeof(rgbw_t));
	retval = libusb_bulk_transfer(handle, 0x01, send_buffer, 64, &transferred,
			1000);

	// TODO: buffered mode, separate apply

	send_buffer[0] = 0x11; // APPLY BUFFER
	send_buffer[1] = 0x00; // BUFFER 0
	send_buffer[2] = unit; // GENERATOR UNIT
	retval = libusb_bulk_transfer(handle, 0x01, send_buffer, 3, &transferred,
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
	{
	char data_buffer[64];
	data_buffer[0] = 0x02; // FILL BUFFER, COMPRESSED DATA GRB
	data_buffer[1] = 0x00; // FILL BUFFER, COMPRESSED DATA GRB
	int transferred;
	res = libusb_bulk_transfer(handle, 0x01, data_buffer, 1, &transferred,
			1000);
	res = libusb_bulk_transfer(handle, 0x81, data_buffer, 64, &transferred,
				1000);

	// well... this is cheating... I should determine the type by code
		devinfo_response_t *devinfo = (devinfo_response_t *) (data_buffer);
		printf( " ARCHITECTURE %02x \n" ,devinfo->info.architecture);
		printf( " CHIP VENDOR %02x \n" ,devinfo->info.vendor);
		printf( " CHIP TYOE %04x \n" ,devinfo->info.device);

	}

	//

	uint8_t data_c0[3072 * 4]; // 4 Clockless channels of either 96 RGBW or 128 RGB leds
	int index = 0;
	rgb_t rgb;
	int test = 0;

	struct timeval t1, t2;
	double elapsedTime;
	gettimeofday(&t1, NULL);

	// now we should so dome server stuff
	{

#define BUFSIZE 1024

		int parentfd; /* parent socket */
		int childfd; /* child socket */
		int portno; /* port to listen on */
		int clientlen; /* byte size of client's address */
		struct sockaddr_in serveraddr; /* server's addr */
		struct sockaddr_in clientaddr; /* client addr */
		struct hostent *hostp; /* client host info */
		uint8_t buf[BUFSIZE]; /* message buffer */
		char *hostaddrp; /* dotted decimal host addr string */
		int optval; /* flag value for setsockopt */
		int n; /* message byte size */

		/*
		 * socket: create the parent socket
		 */
		parentfd = socket(AF_INET, SOCK_STREAM, 0);
		if (parentfd < 0)
			error("ERROR opening socket");

		bzero((char *) &serveraddr, sizeof(serveraddr));

		/* this is an Internet address */
		serveraddr.sin_family = AF_INET;

		/* let the system figure out our IP address */
		serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);

		/* this is the port we will listen on */
		serveraddr.sin_port = htons(7890);

		if (bind(parentfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr))
				< 0)
			error("ERROR on binding");

		/*
		 * listen: make this socket ready to accept connection requests
		 */
		if (listen(parentfd, 5) < 0) /* allow 5 requests to queue up */
			error("ERROR on listen");

		/*
		 * main loop: wait for a connection request, echo input line,
		 * then close connection.
		 */
		clientlen = sizeof(clientaddr);
		while (1) {

			/*
			 * accept: wait for a connection request
			 */
			childfd = accept(parentfd, (struct sockaddr *) &clientaddr,
					&clientlen);
			if (childfd < 0)
				error("ERROR on accept");

			/*
			 * gethostbyaddr: determine who sent the message
			 */
			hostp = gethostbyaddr((const char *) &clientaddr.sin_addr.s_addr,
					sizeof(clientaddr.sin_addr.s_addr), AF_INET);
			if (hostp == NULL)
				error("ERROR on gethostbyaddr");
			hostaddrp = inet_ntoa(clientaddr.sin_addr);
			if (hostaddrp == NULL)
				error("ERROR on inet_ntoa\n");
			printf("server established connection with %s (%s)\n",
					hostp->h_name, hostaddrp);

			while (1) { // test, should be while connected, but eventually in a thread
				/*
				 * read: read input string from the client
				 */
				bzero(buf, BUFSIZE);
				n = read(childfd, buf, BUFSIZE);
				if (n < 0)
					error("ERROR reading from socket");

				if (n == 0) {
					printf("Client Disconnected? \n");
					break;
				}

				// ok, we should have the data in buf,
				// let's decode and process it
				uint8_t channel = buf[0];
				uint8_t command = buf[1];
				uint16_t size = (buf[2] << 8) + buf[3]; // endiannes independant parsing
				if (command == 0) {
					int nr_of_channels = 4; // TODO get this number from device info
					uint8_t dev_unit =  (channel / nr_of_channels);
					uint8_t dev_channel =  (channel % nr_of_channels);

					rgb_t* data = (rgb_t*) (buf + 4);
					send_rgb(data, size, 0, dev_channel, dev_unit, grb); // TODO make permutation configurable
				}


				// Not standarised extention
				if (command == 1) {
					int nr_of_channels = 4; // TODO get this number from device info
					uint8_t dev_unit =  (channel / nr_of_channels);
					uint8_t dev_channel =  (channel % nr_of_channels);

					rgbw_t* data = (rgbw_t*) (buf + 4);
					send_rgbw( data, size, 0, dev_channel, dev_unit);
				}


			}

			//close(childfd);
		}

	}
//

	gettimeofday(&t2, NULL);

	elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;      // sec to ms
	elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;   // us to ms
	printf("%f ms  / %f fps\n", elapsedTime, (1000 / elapsedTime));

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

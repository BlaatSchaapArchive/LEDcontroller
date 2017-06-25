/*
 * ITPHController.cpp
 *
 *  Created on: 26 mei 2017
 *      Author: André van Schoubroeck
 */

#include "ITPHController.h"
#include <string>
#include <iostream>

#include <string.h>
#include <unistd.h>

#include "protocol.h"

using namespace std;

void ITPHController::obtainDeviceInfo() {
	std::lock_guard<std::mutex> guard(UsbMutex);
	uint8_t data_buffer[64];
	data_buffer[0] = CMD_DEVINFO;
	data_buffer[1] = 0x00;
	int transferred;
	int retval = libusb_bulk_transfer(handle, 0x01, data_buffer, 1,
			&transferred, 1000);
	if (retval)
		printf("libusb_bulk_transfer failed: %s: %s\n", libusb_error_name(retval),
				libusb_strerror((libusb_error) retval));
	retval = libusb_bulk_transfer(handle, 0x81, data_buffer, 64, &transferred,
			1000);
	if (retval)
		printf("libusb_bulk_transfer failed: %s: %s\n", libusb_error_name(retval),
				libusb_strerror((libusb_error) retval));

	if (data_buffer[0] != CMD_DEVINFO) {
		// Huh? What happened?
	}
	uint8_t offset = 2;
	uint8_t total_size = data_buffer[1];
	if (total_size > sizeof(data_buffer))
		total_size = sizeof(data_buffer);

	if (total_size) {
		while (data_buffer[offset]) {
			uint8_t chunk_size = data_buffer[offset];
			uint8_t chunk_type = data_buffer[offset + 1];
			switch (chunk_type) {
			case DEVINFO_MCU: {
				devinfo_t* devinfo = (devinfo_t*) (data_buffer + offset);
				printf("MCU\n");
				printf("\tARCHITECTURE %02x \n", devinfo->architecture);
				printf("\tCHIP VENDOR %02x \n", devinfo->vendor);
				printf("\tCHIP TYPE %08x \n", devinfo->device);

				break;
			}
			case DEVINFO_LED: {
				led_dev_t* led_dev = (led_dev_t*) (data_buffer + offset);
				printf("LEDS\n");
				if (LED_C0_MASK & led_dev->implementation)
					printf("\tCLOCKCLESS\n");
				if (LED_C1_MASK & led_dev->implementation)
					printf("\tCLOCKED\n");

				printf("\tCHANNELS %02u \n", led_dev->channels);
				printf("\tCOUNT %02u \n", led_dev->count);
				break;
			}
			case DEVINFO_BUF: {
				buffer_t* buffer = (buffer_t*) (data_buffer + offset);
				printf("BUFFER\n");
				printf("\tSIZE %02u \n", buffer->buffer_size);
				break;
			}
			default:
				printf("Unknown info chunk? (%02x)\n", chunk_type);
				break;
			}
			offset += chunk_size;
			if (offset > total_size)
				break;
		}
	}

}

ITPHController::ITPHController(libusb_device *dev) {
	// TODO Auto-generated constructor stub
	this->dev = dev;
	int retval = libusb_open(dev, &handle);

	struct libusb_device_descriptor desc;
	uint8_t serial[256];
	retval = libusb_get_device_descriptor(dev, &desc);

	retval = libusb_get_string_descriptor_ascii(handle, desc.iSerialNumber,
			serial, sizeof(serial));
	printf("New device added: Serial %s\n", serial);
	this->serial = (char*) serial;

	/* Check whether a kernel driver is attached to interface #0. If so, we'll
	 * need to detach it.
	 */
	if (libusb_kernel_driver_active(handle, 0)) {
		retval = libusb_detach_kernel_driver(handle, 0);
		if (retval == 0) {
			kernelDriverDetached = true;
		} else {
			fprintf(stderr, "Error detaching kernel driver.\n");
			return;
		}
	}

	/* Claim interface #0. */
	retval = libusb_claim_interface(handle, 0);
	if (retval != 0) {
		fprintf(stderr, "Error claiming interface.\n");
		return;
	}

	obtainDeviceInfo();

}

ITPHController::~ITPHController() {
	printf("Deleting ITPHController instance: %s\n", serial.c_str());

	/* Release interface #0. */
	int retval = libusb_release_interface(handle, 0);
	if (0 != retval) {
		fprintf(stderr, "Error releasing interface.\n");
	}

	/* If we detached a kernel driver from interface #0 earlier, we'll now
	 * need to attach it again.  */
	if (kernelDriverDetached) {
		libusb_attach_kernel_driver(handle, 0);
	}

	if (handle)
		libusb_close(handle);
}

void ITPHController::libusb_read_thread_code(ITPHController *dm) {
	while (dm->libusb_read_thread_running) {

	}
}

bool ITPHController::isSupportedDevice(libusb_device *dev) {
	struct libusb_device_descriptor desc;
	libusb_device_handle *handle = NULL;
	uint8_t string_descriptor[256];
	int retval = libusb_open(dev, &handle);
	if (retval) {
		printf("Unable to open device\n");
		return false;
	}

	retval = libusb_get_device_descriptor(dev, &desc);
	if (retval < 0) {
		//fprintf(stderr, "failed to get device descriptor");
		printf("failed to get device descriptor: %s: %s\n",
				libusb_error_name(retval),
				libusb_strerror((libusb_error) retval));
		libusb_close(handle);
		return false;
	}

	retval = libusb_get_string_descriptor_ascii(handle, desc.iManufacturer,
			string_descriptor, sizeof(string_descriptor));

	if (retval < 0) {

		//fprintf(stderr, "failed to get string descriptor");
		printf("failed to get string (iManufacturer) descriptor: %s: %s\n",
				libusb_error_name(retval),
				libusb_strerror((libusb_error) retval));
		libusb_close(handle);
		return false;
	}

	printf("Got %s\n", string_descriptor);

	if (strcmp("The IT Philosoher (https://www.philosopher.it)",
			(char*) string_descriptor)) {
		printf("No match!\n");
		libusb_close(handle);
		return false;
	}

	retval = libusb_get_string_descriptor_ascii(handle, desc.iProduct,
			string_descriptor, sizeof(string_descriptor));

	if (retval < 0) {
		//fprintf(stderr, "failed to get string descriptor");
		printf("failed to get string (iProduct) descriptor: %s: %s\n",
				libusb_error_name(retval),
				libusb_strerror((libusb_error) retval));
		libusb_close(handle);
		return false;
	}

	printf("Got %s\n", string_descriptor);

	char cmpName[256];
	for (int i = 0; i < 0xff; i++) {
		sprintf(cmpName, "LED Controller (variant %02x)", i);
		if (!strcmp(cmpName, (char*) string_descriptor)) {
			printf("Match!\n");
			libusb_close(handle);
			return true;
		}
	}
	printf("No match!\n");
	libusb_close(handle);
	return false;
}

bool ITPHController::isBusy() {

	uint8_t data_buffer[64];
	data_buffer[0] = CMD_BUFFER_STATE;

	int transferred;
	int retval = libusb_bulk_transfer(handle, 0x01, data_buffer, 1,
			&transferred, 1000);
	if (retval)
		printf("libusb_bulk_transfer failed: %s: %s\n", libusb_error_name(retval),
				libusb_strerror((libusb_error) retval));
	retval = libusb_bulk_transfer(handle, 0x81, data_buffer, 64, &transferred,
			1000);
	if (retval)
		printf("libusb_bulk_transfer failed: %s: %s\n", libusb_error_name(retval),
				libusb_strerror((libusb_error) retval));
	return data_buffer[1];


}

void ITPHController::setLeds(void* data, size_t size, int offset) {
	std::lock_guard<std::mutex> guard(UsbMutex);
	uint8_t send_buffer[64];
	int retval = 0;
	int transferred = 0;
	int target_pos = offset;
	send_buffer[0] = CMD_FILL_BUFFER_SC;

	uint16_t *target_offset = (uint16_t*) (send_buffer + 1);
	uint8_t *target_amount = (uint8_t*) (send_buffer + 3);
	*target_offset = target_pos;
	*target_amount = 60;

	int source_offset = 0;
	while (size > 60) {
		memcpy(send_buffer + 5, ((char*)(data))+source_offset, 60 );
		retval = libusb_bulk_transfer(handle, 0x01, send_buffer, 64,
				&transferred, 1000);

		if (retval)
			printf("libusb_bulk_transfer failed: %s: %s\n", libusb_error_name(retval),
					libusb_strerror((libusb_error) retval));

		source_offset += 60;
		size -= 60;
		*target_offset += 60;
	}
	if (size) {
		*target_amount = size;
		memcpy(send_buffer + 4, ((char*)(data))+source_offset, size );
		retval = libusb_bulk_transfer(handle, 0x01, send_buffer, 64,
				&transferred, 1000);

		if (retval)
			printf("libusb_bulk_transfer failed: %s: %s\n", libusb_error_name(retval),
					libusb_strerror((libusb_error) retval));

	}

}

void ITPHController::applyBuffer(int channel, int unit, size_t size) {
	std::lock_guard<std::mutex> guard(UsbMutex);
	uint8_t send_buffer[64];
	int retval;
	int transferred;
	send_buffer[0] = CMD_APPLY_BUF_SC; // APPLY BUFFER
	uint16_t* taget_offset = (uint16_t*) ((send_buffer + 1));
	uint16_t* target_amount = (uint16_t*) ((send_buffer + 3));
	uint8_t* target_first_channel = send_buffer + 5;
	uint8_t* target_nr_channels = send_buffer + 6;
	uint8_t* target_pwm_unit = send_buffer + 7;
	*taget_offset = (3072 * channel);
	*target_amount = size * sizeof(rgbw_t);
	*target_first_channel = channel;
	*target_nr_channels = 1;
	*target_pwm_unit = unit;
	retval = libusb_bulk_transfer(handle, 0x01, send_buffer, 9, &transferred,
			1000);
}

void ITPHController::setLeds(rgbw_t* rgbw_data, size_t size, int offset,
		int channel, int unit) {

	setLeds((void*) rgbw_data, size * sizeof(rgbw_t), offset * sizeof(rgbw_t) + (3072 * channel) );

	applyBuffer(channel, unit, size);
}

/*
void ITPHController::setLeds(rgbw_t* rgbw_data, size_t size, int offset,
		int channel, int unit) {
	//std::thread::id this_id = std::this_thread::get_id();
	//std::cout << "thread " << this_id << " meets guard...\n";
	std::lock_guard<std::mutex> guard(UsbMutex);
	//std::cout << "thread " << this_id << " past guard...\n";

	uint8_t send_buffer[64];

	while (isBusy()) usleep(5);

	send_buffer[0] = CMD_FILL_BUFFER4; // FILL BUFFER, COMPRESSED DATA GRBW
	send_buffer[1] = channel;  // TARGET CHANNEL 0
	send_buffer[2] = offset;  // offset in leds
	send_buffer[3] = 15; // 15 LEDS per packet ( 60 / 4 )
	int retval = 0;
	int leds_remaining = size;
	int transferred;
	while (leds_remaining > 15) {
		memcpy(send_buffer + 4, rgbw_data + send_buffer[2] - offset, 60);
		leds_remaining -= 15;
		retval = libusb_bulk_transfer(handle, 0x01, send_buffer, 64,
				&transferred, 1000);

		if (retval)
			printf("libusb_bulk_transfer failed: %s: %s\n", libusb_error_name(retval),
					libusb_strerror((libusb_error) retval));

		send_buffer[2] += 15;
	}
	// Transmit remaining leds
	send_buffer[3] = leds_remaining;
	memcpy(send_buffer + 4, rgbw_data + send_buffer[2] - offset,
			leds_remaining * sizeof(rgbw_t));
	retval = libusb_bulk_transfer(handle, 0x01, send_buffer, 64, &transferred,
			1000);

	if (retval)
		printf("libusb_bulk_transfer failed: %s: %s\n", libusb_error_name(retval),
				libusb_strerror((libusb_error) retval));

	// TODO: buffered mode, separate apply

	send_buffer[0] = 0x11; // APPLY BUFFER
	send_buffer[1] = 0x00; // BUFFER 0
	send_buffer[2] = unit; // GENERATOR UNIT
	retval = libusb_bulk_transfer(handle, 0x01, send_buffer, 3, &transferred,
			1000);

	//std::cout << "thread " << this_id << " finished...\n";
}
*/
void ITPHController::setLeds(drgb_t* drgb_data, size_t size, int offset,
		int channel, int unit) {
	std::lock_guard<std::mutex> guard(UsbMutex);
	uint8_t send_buffer[64];

	while (isBusy()) usleep(5);
	// TODO: Implement me
}

/*
void ITPHController::setLeds(rgb_t* rgb_data, size_t size, int offset,
		int channel, int unit) {
	//std::thread::id this_id = std::this_thread::get_id();
	//std::cout << "thread " << this_id << " meets guard...\n";
	std::lock_guard<std::mutex> guard(UsbMutex);
	//std::cout << "thread " << this_id << " past guard...\n";

	uint8_t send_buffer[64];

	while (isBusy()) usleep(5);

	send_buffer[0] = CMD_FILL_BUFFER3; // FILL BUFFER, COMPRESSED DATA GRB
	send_buffer[1] = channel;  // TARGET CHANNEL 0
	send_buffer[2] = offset;  // offset in leds
	send_buffer[3] = 20; // 20 LEDS per packet ( 60 / 3 )
	int retval = 0;
	int leds_remaining = size;
	int transferred;
	while (leds_remaining > 20) {
		memcpy(send_buffer + 4, rgb_data + send_buffer[2] - offset, 60);
		leds_remaining -= 20;
		retval = libusb_bulk_transfer(handle, 0x01, send_buffer, 64,
				&transferred, 1000);

		if (retval)
			printf("libusb_bulk_transfer failed: %s: %s\n", libusb_error_name(retval),
					libusb_strerror((libusb_error) retval));

		send_buffer[2] += 20;
	}
	// Transmit remaining leds
	send_buffer[3] = leds_remaining;
	memcpy(send_buffer + 4, rgb_data + send_buffer[2] - offset,
			leds_remaining * sizeof(rgb_t));
	retval = libusb_bulk_transfer(handle, 0x01, send_buffer, 64, &transferred,
			1000);


	if (retval)
		printf("libusb_bulk_transfer failed: %s: %s\n", libusb_error_name(retval),
				libusb_strerror((libusb_error) retval));
	// TODO: buffered mode, separate apply

	send_buffer[0] = 0x11; // APPLY BUFFER
	send_buffer[1] = 0x00; // BUFFER 0
	send_buffer[2] = unit; // GENERATOR UNIT
	retval = libusb_bulk_transfer(handle, 0x01, send_buffer, 3, &transferred,
			1000);

	//std::cout << "thread " << this_id << " finished...\n";
}
*/

void ITPHController::setLeds(rgb_t* rgb_data, size_t size, int offset,
		int channel, int unit) {

	setLeds((void*) rgb_data, size * sizeof(rgb_t), offset * sizeof(rgb_t) + (3072 * channel) );

	applyBuffer(channel, unit, size);
}

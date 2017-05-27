/*
 * ITPHController.cpp
 *
 *  Created on: 26 mei 2017
 *      Author: André van Schoubroeck
 */

#include "ITPHController.h"
#include <string>
#include <string.h>
#include <unistd.h>
using namespace std;

ITPHController::ITPHController(libusb_device *dev) {
	// TODO Auto-generated constructor stub
	this->dev = dev;
	int retval = libusb_open(dev, &handle);

	struct libusb_device_descriptor desc;
	uint8_t serial[256];
	retval = libusb_get_device_descriptor(dev, &desc);

	retval = libusb_get_string_descriptor_ascii(handle, desc.iSerialNumber, serial,
			sizeof(serial));
	printf("New device added: Serial %s\n", serial);
	this->serial = (char*)serial;
}

ITPHController::~ITPHController() {
	// TODO Auto-generated destructor stub
	printf("Deleting ITPHController instance: %s\n", serial.c_str());

	if (handle) libusb_close(handle);
}





bool ITPHController::isSupportedDevice(libusb_device *dev){
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
			fprintf(stderr, "failed to get device descriptor");
			libusb_close(handle);
			return false;
		}

		retval = libusb_get_string_descriptor_ascii(handle, desc.iManufacturer,
				string_descriptor, sizeof(string_descriptor));
		if (LIBUSB_ERROR_BUSY == retval) {
			printf("Busy, sleeping\n");
			usleep(1000);
			printf("Trying again\n");
			retval = libusb_get_string_descriptor_ascii(handle, desc.iManufacturer,
							string_descriptor, sizeof(string_descriptor));
		}


		if (retval < 0) {
			fprintf(stderr, "failed to get string descriptor");
			libusb_close(handle);
			return false;
		}

		printf("Got %s\n", string_descriptor);

		if (strcmp("The IT Philosoher (https://www.philosopher.it)", string_descriptor)) {
			libusb_close(handle);
			return false;
		}

		retval = libusb_get_string_descriptor_ascii(handle, desc.iProduct, string_descriptor,
				sizeof(string_descriptor));

		if (retval < 0) {
					fprintf(stderr, "failed to get string descriptor");
					libusb_close(handle);
					return false;
				}

		printf("Got %s\n", string_descriptor);

		uint8_t cmpName[256];
		for (int i = 0; i < 0xff; i++) {
			sprintf(cmpName, "LED Controller (variant %02x)", i);
			if (!strcmp(cmpName, string_descriptor)) {
				libusb_close(handle);
				return true;
			}
		}

		libusb_close(handle);
		return false;
}

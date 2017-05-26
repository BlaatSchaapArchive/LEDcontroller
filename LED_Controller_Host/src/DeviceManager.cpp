/*
 * DeviceManager.cpp
 *
 *  Created on: 26 mei 2017
 *      Author: andre
 */

#include "DeviceManager.h"
#include "ITPHController.h"

#include <unistd.h>

DeviceManager::DeviceManager() {
	int res = libusb_init(&ctx);
	if (res != 0) {
		fprintf(stderr, "Error initialising libusb.\n");
		return;
	}
	res = libusb_hotplug_register_callback(ctx, LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED |
			LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT, LIBUSB_HOTPLUG_ENUMERATE, LIBUSB_HOTPLUG_MATCH_ANY, LIBUSB_HOTPLUG_MATCH_ANY,
			LIBUSB_HOTPLUG_MATCH_ANY, DeviceManager::libusb_hotplug_callback, this,
			&handle);

}

DeviceManager::~DeviceManager() {

	libusb_exit(ctx);
}


void DeviceManager::poll() {
	libusb_handle_events_completed(ctx, nullptr);
}
void DeviceManager::addController(LedController *controller) {
	mapStringLedController[controller->getSerial()] = controller;
	mapDeviceLedController[controller->getDevice()] = controller;
}


void DeviceManager::eraseController(LedController *controller) {
	mapStringLedController.erase(controller->getSerial());
	mapDeviceLedController.erase(controller->getDevice());
}
int DeviceManager::libusb_hotplug_callback(struct libusb_context *ctx, struct libusb_device *dev,
		libusb_hotplug_event event, void *user_data) {
	DeviceManager* dm =(DeviceManager*)(user_data);

	usleep(100000); //?? try a sleep? or is something else wrong?

	if (LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED == event) {
		if ( ITPHController::isSupportedDevice(dev) ) {
			ITPHController *controller = new ITPHController(dev);
			dm->addController(controller);
		}
	} else if (LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT == event) {
		LedController * lc = dm->mapDeviceLedController[dev];
		if (lc) {
			dm->eraseController(lc);
			delete lc;
		}
	} else {
		printf("Unhandled event %d\n", event);
	}
	//count++;
	return 0;
}

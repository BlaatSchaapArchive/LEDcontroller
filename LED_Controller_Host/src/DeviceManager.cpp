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
	//int res = libusb_init(&ctx);
	int res = libusb_init(nullptr);
	if (res != 0) {
		fprintf(stderr, "Error initialising libusb.\n");
		return;
	}
	libusb_hotplug_callback_thread = thread(libusb_hotplug_callback_thread_code, this);
	libusb_handle_events_thread = thread(libusb_handle_events_thread_code, this);
	res = libusb_hotplug_register_callback(ctx, LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED |
			LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT, LIBUSB_HOTPLUG_ENUMERATE, LIBUSB_HOTPLUG_MATCH_ANY, LIBUSB_HOTPLUG_MATCH_ANY,
			LIBUSB_HOTPLUG_MATCH_ANY, DeviceManager::libusb_hotplug_callback, this,
			&handle);

}

DeviceManager::~DeviceManager() {

	libusb_exit(ctx);
}


void DeviceManager::libusb_handle_events_thread_code(DeviceManager *dm) {
	while (dm->libusb_handle_events_thread_running)
		libusb_handle_events_completed(dm->ctx, nullptr);
}
void DeviceManager::addController(LedController *controller) {
	mapStringLedController[controller->getSerial()] = controller;
	mapDeviceLedController[controller->getDevice()] = controller;
}


void DeviceManager::eraseController(LedController *controller) {
	mapStringLedController.erase(controller->getSerial());
	mapDeviceLedController.erase(controller->getDevice());
}

void DeviceManager::libusb_hotplug_callback_thread_code(DeviceManager *dm) {
	while (dm->libusb_hotplug_callback_thread_running) {
		// todo: add some signalling to prevent busy waiting useless cpu consumption

		if (!dm->libusb_hotplug_event_queue.empty()) {
		auto libusb_hotplug_callback_event = dm->libusb_hotplug_event_queue.front();
		dm->libusb_hotplug_event_queue.pop();

		if (LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED == libusb_hotplug_callback_event.event) {

			if ( ITPHController::isSupportedDevice(libusb_hotplug_callback_event.dev) ) {
				ITPHController *controller = new ITPHController(libusb_hotplug_callback_event.dev);
				dm->addController(controller);
			}

		} else if (LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT == libusb_hotplug_callback_event.event) {
			LedController * lc = dm->mapDeviceLedController[libusb_hotplug_callback_event.dev];
			if (lc) {
				dm->eraseController(lc);
				delete lc;
			}
		} else {
			printf("Unhandled event %d\n", libusb_hotplug_callback_event.event);
		}
		}
	}
}

int DeviceManager::libusb_hotplug_callback(struct libusb_context *ctx, struct libusb_device *dev,
		libusb_hotplug_event event, void *user_data) {
	DeviceManager* dm =(DeviceManager*)(user_data);
	dm->libusb_hotplug_event_queue.push({ctx,dev,event});
	return 0;

/*
	if (LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED == event) {

		// this must be run elsewhere... descriptor strings cannot be obtained from hotplug callback
		// we will get a busy reponse if we try
		// https://github.com/libusb/libusb/issues/299
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
	*/
}

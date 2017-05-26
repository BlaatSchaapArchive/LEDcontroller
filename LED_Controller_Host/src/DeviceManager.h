/*
 * DeviceManager.h
 *
 *  Created on: 26 mei 2017
 *      Author: andre
 */

#ifndef SRC_DEVICEMANAGER_H_
#define SRC_DEVICEMANAGER_H_

#include <vector>
#include <map>
#include <string>
#include "LedController.h"

using namespace std;


class DeviceManager {
public:
	DeviceManager();
	virtual ~DeviceManager();
	void addController(LedController *controller);
	void eraseController(LedController *controller);

	void poll() ;
private:
	//vector<LedController> m_LedControllers;
	map<string,LedController*> mapStringLedController;
	map<libusb_device*,LedController*> mapDeviceLedController;

	libusb_context *ctx;
	libusb_hotplug_callback_handle handle;
	static int libusb_hotplug_callback(struct libusb_context *ctx, struct libusb_device *dev,
	                     libusb_hotplug_event event, void *user_data);
};

#endif /* SRC_DEVICEMANAGER_H_ */

/*
 * ITPHController.h
 *
 *  Created on: 26 mei 2017
 *      Author: André van Schoubroeck
 */

#ifndef SRC_ITPHCONTROLLER_H_
#define SRC_ITPHCONTROLLER_H_

#include "LedController.h"

#include "libusb.h"

class ITPHController: public LedController {
public:
	ITPHController(libusb_device *dev);
	virtual ~ITPHController();
	static bool isSupportedDevice(libusb_device *dev);

	virtual libusb_device* getDevice() { return dev; } ;
	virtual string getSerial() { return serial; }
private:
	libusb_device *dev;
	libusb_device_handle* handle;
	string serial;

};

#endif /* SRC_ITPHCONTROLLER_H_ */

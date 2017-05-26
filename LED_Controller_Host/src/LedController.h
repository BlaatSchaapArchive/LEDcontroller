/*
 * LedController.h
 *
 *  Created on: 26 mei 2017
 *      Author: André van Schoubroeck
 */

#ifndef SRC_LEDCONTROLLER_H_
#define SRC_LEDCONTROLLER_H_

#include <string>
#include "libusb.h"

using namespace std;

class LedController {
public:
	LedController();
	virtual ~LedController();
	virtual string getSerial() = 0;
	static bool isSupportedDevice(libusb_device *dev) { return false; };
	virtual libusb_device* getDevice() = 0;
};

#endif /* SRC_LEDCONTROLLER_H_ */

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
#include "colour.h"

using namespace std;

class LedController {
public:
	LedController();
	virtual ~LedController();
	virtual string getSerial() = 0;
	static bool isSupportedDevice(libusb_device *dev) { return false; };
	virtual libusb_device* getDevice() = 0;


	virtual void setLeds(rgb_t* data, size_t size, int offset, int channel, int unit) = 0;
	virtual void setLeds(rgbw_t* data, size_t size, int offset, int channel, int unit) = 0;
	virtual void setLeds(drgb_t* data, size_t size, int offset, int channel, int unit) = 0;

};

#endif /* SRC_LEDCONTROLLER_H_ */

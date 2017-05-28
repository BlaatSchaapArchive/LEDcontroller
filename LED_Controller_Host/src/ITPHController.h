/*
 * ITPHController.h
 *
 *  Created on: 26 mei 2017
 *      Author: André van Schoubroeck
 */

#ifndef SRC_ITPHCONTROLLER_H_
#define SRC_ITPHCONTROLLER_H_

#include "LedController.h"

#include <thread>         // std::thread

#include "libusb.h"

class ITPHController: public LedController {
public:
	ITPHController(libusb_device *dev);
	virtual ~ITPHController();
	static bool isSupportedDevice(libusb_device *dev);

	virtual libusb_device* getDevice() { return dev; } ;
	virtual string getSerial() { return serial; }


	virtual void setLeds(rgb_t* data, size_t size, int offset, int channel, int unit);
	virtual void setLeds(rgbw_t* data, size_t size, int offset, int channel, int unit);
	virtual void setLeds(drgb_t* data, size_t size, int offset, int channel, int unit);


private:
	libusb_device *dev;
	libusb_device_handle* handle;
	string serial;
	bool kernelDriverDetached;

	bool libusb_read_thread_running = false;
	thread libusb_read_thread;
	static void libusb_read_thread_code(ITPHController *dm);

	void obtainDeviceInfo();

	bool isBusy();

};

#endif /* SRC_ITPHCONTROLLER_H_ */

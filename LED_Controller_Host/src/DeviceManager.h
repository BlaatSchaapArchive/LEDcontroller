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
#include <queue>          // std::queue
#include <thread>         // std::thread


#include "LedController.h"

using namespace std;


typedef struct {
	struct libusb_context *ctx;
	struct libusb_device *dev;
	libusb_hotplug_event event;
} libusb_hotplug_event_t;

class DeviceManager {
public:
	DeviceManager();
	virtual ~DeviceManager();
	void addController(LedController *controller);
	void eraseController(LedController *controller);



	virtual void setLeds(rgb_t* data, size_t size,  int channel);
	virtual void setLeds(rgbw_t* data, size_t size, int channel);
	virtual void setLeds(drgb_t* data, size_t size, int channel);

private:
	bool libusb_hotplug_callback_thread_running = true;
	bool libusb_handle_events_thread_running = true;
	thread libusb_hotplug_callback_thread;
	thread libusb_handle_events_thread;
	static void libusb_hotplug_callback_thread_code(DeviceManager *dm);
	static void libusb_handle_events_thread_code(DeviceManager *dm) ;

	//vector<LedController> m_LedControllers;
	map<string,LedController*> mapStringLedController;
	map<libusb_device*,LedController*> mapDeviceLedController;

	libusb_context *ctx;
	libusb_hotplug_callback_handle handle;
	static int libusb_hotplug_callback(struct libusb_context *ctx, struct libusb_device *dev,
	                     libusb_hotplug_event event, void *user_data);
	queue<libusb_hotplug_event_t> libusb_hotplug_event_queue;





};

#endif /* SRC_DEVICEMANAGER_H_ */

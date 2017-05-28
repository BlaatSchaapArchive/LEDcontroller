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
#include "colour.h"

using namespace std;

typedef enum led_type_t : uint8_t {
	generic_clockless_rgb = 0x10,
	generic_clockless_rgbw = 0x20,
	generic_clockless_wwa = 0x30,

	generic_clocked_rgb = 0x80,
	apa102 = 0x81, // High freq PWM, DIM low frew PWM
	ws9822 = 0x82, // Medium freq PWM, DIM current limited
} led_type_t;


typedef enum direction_t : uint8_t {
	forwards,
	backwards,
} direction_t;

typedef struct {
	struct libusb_context *ctx;
	struct libusb_device *dev;
	libusb_hotplug_event event;
} libusb_hotplug_event_t;


typedef struct {
	uint8_t opc_channel;
	uint8_t opc_offset;
	uint8_t opc_size;

	string  device_serial;
	uint8_t device_channel;
	uint8_t device_offset;

	direction_t 		led_direction;
	led_type_t 			led_type;
	rgb_permurations_t 	led_permutation;

	uint32_t 			option_flags;
} opc_device_map_t;

class DeviceManager {
public:
	DeviceManager();
	virtual ~DeviceManager();
	void addController(LedController *controller);
	void eraseController(LedController *controller);



	 void setLeds(rgb_t* data, size_t size,  int channel);
	 void setLeds(rgbw_t* data, size_t size, int channel);
	 void setLeds(drgb_t* data, size_t size, int channel);

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

	vector<opc_device_map_t> opc_device_map;

	void permute_rgb_data(rgb_t* data, size_t size, rgb_permurations_t permutation);



};

#endif /* SRC_DEVICEMANAGER_H_ */

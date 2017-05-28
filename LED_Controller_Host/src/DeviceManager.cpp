/*
 * DeviceManager.cpp
 *
 *  Created on: 26 mei 2017
 *      Author: andre
 */

#include "DeviceManager.h"
#include "ITPHController.h"

#include <unistd.h>
#include <string.h>

DeviceManager::DeviceManager() {
	//int res = libusb_init(&ctx);
	int res = libusb_init(nullptr);
	if (res != 0) {
		fprintf(stderr, "Error initialising libusb.\n");
		return;
	}
	libusb_hotplug_callback_thread = thread(libusb_hotplug_callback_thread_code, this);
	libusb_handle_events_thread = thread(libusb_handle_events_thread_code, this);
	res = libusb_hotplug_register_callback(ctx, libusb_hotplug_event (LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED |
			LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT), LIBUSB_HOTPLUG_ENUMERATE, LIBUSB_HOTPLUG_MATCH_ANY, LIBUSB_HOTPLUG_MATCH_ANY,
			LIBUSB_HOTPLUG_MATCH_ANY, DeviceManager::libusb_hotplug_callback, this,
			&handle);



	// For testing purposes, we hard code some mapping now.

	opc_device_map_t map;
	map.device_channel = 2;
	map.device_offset = 0;
	map.device_serial = "I1616UBD";
	map.led_direction = forwards;
	map.led_type = generic_clockless_rgb;
	map.led_rgb_permutation = grb;
	map.opc_channel = 1;
	map.opc_offset = 0;
	map.opc_size = 30;
	opc_device_map.push_back(map);

	map.device_channel = 4;
	map.led_rgb_permutation = rgb;
	map.opc_channel = 2;
	map.opc_size = 2;
	opc_device_map.push_back(map);

	map.led_type = generic_clockless_rgbw;
	map.led_rgbw_permutation = grbw;
	map.device_channel = 1;
	map.opc_channel = 3;
	map.opc_size = 3;
	opc_device_map.push_back(map);


}

DeviceManager::~DeviceManager() {

	libusb_exit(ctx);
}

void DeviceManager::permute_rgbw_data(rgbw_t* data, size_t size, rgbw_permurations_t permutation) {
	if (permutation == rgbw)
		return;

	rgbw_t data_orig[size];
	memcpy(data_orig, data, sizeof(data_orig));
	for (int i = 0; i < size; i++) {
		switch (permutation) {
	  case bgrw:
			data[i].r = data_orig[i].b;
			data[i].g = data_orig[i].g;
			data[i].b = data_orig[i].r;
			data[i].w = data_orig[i].w;
      break;
	  case bgwr:
			data[i].r = data_orig[i].b;
			data[i].g = data_orig[i].g;
			data[i].b = data_orig[i].w;
			data[i].w = data_orig[i].r;
      break;
	  case brgw:
			data[i].r = data_orig[i].b;
			data[i].g = data_orig[i].r;
			data[i].b = data_orig[i].g;
			data[i].w = data_orig[i].w;
      break;
	  case brwg:
			data[i].r = data_orig[i].b;
			data[i].g = data_orig[i].r;
			data[i].b = data_orig[i].w;
			data[i].w = data_orig[i].g;
      break;
	  case bwgr:
			data[i].r = data_orig[i].b;
			data[i].g = data_orig[i].w;
			data[i].b = data_orig[i].g;
			data[i].w = data_orig[i].r;
      break;
	  case bwrg:
			data[i].r = data_orig[i].b;
			data[i].g = data_orig[i].w;
			data[i].b = data_orig[i].r;
			data[i].w = data_orig[i].g;
      break;
	  case gbrw:
			data[i].r = data_orig[i].g;
			data[i].g = data_orig[i].b;
			data[i].b = data_orig[i].r;
			data[i].w = data_orig[i].w;
      break;
	  case gbwr:
			data[i].r = data_orig[i].g;
			data[i].g = data_orig[i].b;
			data[i].b = data_orig[i].w;
			data[i].w = data_orig[i].r;
      break;
	  case grbw:
			data[i].r = data_orig[i].g;
			data[i].g = data_orig[i].r;
			data[i].b = data_orig[i].b;
			data[i].w = data_orig[i].w;
      break;
	  case grwb:
			data[i].r = data_orig[i].g;
			data[i].g = data_orig[i].r;
			data[i].b = data_orig[i].w;
			data[i].w = data_orig[i].b;
      break;
	  case gwbr:
			data[i].r = data_orig[i].g;
			data[i].g = data_orig[i].w;
			data[i].b = data_orig[i].b;
			data[i].w = data_orig[i].r;
      break;
	  case gwrb:
			data[i].r = data_orig[i].g;
			data[i].g = data_orig[i].w;
			data[i].b = data_orig[i].r;
			data[i].w = data_orig[i].b;
      break;
	  case rbgw:
			data[i].r = data_orig[i].r;
			data[i].g = data_orig[i].b;
			data[i].b = data_orig[i].g;
			data[i].w = data_orig[i].w;
      break;
	  case rbwg:
	  case rgbw:
	  case rgwb:
	  case rwbg:
	  case rwgb:
	  case wbgr:
	  case wbrg:
	  case wgbr:
	  case wgrb:
	  case wrbg:
	  case wrgb:
      // TODO... but low prority: I don't expect them to appear in the wild.
			break;
		default:
			break;
		}
	}
}

void DeviceManager::permute_rgb_data(rgb_t* data, size_t size, rgb_permurations_t permutation) {
	if (permutation == rgb)
		return;

	rgb_t data_orig[size];
	memcpy(data_orig, data, sizeof(data_orig));
	for (int i = 0; i < size; i++) {
		switch (permutation) {
		case rbg:
			data[i].r = data_orig[i].r;
			data[i].g = data_orig[i].b;
			data[i].b = data_orig[i].g;
			break;
		case grb:
			data[i].r = data_orig[i].g;
			data[i].g = data_orig[i].r;
			data[i].b = data_orig[i].b;
			break;
		case gbr:
			data[i].r = data_orig[i].g;
			data[i].g = data_orig[i].b;
			data[i].b = data_orig[i].r;
			break;
		case brg:
			data[i].r = data_orig[i].b;
			data[i].g = data_orig[i].r;
			data[i].b = data_orig[i].g;
			break;
		case bgr:
			data[i].r = data_orig[i].b;
			data[i].g = data_orig[i].g;
			data[i].b = data_orig[i].r;
			break;
		case rgb:
			// Will never get here, but to keep the compiler happy
		default:
			break;
		}
	}
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



void DeviceManager::setLeds(rgb_t* data, size_t size,  int channel){
	for  (auto &map : opc_device_map) {
		if (map.opc_channel == channel) {
			if (map.opc_offset == 0) {
				if (map.device_offset == 0) {
					if (map.led_type & generic_clockless_rgb) {
						if (map.led_direction == forwards) {
							permute_rgb_data(data,size,map.led_rgb_permutation);
							auto device = mapStringLedController[map.device_serial];
							if (device) {
								// TODO!!!!  Channel/Unit to be handled *in* the device, and the 4 should come from the device.
								// Perhaps some handling of this could be done in Device Manager in order to implement
								// some more advanced features taking advantage of buffering, but while we are implementing
								// basic OPC, no such features are supported.
								device->setLeds(data,size, map.device_offset, map.device_channel%4, map.device_channel/4 );
							} else {
								// TODO IMPLEMENT REVERSING
							}
						}
					} else {
						// ERROR, DEVICE NOT CONNECTED
					}
				} else {
					// TODO IMPLEMENT ME / LED TYPE CONVERSION
				}
			} else {
				// TODO IMPLEMENT ME
			}
		} else {
			// TODO IMPLEMENT ME
		}
	}
}




void DeviceManager::setLeds(rgbw_t* data, size_t size, int channel){
	for  (auto &map : opc_device_map) {
		if (map.opc_channel == channel) {
			if (map.opc_offset == 0) {
				if (map.device_offset == 0) {
					if (map.led_type & generic_clockless_rgbw) {
						if (map.led_direction == forwards) {
							permute_rgbw_data(data,size,map.led_rgbw_permutation);
							auto device = mapStringLedController[map.device_serial];
							if (device) {
								// TODO!!!!  Channel/Unit to be handled *in* the device, and the 4 should come from the device.
								// Perhaps some handling of this could be done in Device Manager in order to implement
								// some more advanced features taking advantage of buffering, but while we are implementing
								// basic OPC, no such features are supported.
								device->setLeds(data,size, map.device_offset, map.device_channel%4, map.device_channel/4 );
							} else {
								// TODO IMPLEMENT REVERSING
							}
						}
					} else {
						// ERROR, DEVICE NOT CONNECTED
					}
				} else {
					// TODO IMPLEMENT ME / LED TYPE CONVERSION
				}
			} else {
				// TODO IMPLEMENT ME
			}
		} else {
			// TODO IMPLEMENT ME
		}
	}
}

void DeviceManager::setLeds(drgb_t* data, size_t size, int channel){
	// Old test implementation for now
	int nr_of_channels = 4; // TODO get this number from device info
	uint8_t dev_unit =  (channel / nr_of_channels);
	uint8_t dev_channel =  (channel % nr_of_channels);

	// Until we have a mapper, we use this to address the first connected controller
	// And map the channels 1 to 1 on it.
	auto controller = mapStringLedController.begin();
	controller->second->setLeds(data,size, 0 , dev_channel, dev_unit);
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

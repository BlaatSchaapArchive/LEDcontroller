/*
 * main.cpp
 *
 *  Created on: 26 mei 2017
 *      Author: André van Schoubroeck
 */



#include "DeviceManager.h"
#include "OpenPixelControl.h"

int main(int argc, char* argv[]) {
	DeviceManager *dm = new DeviceManager();
	OpenPixelControl *opc = new OpenPixelControl(dm);



	while(1) {
		//dm->libusb_handle_events_thread_code();
	}
}

/*
 * OpenPixelControl.h
 *
 *  Created on: 28 mei 2017
 *      Author: andre
 */

#ifndef SRC_OPENPIXELCONTROL_H_
#define SRC_OPENPIXELCONTROL_H_

#include <thread>
#include <vector>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>


#include "DeviceManager.h"
using namespace std;

#define BUFSIZE 1024

class OpenPixelControl {
public:
	OpenPixelControl(DeviceManager *dm);
	virtual ~OpenPixelControl();
private:

	bool opc_listen_thread_running = true;
	thread opc_listen_thread;
	static void opc_listen_thread_code(OpenPixelControl *opc);

	void add_opc_connection(int socket);
	vector<thread*> opc_connections;
	static void opc_connection_thread_code(OpenPixelControl *opc, int socket);

	int portno = 7890; /* port to listen on */

	DeviceManager *dm;


};

#endif /* SRC_OPENPIXELCONTROL_H_ */

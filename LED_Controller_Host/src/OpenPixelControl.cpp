/*
 * OpenPixelControl.cpp
 *
 *  Created on: 28 mei 2017
 *      Author: andre
 */

#include "OpenPixelControl.h"


#include <unistd.h>

// network stuff
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <protocol.h>
#include <colour.h>

static void error(const char* msg) { printf("%s\n",msg);}

OpenPixelControl::OpenPixelControl(DeviceManager *dm) {
	printf("New OpenPixelControl\n");
	this->dm=dm;
	opc_listen_thread = thread(opc_listen_thread_code, this);
}

OpenPixelControl::~OpenPixelControl() {
	// TODO Auto-generated destructor stub
}

void OpenPixelControl::add_opc_connection(int socket){
	thread *opc_connection = new thread(opc_connection_thread_code, this, socket);
	opc_connections.push_back(opc_connection);
}

void OpenPixelControl::opc_listen_thread_code(OpenPixelControl *opc){
	printf("OpenPixelControl Listen Thread\n");
	int parentfd; /* parent socket */
		int childfd; /* child socket */

		socklen_t clientlen; /* byte size of client's address */
		struct sockaddr_in serveraddr; /* server's addr */
		struct sockaddr_in clientaddr; /* client addr */
		/*
		 * socket: create the parent socket
		 */
		parentfd = socket(AF_INET, SOCK_STREAM, 0);
		if (parentfd < 0)
			error("ERROR opening socket");

		bzero((char *) &serveraddr, sizeof(serveraddr));

		/* this is an Internet address */
		serveraddr.sin_family = AF_INET;

		/* let the system figure out our IP address */
		serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);

		/* this is the port we will listen on */
		serveraddr.sin_port = htons(opc->portno);

		if (bind(parentfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr))
				< 0)
			error("ERROR on binding");

		/*
		 * listen: make this socket ready to accept connection requests
		 */
		if (listen(parentfd, 5) < 0) /* allow 5 requests to queue up */
			error("ERROR on listen");

		/*
		 * main loop: wait for a connection request, echo input line,
		 * then close connection.
		 */
		clientlen = sizeof(clientaddr);
		while (opc->opc_listen_thread_running) {

			/*
			 * accept: wait for a connection request
			 */
			childfd = accept(parentfd, (struct sockaddr *) &clientaddr,
					&clientlen);
			if (childfd < 0)
				error("ERROR on accept");
			opc->add_opc_connection(childfd);
		}

}


void OpenPixelControl::opc_connection_thread_code(OpenPixelControl *opc, int childfd){
	struct hostent *hostp; /* client host info */
	char *hostaddrp; /* dotted decimal host addr string */
	struct sockaddr_in clientaddr; /* client addr */
	uint8_t buf[BUFSIZE]; /* message buffer */
	ssize_t n; /* message byte size */


	// Add this again, we has clientaddr from accept, but as we run in a different
	// thread, only getting passed the socket, we have to request it again.
	socklen_t len = sizeof(clientaddr);
	getsockname(childfd, (struct sockaddr *)&clientaddr, &len);


	/*
	 * gethostbyaddr: determine who sent the message
	 */
	hostp = gethostbyaddr((const char *) &clientaddr.sin_addr.s_addr,
			sizeof(clientaddr.sin_addr.s_addr), AF_INET);
	if (hostp == NULL)
		error("ERROR on gethostbyaddr");
	hostaddrp = inet_ntoa(clientaddr.sin_addr);
	if (hostaddrp == NULL)
		error("ERROR on inet_ntoa\n");
	printf("server established connection with %s (%s)\n",
			hostp->h_name, hostaddrp);

	while (1) { // test, should be while connected, but eventually in a thread
		/*
		 * read: read input string from the client
		 */
		bzero(buf, BUFSIZE);
		n = read(childfd, buf, BUFSIZE);
		if (n < 0)
			error("ERROR reading from socket");

		if (n == 0) {
			printf("Client Disconnected \n");
			break;
		}

		// ok, we should have the data in buf,
		// let's decode and process it
		uint8_t channel = buf[0];
		uint8_t command = buf[1];
		uint16_t size = (buf[2] << 8) + buf[3]; // endiannes independant parsing
		if ((size - 2) > sizeof(buf)) continue; // invalid data
		if (command == 0) {
			/*
			int nr_of_channels = 4; // TODO get this number from device info
			uint8_t dev_unit =  (channel / nr_of_channels);
			uint8_t dev_channel =  (channel % nr_of_channels);
			rgb_t* data = (rgb_t*) (buf + 4);
			send_rgb(data, size / sizeof(rgb_t), 0, dev_channel, dev_unit, grb); // TODO make permutation configurable
			*/
			rgb_t* data = (rgb_t*) (buf + 4);
			opc->dm->setLeds(data,size / sizeof(rgb_t),channel);
		}


		// Not standarised extention
		if (command == 1) {
			/*
			int nr_of_channels = 4; // TODO get this number from device info
			uint8_t dev_unit =  (channel / nr_of_channels);
			uint8_t dev_channel =  (channel % nr_of_channels);
			rgbw_t* data = (rgbw_t*) (buf + 4);
			send_rgbw( data, size, 0, dev_channel, dev_unit);
			*/

			rgbw_t* data = (rgbw_t*) (buf + 4);
			opc->dm->setLeds(data,size / sizeof(rgbw_t),channel);
		}
	}

}





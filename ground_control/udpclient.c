/*
 * @author: Brian Bauman
 *
 * UDP client for communication between
 * Raspbian and Ubuntu 15.04
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include "main.h" 
#include <time.h>

void *udp_client(void *arg)
{
	int sock,bytes_recv,sin_size;
	struct sockaddr_in server_addr;
	struct hostent *host;
	char send_data[4],recv_data[1024];

	// get outgoing socket
	host= (struct hostent *) gethostbyname((char *)"192.168.1.2");


	if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
	{
		perror("socket");
		exit(1);
	}

	// setup response socket
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(5000); // port 5000
	server_addr.sin_addr = *((struct in_addr *)host->h_addr);
	memset(&(server_addr.sin_zero),0,8);
	sin_size = sizeof(struct sockaddr);

	while (!stop)
	{
		wait();
		// set data
		send_data[0] = joysticks.roll;
		send_data[1] = joysticks.throttle;
		send_data[2] = joysticks.yaw;
		send_data[3] = joysticks.pitch;

		release();

		printf("Sending: %5d %5d %5d %5d\r", send_data[0],send_data[1],send_data[2],send_data[3]);

		// send packet with control info
		sendto(sock, send_data, strlen(send_data), 0, (struct sockaddr *)&server_addr, sizeof(struct sockaddr));

		// receive response
		//bytes_recv = recvfrom(sock, recv_data, 1024, 0, (struct sockaddr *)&server_addr, &sin_size);
		//recv_data[bytes_recv] = '\0'; // to string

		// next udp packet should be sent after response???
		//printf("Received :%s\n",recv_data);

		fflush(stdout);
		if (nanosleep( (const struct timespec[]) {{0,100000000L}} , NULL) ) printf("nanosleep failed\n");
	}
}

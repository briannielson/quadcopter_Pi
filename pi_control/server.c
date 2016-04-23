/*
 * @author: Brian Bauman
 *
 * UDP server for receiving data from
 * Ubuntu 15.10 to Raspbian Jessie
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include "main.h"

void *udp_server(void *arg)
{
        int sock;
        socklen_t addr_len;
        signed char recv_data[4];
        struct sockaddr_in server_addr , client_addr;


        if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
            perror("Socket");
            exit(1);
        }

        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(5000); // port
        server_addr.sin_addr.s_addr = INADDR_ANY;
        memset(&(server_addr.sin_zero),0,8);


        if (bind(sock,(struct sockaddr *)&server_addr,
            sizeof(struct sockaddr)) == -1)
        {
            perror("Bind");
            exit(1);
        }

        addr_len = sizeof(struct sockaddr);

        printf("UDPServer Waiting for client on port 5000\n");
        fflush(stdout);

        char a[20][10],d[20][10];
        strcpy(a[0],"A");
        strcpy(a[1],"B");
        strcpy(a[2],"C");
        strcpy(a[3],"D");
        strcpy(d[0],"123");
        strcpy(d[1],"124");
        strcpy(d[2],"100");
        strcpy(d[3],"99");

        while (!stop)
        {

        	recvfrom(sock,recv_data,1024,0,
                            (struct sockaddr *)&client_addr, &addr_len);

		wait();
		//printf("Y: %4d, P: %4d, R: %4d, T: %4d\n", recv_data[2], recv_data[3], recv_data[0], recv_data[1]);
		motor.roll = recv_data[0];
		motor.throttle = recv_data[1];
		motor.yaw = recv_data[2];
		motor.pitch = recv_data[3];
		release();


/*        printf("\n(%s , %d) said : ",inet_ntoa(client_addr.sin_addr),
                                       ntohs(client_addr.sin_port));
          printf("Roll: %d, Thrust: %d, Yaw: %d, Pitch: %d\n",
		recv_data[0], recv_data[1], recv_data[2], recv_data[3]);

	  printf("SEND : ");
          // gets(send_data);

             int p = 0;
                 for(i=0;i<4;i++)
                 {
                        if(strcmp(recv_data,d[i]) == 0)
                        {
                             strcpy(send_data,a[i]);p=1;
                        }
                 }
                 if(p == 0)
                 strcpy(send_data,"No one on that role.");

          sendto(sock,send_data,strlen(send_data),0,(struct sockaddr *)&client_addr,sizeof(struct sockaddr));
*/
          fflush(stdout);

        }
        return 0;
}

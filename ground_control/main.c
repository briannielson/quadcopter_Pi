/*
 * @author: Brian Bauman
 *
 * main program to allow writing from
 * controller to the shared struct which
 * stores the axes values and reading
 * from the UDP client to send values
 * to the server.
 */

#include <stdio.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include "controller.h"
#include "udpclient.h"
#include "main.h"
#include <stdatomic.h>

volatile sig_atomic_t stop = 0;
atomic_int lock = 1;

void sigterm_catch(int signum)
{
	stop = 1;
}

int main(void)
{
	joysticks.roll = 0;
	joysticks.throttle = 0;
	joysticks.yaw = 0;
	joysticks.pitch = 0;

	pthread_t pth_ctrl;
	pthread_t pth_udp;

	// worker thread for controller
	pthread_create(&pth_ctrl, NULL, controller_mod, 0);
	sleep(2);
	pthread_create(&pth_udp, NULL, udp_client, 0);

	signal(SIGINT, sigterm_catch);

	while (!stop);

	printf("Terminating threads...\n");

	pthread_join(pth_ctrl, NULL);
	pthread_join(pth_udp, NULL);

	release();

	return 0;
}

void wait(void)
{
	while (lock < 1);
	lock-=1;
}

void release(void)
{
	lock+=1;
}

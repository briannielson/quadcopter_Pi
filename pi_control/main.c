/*
 * @author: Brian Bauman
 *
 * Main program to catch commands from UDP
 * and transate to directives using a Linear
 * Kalman filter. The controls will show a
 * target TNB vector, and the Kalman filter
 * over Accelerometer and Gyroscope readings
 * will show the actual TNB vector.
 */

#include <stdio.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include "main.h"
#include "server.h"
#include "motorout.h"
#include <stdatomic.h>

volatile sig_atomic_t stop = 0;

atomic_int lock = 1;

void sigterm_catch(int signum)
{
	stop = 1;
}

int main (void)
{
	// roll, thrust, yaw, pitch
	motor.roll = 0;
	motor.throttle = 0;
	motor.yaw = 0;
	motor.pitch = 0;

	// yaw, pitch, roll input from IMU
	imu.yaw = 0;
	imu.pitch = 0;
	imu.roll = 0;

	pthread_t pth_motorout;
	pthread_t pth_udpserver;

	// worker threads starting...
	pthread_create(&pth_motorout, NULL, motorout, 0);
	sleep(2);
	pthread_create(&pth_udpserver, NULL, udp_server, 0);

	signal(SIGINT, sigterm_catch);

	while (!stop)
	{
		sleep(5);
		printf("main running...\n");
	}

	printf("\nTerminating threads...\n");

	pthread_join(pth_motorout, NULL);
	pthread_join(pth_udpserver, NULL);

	return 0;
}

void wait(void)
{
	while (lock < 1);
	lock -= 1;
}

void release(void)
{
	lock += 1;
}

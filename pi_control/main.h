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
#ifndef _MAIN_H_
#define _MAIN_H_
#include <stdio.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <stdatomic.h>

volatile sig_atomic_t stop;

atomic_int lock;

struct axes
{
	signed char throttle;
	signed char roll;
	signed char pitch;
	signed char yaw;
};

struct sensor
{
	float yaw;
	float pitch;
	float roll;
};

struct axes motor;

struct sensor imu;

void sigterm_catch(int signum);

int main (void);

void wait(void);

void release(void);

#endif

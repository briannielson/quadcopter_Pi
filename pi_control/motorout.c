/*
 * @author: Brian Bauman
 *
 * Translate roll, pitch, yaw, and throttle (thrust)
 * character values from raw input to output
 * to the different ESCs
 */

#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "pwm.h"
#include "main.h"
#include "motorout.h"

struct motorprop {
	int gpio;
	int channel;
	int power;
};

FILE *fp;
char buffer[64];

// pid values
float pid_Kp = 0.005;
float pid_Ki = 0.0005;
float pid_Kd = 0.001;
float pid_out = 0;

// pid vars
float Ki_sum = 0;
float Kd_oldDiff = 0;

void *motorout (void *arg)
{
	struct motorprop m1 = { 22, 0, 0 };
	struct motorprop m2 = { 18, 0, 0 };
	struct motorprop m3 = { 27, 0, 0 };
	struct motorprop m4 = { 17, 0, 0 };

	setup(PULSE_WIDTH_INCREMENT_GRANULARITY_US_DEFAULT, DELAY_VIA_PWM);

	// initialize channels
	init_channel(m1.channel, SUBCYCLE_TIME_US_DEFAULT);

	// sleep for 5 seconds after init
	sleep(5);

	// then add max value pulse to set max range
	add_channel_pulse(m1.channel, m1.gpio, 0, 200);
        add_channel_pulse(m2.channel, m2.gpio, 0, 200);
        add_channel_pulse(m3.channel, m3.gpio, 0, 200);
        add_channel_pulse(m4.channel, m4.gpio, 0, 200);

	// sleep for 1 second to allow ESC to calibrate
	sleep(1);

	// clear all channels
	clear_channel_gpio(m1.channel, m1.gpio);
        clear_channel_gpio(m2.channel, m2.gpio);
        clear_channel_gpio(m3.channel, m3.gpio);
        clear_channel_gpio(m4.channel, m4.gpio);

	set_loglevel(2);

	// sleep for 5 seconds
	sleep(5);
	printf("Armed.\n");

	// ready for action
	while(!stop) {
		calculate_power(&m1.power, &m2.power, &m3.power, &m4.power);
		add_channel_pulse(m1.channel, m1.gpio, 0, m1.power);
       		add_channel_pulse(m2.channel, m2.gpio, 0, m2.power);
       	 	add_channel_pulse(m3.channel, m3.gpio, 0, m3.power);
	        add_channel_pulse(m4.channel, m4.gpio, 0, m4.power);
		//printf("m1: %3d, m2: %3d, m3: %3d, m4: %3d\r", m1.power, m2.power, m3.power, m4.power);
	}

        clear_channel_gpio(m1.channel, m1.gpio);
        clear_channel_gpio(m2.channel, m2.gpio);
        clear_channel_gpio(m3.channel, m3.gpio);
        clear_channel_gpio(m4.channel, m4.gpio);

	shutdown();
	exit(0);
}

/*    1     2
 *     \   /
 *      |||
 *      |||
 *      |||
 *     /   \
 *    3     4
 */

void calculate_power(int *m1p, int *m2p, int *m3p, int *m4p)
{
	int proll, ppitch, pyaw;
	signed char pitch, roll, yaw, throttle;
	int throttleOut;

	fp = fopen("/home/brian/imu_sensor/imuData", "r");
	if (fp > 0) {
		if (fread(buffer, 20, 1, fp) > 0) {

			float ypr[3] = {0};
			int i = 0;

			//if (n >= 0) printf("%s\n", buffer);

			char *token = strtok(buffer, "/ \r\n");
			//printf("tokens:\n");
			while (token != NULL && i < 3) {
				//printf("%s\n", token);
				ypr[i] = strtof(token, NULL);
				token = strtok(NULL, "/ \r\n");
				i++;
			}

			if (imu.yaw == 0) imu.yaw = ypr[0];
			imu.pitch = ypr[1] + 2.;
			imu.roll  = -1 * ypr[2] + 2.;
		}
	} fclose(fp);
	//printf("%8.3f, %8.3f, %8.3f\n", ypr[0], ypr[1], ypr[2]);

	wait();
	//printf("throttle input: %d\n", motor.throttle);
	throttle = motor.throttle < 0 ? ~motor.throttle : 0;
	roll = ~motor.roll / 2;
	pitch = motor.pitch / 2;
	yaw = ~motor.yaw / 2;
	release();

	// override for stable testing
	roll = 0;
	pitch = 0;
	yaw = 0;

	//printf("throttle: %3d, pitch: %3d, roll: %3d, yaw: %3d\n", throttle, pitch, roll, yaw);
	//printf("complement:    pitch: %3d, roll: %3d, yaw: %3d\n", ~pitch, ~roll, ~yaw);

	// map throttle between 100-200
	throttleOut = ((float) throttle / 128) * 100 + 100;

	//printf("ThrottleOutput: %4d, Yaw: %4.2f, Pitch: %4.2f, Roll: %4.2f\r", throttleOut, imu.yaw, imu.pitch, imu.roll);

	// calculate pid values
	proll = throttleOut > 130 ? calculate_pid(imu.roll) : 0;
	pyaw = throttleOut > 130 ? calculate_pid(imu.yaw) : 0;
	ppitch = throttleOut > 130 ? calculate_pid(imu.pitch) : 0;

	*m1p = throttleOut - ppitch + proll + pyaw;
        *m2p = throttleOut - ppitch - proll - pyaw;
        *m3p = throttleOut + ppitch + proll - pyaw;
        *m4p = throttleOut + ppitch - proll + pyaw;
}

void set_prop_gain (float kp) {
	pid_Kp = kp;
}

void set_int_gain (float kint) {
	pid_Ki = kint;
}

void set_der_gain (float kder) {
	pid_Kd = kder;
}

int calculate_pid (float ori_target) {
	// ori_target : Where I want to be in orientation
	// ori : Where I am currently
	pid_out = 0;

	float error = ori_target;
	float deltaDiff;
	float pidTemp;

	// proportional
	pidTemp = pid_Kp * error;
	if (pidTemp > 4.) pidTemp = 4.;
	else if (pidTemp < -4.) pidTemp = -4.;

	pid_out += pidTemp;

	// integral
	Ki_sum += pid_Ki * error;
	if (Ki_sum > 4.) Ki_sum = 4.;
	else if (Ki_sum < 4.) Ki_sum = -4.;

	pid_out += Ki_sum;

	// derivative
	deltaDiff = pid_Kd * (Kd_oldDiff - error);
	Kd_oldDiff = error;
	if (deltaDiff > 4.) deltaDiff = 4;
	else if (deltaDiff < -4.) deltaDiff = -4;

	pid_out += deltaDiff;

	//printf("PID_P: %6.2f, PID_I: %6.2f, PID_D: %6.2f\n", pidTemp, Ki_sum, deltaDiff);

	return (int) pid_out;
}


/*
 * @author: Brian Bauman
 *
 * Translate roll, pitch, yaw, and throttle (thrust)
 * character values from raw input to output
 * to the different ESCs
 */

#ifndef _MOTOROUT_H_
#define _MOTOROUT_H_

void *motorout (void *arg);

void calculate_power(int *m1p, int *m2p, int *m3p, int *m4p);

void set_prop_gain (float kp);

void set_int_gain (float kint);

void set_der_gain (float kder);

int calculate_pid (float ori_target);

#endif

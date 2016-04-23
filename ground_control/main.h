/*
 * @author: Brian Bauman
 *
 * main program to allow writing from
 * controller to the shared struct which
 * stores the axes values and reading
 * from the UDP client to send values
 * to the server.
 */

#include <signal.h>
#include <stdatomic.h>

volatile sig_atomic_t stop;
atomic_int lock;

void sigterm_catch(int signum);

struct axes
{
	char throttle;
	char roll;
	char pitch;
	char yaw;
};

struct axes joysticks;

int main(void);

void wait(void);

void release(void);

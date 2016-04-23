/*
 * @author: Brian Bauman
 *
 * Testing I/O for 360 controller at /dev/input/event10
 */

#include <stdio.h>
#include <sys/ioctl.h>
#include <linux/input.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "main.h"

// struct to store device info, defined in
// linux/input.h

/*struct input_id {
	__u16 bustype;
	__u16 vendor;
	__u16 product;
	__u16 version;
};*/

float int_to_char;

// map a 4-byte signed integer to a 1-byte signed char
char handle_input(int x)
{
	if (x == 0) return 0;
	if (x&(1<<31)) {
		int_to_char = (float) x / 32768 * 128;
		//printf("%d\n", (char) int_to_char);
		return (char) int_to_char;
	} else {
		int_to_char = (float) x / 32767 * 127;
		//printf("%d\n", (char) int_to_char);
		return (char) int_to_char;
	}
}

void *controller_mod (void *arg)
{
	// Place to store the version of driver
	unsigned int version;

	// name of file
	char * fd_source = "/dev/input/event10";

	/* open() returns a file descriptor. The file descriptor
	 * is an abstract indicator (handle) to access a file or
	 * other I/O (see POSIX API).
	 * open() returns 0 for stdin, 1 for stdout, 2 for stderr
	 * open() returns -1 in C for error (file not found, permissions issue...)
	 */
	int fd = open(fd_source, O_RDONLY);

	// instance of variable to store device info
        struct input_id device_info;

	// name place holder
	char name[256] = "Unknown";

	// path place holder
	char path[256] = "Unknown";

	// unique ID
	char uID[256] = "Unknown";

	// event type bit holder, filled with zeros
	char evtype_b[256];
	memset(evtype_b, 0, sizeof(evtype_b));

	// bytes to be read
	size_t rb; // rb -- read buffer

	// individual events
	struct input_event ev[64];

	// handling if file is not found.
	if (fd < 0) perror("file not found or insufficient permissions. Try running as sudo.");
	else {
		// ioctl -- device version
		if (ioctl(fd, EVIOCGVERSION, &version)) {
			perror("evdev ioctl -- dev-version read error.");
		}

		printf("evdev driver version is %d.%d.%d\n",
			version >> 16, (version >> 8) & 0xff,
			version & 0xff);

		// ioctl -- device info
		if (ioctl(fd, EVIOCGID, &device_info)) {
			perror("evdev ioctl -- dev-info read error.");
		}

		printf("vendor %04hx product %04hx version %04hx",
			device_info.vendor, device_info.product,
			device_info.version);
		switch ( device_info.bustype)
		{
			case BUS_PCI :
				printf(" is on a PCI bus\n");
				break;
			case BUS_USB :
				printf(" is on a Universal Serial Bus\n");
				break;
			default :
				printf(" isn't on USB or PCI, but is connected\n");
		}

		// ioctl -- Device name
		if (ioctl(fd, EVIOCGNAME(sizeof(name)), name) < 0)
			perror("evdev ioctl -- device name is not defined or not found.");

		printf("The device on %s says its name is %s\n",
			fd_source, name);

		// ioctl -- topology (physical address)
		if (ioctl(fd, EVIOCGPHYS(sizeof(path)), path) < 0)
			perror("event ioctl -- error reading device address");

		printf("The device on %s says its path is %s\n",
			fd_source, path);

		// ioctl -- unique ID
		if (ioctl(fd, EVIOCGUNIQ(sizeof(uID)), uID) < 0)
			perror("event ioctl -- error reading unique ID");

		printf("The device on %s says its identity is %s\n",
			fd_source, uID);

		// ioctl -- event capabilities
		if (ioctl(fd, EVIOCGBIT(0, EV_MAX), evtype_b) < 0)
			perror("evdev ioctl -- error reading event capabilities");

		printf("Supported event types:\n");

		for (int i = 0; i < EV_MAX; i++)
		{
			// if the bit is in the list as 1
			if (evtype_b[i]) {
				printf("%2sEvent type 0x%02x "," ", i);
				switch (i)
				{
					case EV_SYN :
						printf(" (Synch Events)\n");
						break;
					case EV_KEY :
						printf(" (Keys or Buttons)\n");
						break;
					case EV_REL :
						printf(" (Relative Axes)\n");
						break;
					case EV_ABS :
						printf(" (Absolute Axes)\n");
						break;
					case EV_MSC :
						printf(" (Miscellaneous)\n");
						break;
					case EV_LED :
						printf(" (LEDs)\n");
						break;
					case EV_SND :
						printf(" (Sounds)\n");
						break;
					case EV_REP :
						printf(" (Repeat)\n");
						break;
					case EV_FF :
					case EV_FF_STATUS:
						printf(" (Force Feedback)\n");
						break;
					case EV_PWR:
						printf(" (Power Management)\n");
						break;
					default:
						printf(" (Unknown: 0x%04hx)\n", i);
				}
			}
		}

		// testing putting read in an infinite loop
		// stop is sig_atomic_t defined global in main.h
		while (!stop)
		{
			// We know the device exists, read all the info
			// we can. Now lets actually read input...
			//
			// Setting up read buffer
			rb = read(fd, ev, sizeof(struct input_event)*64);
	
			// make sure events are the right size
			if (rb < (int) sizeof(struct input_event)) {
				perror("evtest: short read");
				exit(1);
			}

			/*printf("%ld.%06ld ",
				ev[0].time.tv_sec,
				ev[0].time.tv_usec);
			printf("type %d code %d value %d\n",
				ev[0].type,
				ev[0].code,
				ev[0].value);
			*/
			wait();
			switch (ev[0].code)
			{
				case 0: // left-x (left +)
					joysticks.yaw = handle_input(ev[0].value);
					break;
				case 1: // left-y (down +)
					joysticks.throttle = handle_input(ev[0].value);
					break;
				case 3: // right-x (left +)
					joysticks.roll = handle_input(ev[0].value);
					break;
				case 4: // right-y (down +)
					joysticks.pitch = handle_input(ev[0].value);
					break;
				default:
					printf("unmapped button pressed.\n");
			}
			release();
		}
	}

	printf("Exitting, closing file descriptor...\n");
	close(fd);
}

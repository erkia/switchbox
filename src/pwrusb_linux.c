#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <libudev.h>
#include <termios.h>
#include <errno.h>

#include "pwrusb_internal.h"


LIBPWRUSB_DLL_EXPORTED int pwrusb_search (const char *search_serial, char *buf, size_t buflen)
{
    size_t ret = -1;
    struct udev *udev;
    struct udev_enumerate *enumerate;
    struct udev_list_entry *devices, *dev_list_entry;
    struct udev_device *dev, *parent;
    const char *serial, *node;

    if (strlen (search_serial) == 0) {
		return -1;
	}

	// Create the udev object
    udev = udev_new ();
    if (udev == NULL) {
        return -1;
    }

    // Create a list of the devices in the 'tty' subsystem
    enumerate = udev_enumerate_new (udev);
    udev_enumerate_add_match_subsystem (enumerate, "tty");
    udev_enumerate_scan_devices (enumerate);
    devices = udev_enumerate_get_list_entry (enumerate);

    udev_list_entry_foreach (dev_list_entry, devices) {

        // Get the filename of the /sys entry for the device and create a udev_device object representing it
	    dev = udev_device_new_from_syspath (udev, udev_list_entry_get_name (dev_list_entry));
        if (dev == NULL) {
            continue;
        }

	    parent = udev_device_get_parent_with_subsystem_devtype (dev, "usb", "usb_device");
	    if (parent == NULL) {
    	    udev_device_unref (dev);
            continue;
	    }

        serial = udev_device_get_sysattr_value (parent, "serial");

        if (serial != NULL && !strcasecmp (serial, search_serial)) {

            node = udev_device_get_devnode (dev);
            ret = strlen (node);
            if (ret < buflen) {
                strcpy (buf, node);
            } else {
                ret = -1;
            }

    	    udev_device_unref (dev);
            break;

        }

	    udev_device_unref (dev);

    }

    // Free objects
    udev_enumerate_unref (enumerate);
    udev_unref (udev);

    return ret;       
}


LIBPWRUSB_DLL_EXPORTED int pwrusb_init (pwrusb_ctx *ctx)
{
	ctx->fd = -1;
    return 0;
}



LIBPWRUSB_DLL_EXPORTED int pwrusb_open (pwrusb_ctx *ctx, const char *device)
{
    int fd;
    struct termios tty;

    fd = open (device, O_RDWR | O_NOCTTY | O_SYNC);
    if (fd == -1) {
        return -1;
    }
    memset (&tty, 0, sizeof tty);

    if (tcgetattr (fd, &tty) != 0) {
        close (fd);
        return -1;
    }

    cfsetospeed (&tty, B115200);
    cfsetispeed (&tty, B115200);

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars

    tty.c_iflag &= ~IGNBRK;                         // disable break processing
    tty.c_lflag = 0;                                // no signaling chars, no echo, no canonical processing
    tty.c_oflag = 0;                                // no remapping, no delays
    tty.c_cc[VMIN]  = 0;                            // read doesn't block
    tty.c_cc[VTIME] = 5;                            // 0.5 seconds read timeout

    tty.c_iflag &= ~(IXON | IXOFF | IXANY);         // shut off xon/xoff ctrl

    tty.c_cflag |= (CLOCAL | CREAD);                // ignore modem controls, enable reading
    tty.c_cflag &= ~(PARENB | PARODD);              // shut off parity
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;

    if (tcsetattr (fd, TCSANOW, &tty) != 0) {
        close (fd);
        return -1;
    }

    ctx->fd = fd;

    return 0;
}


LIBPWRUSB_DLL_EXPORTED int pwrusb_close (pwrusb_ctx *ctx)
{
    if (ctx->fd != -1) {
        return close (ctx->fd);
    } else {
        return -1;
    }
}


int pwrusb_get_state (pwrusb_ctx *ctx, int *state)
{
    uint8_t cmd = 0x80;

	*state = 0;

    if (write (ctx->fd, &cmd, 1) != 1) {
        return -1;
    }

    if (read (ctx->fd, (uint8_t *)state, 1) != 1) {
        return -1;
    }

    return 0;
}


int pwrusb_set_state (pwrusb_ctx *ctx, int *state)
{
    uint8_t cmd = 0xC0;

    if (write (ctx->fd, &cmd, 1) != 1) {
        return -1;
    }

    if (write (ctx->fd, (uint8_t *)state, 1) != 1) {
        return -1;
    }

    return 0;
}


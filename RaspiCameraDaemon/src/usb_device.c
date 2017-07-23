/*
 * The MIT License
 *
 * Copyright 2017 Riccardo Giannetto <rgiannetto at gmail.com>.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <stdlib.h>
#include <stdio.h>

#include <libusb-1.0/libusb.h>

int done = 0;
libusb_device_handle *handle = NULL;

static int LIBUSB_CALL hotplug_callback(libusb_context *ctx, libusb_device *dev, libusb_hotplug_event event, void *user_data)
{
	struct libusb_device_descriptor desc;
	int rc;

	rc = libusb_get_device_descriptor(dev, &desc);
	if (LIBUSB_SUCCESS != rc) {
		fprintf (stderr, "Error getting device descriptor\n");
	}

	//printf ("Device attached: %04x:%04x\n", desc.idVendor, desc.idProduct);
        printf("%04x:%04x (bus %d, device %d)\n",
            desc.idVendor, desc.idProduct,
            libusb_get_bus_number(dev), libusb_get_device_address(dev));        

	if (handle) {
		libusb_close (handle);
		handle = NULL;
	}

	rc = libusb_open (dev, &handle);
	if (LIBUSB_SUCCESS != rc) {
		fprintf (stderr, "Error opening device\n");
	}

	done++;

	return 0;
}

static int LIBUSB_CALL hotplug_callback_detach(libusb_context *ctx, libusb_device *dev, libusb_hotplug_event event, void *user_data)
{
	printf ("Device detached\n");

	if (handle) {
		libusb_close (handle);
		handle = NULL;
	}

	done++;

	return 0;
}


int rdc_usb_device_connection_init() {
	libusb_hotplug_callback_handle hp[2];
	int product_id, vendor_id, class_id;
	int rc;

	vendor_id  = LIBUSB_HOTPLUG_MATCH_ANY;
	product_id = LIBUSB_HOTPLUG_MATCH_ANY;
	class_id   = LIBUSB_HOTPLUG_MATCH_ANY;

	rc = libusb_init (NULL);
	if (rc < 0) {
		fprintf(stderr, "failed to initialise libusb: %s\n", libusb_error_name(rc));
		return EXIT_FAILURE;
	}

	if (!libusb_has_capability (LIBUSB_CAP_HAS_HOTPLUG)) {
		fprintf(stderr, "Hotplug capabilites are not supported on this platform\n");
		libusb_exit (NULL);
		return EXIT_FAILURE;
	}

	rc = libusb_hotplug_register_callback (NULL, LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED, 0, vendor_id,
		product_id, class_id, hotplug_callback, NULL, &hp[0]);
	if (LIBUSB_SUCCESS != rc) {
		fprintf (stderr, "Error registering callback 0\n");
		libusb_exit (NULL);
		return EXIT_FAILURE;
	}

	rc = libusb_hotplug_register_callback (NULL, LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT, 0, vendor_id,
		product_id,class_id, hotplug_callback_detach, NULL, &hp[1]);
	if (LIBUSB_SUCCESS != rc) {
		fprintf (stderr, "Error registering callback 1\n");
		libusb_exit (NULL);
		return EXIT_FAILURE;
	}

	while (1) {
		rc = libusb_handle_events (NULL);
		if (rc < 0)
			fprintf(stderr, "libusb_handle_events() failed: %s\n", libusb_error_name(rc));
	}

	if (handle) {
		libusb_close (handle);
	}

	libusb_exit (NULL);

	return EXIT_SUCCESS;
}

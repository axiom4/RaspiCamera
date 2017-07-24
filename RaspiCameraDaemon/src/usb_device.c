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


#include <RaspiCameraDaemon.h>
#include <libusb-1.0/libusb.h>

libusb_device_handle *handle = NULL;

static int LIBUSB_CALL hotplug_callback(libusb_context *ctx, libusb_device *dev, libusb_hotplug_event event, void *user_data) {
    struct libusb_device_descriptor desc;
    int rc;

    rc = libusb_get_device_descriptor(dev, &desc);
    if (LIBUSB_SUCCESS != rc) {
        perr("Error getting device descriptor\n");
    }

    //printf ("Device attached: %04x:%04x\n", desc.idVendor, desc.idProduct);
    pinfo("%04x:%04x (bus %d, device %d) - Device attached\n",
            desc.idVendor, desc.idProduct,
            libusb_get_bus_number(dev), libusb_get_device_address(dev));

    /*
        if (handle) {
            libusb_close(handle);
            handle = NULL;
        }

        rc = libusb_open(dev, &handle);
        if (LIBUSB_SUCCESS != rc) {
            perr("Error opening device\n");
        }
     */

    return 0;
}

static int LIBUSB_CALL hotplug_callback_detach(libusb_context *ctx, libusb_device *dev, libusb_hotplug_event event, void *user_data) {
    struct libusb_device_descriptor desc;
    int rc;

    rc = libusb_get_device_descriptor(dev, &desc);
    if (LIBUSB_SUCCESS != rc) {
        perr("Error getting device descriptor\n");
    }

    pinfo("%04x:%04x (bus %d, device %d) - Device detached\n",
            desc.idVendor, desc.idProduct,
            libusb_get_bus_number(dev), libusb_get_device_address(dev));

    /*
        if (handle) {
            libusb_close(handle);
            handle = NULL;
        }
     */

    return 0;
}

static void print_devs(libusb_device **devs) {
        libusb_device *dev;
        int i = 0, j = 0;
        uint8_t path[8];

        while ((dev = devs[i++]) != NULL) {
                struct libusb_device_descriptor desc;
                int r = libusb_get_device_descriptor(dev, &desc);
                if (r < 0) {
                        perr("failed to get device descriptor\n");
                        return;
                }

                pinfo("USB %04x:%04x (bus %d, device %d) - Was Found\n",
                        desc.idVendor, desc.idProduct,
                        libusb_get_bus_number(dev), libusb_get_device_address(dev));
        }
}

void *rcd_usb_device_connection_init(void *t) {
    libusb_hotplug_callback_handle hp[2];
    int product_id, vendor_id, class_id;
    int rc;
    libusb_device **devs;
    int cnt;

    struct timeval tm;

    tm.tv_sec = 1;
    tm.tv_usec = 0;

    vendor_id = LIBUSB_HOTPLUG_MATCH_ANY;
    product_id = LIBUSB_HOTPLUG_MATCH_ANY;
    class_id = LIBUSB_HOTPLUG_MATCH_ANY;

    pinfo("Usb detect thread has been created\n");

    pdebug("initiating libusb\n");
    rc = libusb_init(NULL);
    if (rc < 0) {
        perr("failed to initialise libusb: %s\n", libusb_error_name(rc));
        pthread_exit(NULL);
    }

    pdebug("search usb attached device\n");
    cnt = libusb_get_device_list(NULL, &devs);
    if (cnt > 0)
        print_devs(devs);
    
    libusb_free_device_list(devs, 1);

    if (!libusb_has_capability(LIBUSB_CAP_HAS_HOTPLUG)) {
        perr("Hotplug capabilites are not supported on this platform\n");
        libusb_exit(NULL);
        pthread_exit(NULL);
    }

    pdebug("register usb hotplug attached device callback\n");

    rc = libusb_hotplug_register_callback(NULL, LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED, 0, vendor_id,
            product_id, class_id, hotplug_callback, NULL, &hp[0]);
    
    if (LIBUSB_SUCCESS != rc) {
        perr("Error registering callback 0\n");
        libusb_exit(NULL);

        pthread_exit(NULL);
    }

    pdebug("register usb hotplug detached device callback\n");

    rc = libusb_hotplug_register_callback(NULL, LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT, 0, vendor_id,
            product_id, class_id, hotplug_callback_detach, NULL, &hp[1]);
    
    if (LIBUSB_SUCCESS != rc) {

        libusb_hotplug_deregister_callback(NULL, hp[0]);
        perr("Error registering callback 1\n");
        libusb_exit(NULL);
        pthread_exit(NULL);
    }

    pdebug("running libusb hotplug handle events monitoring\n");

    while (!rcd_exit) {
        rc = libusb_handle_events_timeout(NULL, &tm);

        if (rc < 0)
            perr("libusb_handle_events() failed: %s\n", libusb_error_name(rc));
    }

    pdebug("deregister libusb hotplug callback\n");

    libusb_hotplug_deregister_callback(NULL, hp[0]);
    libusb_hotplug_deregister_callback(NULL, hp[1]);

/*
    if (handle) {
        libusb_close(handle);
    }
*/
    pdebug("deinit libusb\n");

    libusb_exit(NULL);

    pthread_exit(NULL);
}

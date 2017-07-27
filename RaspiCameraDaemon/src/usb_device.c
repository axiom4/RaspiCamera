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

static void add_device(libusb_device *dev) {
    char *port;
    RcdCameraObj *camera;
    struct libusb_device_descriptor desc;
    int r = libusb_get_device_descriptor(dev, &desc);

    if (r < 0) {
        perr("failed to get device descriptor");
        return;
    }

    pdebug("USB %04x:%04x (bus %d, device %d) - attached",
            desc.idVendor, desc.idProduct,
            libusb_get_bus_number(dev), libusb_get_device_address(dev));


    asprintf(&port, "usb:%03d,%03d", libusb_get_bus_number(dev), libusb_get_device_address(dev));

    if ((camera = newCamera(desc.idVendor, desc.idProduct, port)))
        add_new_camera_list(&config.camera_list, camera);

    print_camera_list(&config.camera_list);

    free(port);
}

static void delete_device(libusb_device *dev) {
    char *port;
    struct camera_list_elem *camera;
    struct libusb_device_descriptor desc;
    int r = libusb_get_device_descriptor(dev, &desc);

    if (r < 0) {
        perr("failed to get device descriptor");
        return;
    }

    pdebug("USB %04x:%04x (bus %d, device %d) - detached",
            desc.idVendor, desc.idProduct,
            libusb_get_bus_number(dev), libusb_get_device_address(dev));


    asprintf(&port, "usb:%03d,%03d", libusb_get_bus_number(dev), libusb_get_device_address(dev));

    if ((camera = search_camera_list(&config.camera_list, port))) {
        pinfo("Remove Camera: %s (%s)", camera->camera->camera_name, camera->camera->camera_port);
        free(camera->camera->camera_name);
        free(camera->camera->camera_port);
        
        delete_camera_list(&config.camera_list, camera);
    }

    print_camera_list(&config.camera_list);

    free(port);
}

static int LIBUSB_CALL hotplug_callback(libusb_context *ctx, libusb_device *dev, libusb_hotplug_event event, void *user_data) {
    struct libusb_device_descriptor desc;
    int rc;

    rc = libusb_get_device_descriptor(dev, &desc);
    if (LIBUSB_SUCCESS != rc) {
        perr("Error getting device descriptor");
    }

    add_device(dev);

    return 0;
}

static int LIBUSB_CALL hotplug_callback_detach(libusb_context *ctx, libusb_device *dev, libusb_hotplug_event event, void *user_data) {
    struct libusb_device_descriptor desc;
    int rc;

    rc = libusb_get_device_descriptor(dev, &desc);
    if (LIBUSB_SUCCESS != rc) {
        perr("Error getting device descriptor");
    }

    delete_device(dev);

    return 0;
}

static void add_dev_list(libusb_device **devs) {
    libusb_device *dev;
    int i = 0;

    while ((dev = devs[i++]) != NULL) {
        add_device(dev);
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

    pinfo("Usb detect thread has been created");

    pdebug("initiating libusb");
    rc = libusb_init(NULL);
    if (rc < 0) {
        perr("failed to initialise libusb: %s", libusb_error_name(rc));
        pthread_exit(NULL);
    }

    pdebug("search usb attached device");
    cnt = libusb_get_device_list(NULL, &devs);
    if (cnt > 0) {
        add_dev_list(devs);
    }

    libusb_free_device_list(devs, 1);

    if (!libusb_has_capability(LIBUSB_CAP_HAS_HOTPLUG)) {
        perr("Hotplug capabilites are not supported on this platform");
        libusb_exit(NULL);
        pthread_exit(NULL);
    }

    pdebug("register usb hotplug attached device callback");

    rc = libusb_hotplug_register_callback(NULL, LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED, 0, vendor_id,
            product_id, class_id, hotplug_callback, NULL, &hp[0]);

    if (LIBUSB_SUCCESS != rc) {
        perr("Error registering attached device callback ");
        libusb_exit(NULL);

        pthread_exit(NULL);
    }

    pdebug("register usb hotplug detached device callback");

    rc = libusb_hotplug_register_callback(NULL, LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT, 0, vendor_id,
            product_id, class_id, hotplug_callback_detach, NULL, &hp[1]);

    if (LIBUSB_SUCCESS != rc) {

        libusb_hotplug_deregister_callback(NULL, hp[0]);
        perr("Error registering  detached device callback");
        libusb_exit(NULL);
        pthread_exit(NULL);
    }

    pdebug("running libusb hotplug handle events monitoring");

    while (!rcd_exit) {
        rc = libusb_handle_events_timeout(NULL, &tm);

        if (rc < 0)
            perr("libusb_handle_events() failed: %s", libusb_error_name(rc));
    }

    pdebug("deregister libusb hotplug callback");

    libusb_hotplug_deregister_callback(NULL, hp[0]);
    libusb_hotplug_deregister_callback(NULL, hp[1]);

    pdebug("deinit libusb");

    libusb_exit(NULL);

    pthread_exit(NULL);
}

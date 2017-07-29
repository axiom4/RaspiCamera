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

#include <getopt.h>
#include <gphoto2/gphoto2/gphoto2-port.h>

int rcd_exit = 0;
RcdRunConfig config;

void rcd_usage(int argc, char**argv) {
    printf(
            "Usage: %s [OPTIONS]\n\n"
            "  -c [CONFIG], --config [CONFIG]\n\t\tSpecify config file [required]\n"
            "  -d, --debug\n\t\tEnable debug log\n"
            "  -D, --daemon\n\t\tEnable process background\n"
            "  -h, --help\n\t\tThis help\n", config.app_name);
    exit(1);
}

static struct option long_options[] = {
    {"config", required_argument, NULL, 'c'},
    {"debug", no_argument, NULL, 'd'},
    {"daemon", no_argument, NULL, 'D'},
    {"help", no_argument, NULL, 'h'},
    {NULL, 0, NULL, 0}
};

int main(int argc, char** argv) {
    int c, ret;
    int option_index = 0;

    rcdConfigInit(&config);

    config.app_name = basename(argv[0]);

    rcdSignal(SIGINT, &rcdSignalTerm);
    rcdSignal(SIGTERM, &rcdSignalTerm);

    rcdSignal(SIGPIPE, &rcdSignalPipe);

    openlog(config.app_name, 0, LOG_DAEMON);

    while ((c = getopt_long(argc, argv, "c:Dh", long_options, &option_index)) != -1) {
        switch (c) {
            case 'c':
                config.configfile = optarg;
                break;
            case 'd':
                config.debug = 1;
                break;
            case 'D':
                config.daemonize = 1;
                break;
            case 'h':
                rcd_usage(argc, argv);
                break;
            default:
                rcd_usage(argc, argv);
                break;
        }
    }

    if (!config.configfile) {
        rcd_usage(argc, argv);
    }

    pinfo("Starting %s", config.app_name);

    rcdConfigParse(config.configfile, &config);

    if (rcd_exit)
        exit(-1);

    if (config.daemonize)
        rcdDaemonInit();

    pinfo("controller initialization");
    controllerSocketInit();

    pinfo("init camera module");
    gphotoInit();

    pthread_attr_init(&config.t_usb_detect.attr);
    pthread_attr_setdetachstate(&config.t_usb_detect.attr, PTHREAD_CREATE_JOINABLE);

    ret = pthread_create(&config.t_usb_detect.thread, &config.t_usb_detect.attr, rcdUsbDeviceConnectionInit, NULL);

    if (ret) {
        perr("return code from pthread_create() is %d", ret);
        exit(-1);
    }


    while (!rcd_exit) {
        controllerAccept();
    }

    pinfo("closing controller");
    controllerSocketFree();

    pthread_attr_destroy(&config.t_usb_detect.attr);
    ret = pthread_join(config.t_usb_detect.thread, &config.t_usb_detect.status);
    if (ret) {
        perr("return code from pthread_join() is %d", ret);
        exit(-1);
    }
    pinfo("Main: completed join with Usb Detect Thread having a status of % ld", (long) config.t_usb_detect.status);

    closelog();

    pinfo("free camera module");
    gphotoFree();

    pinfo("Free camera list memory");
    freeCameraList(&config.camera_list);

    pinfo("%s terminated!", config.app_name);

    rcdConfigFree(&config);

    pthread_exit(NULL);
}

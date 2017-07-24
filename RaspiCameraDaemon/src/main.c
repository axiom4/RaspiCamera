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

/* 
 * File:   main.c
 * Author: Riccardo Giannetto <rgiannetto@gmail.com>
 *
 * Created on 16 luglio 2017, 19.00
 */

#include <RaspiCameraDaemon.h>

#include <getopt.h>

int rcd_exit = 0;
RcdRunConfig config;

void rcd_usage(int argc, char**argv) {
    printf(
            "Usage: %s [OPTIONS]\n\n"
            "  -c [CONFIG], --config [CONFIG]\n\t\tSpecify config file [required]\n"
            "  -D, --daemon\n\t\tEnable process background\n"
            "  -h, --help\n\t\tThis help\n", basename(argv[0]));
    exit(1);
}

static struct option long_options[] = {
    {"config", required_argument, NULL, 'c'},
    {"daemon", no_argument, NULL, 'D'},
    {"help", no_argument, NULL, 'h'},
    {NULL, 0, NULL, 0}
};

int main(int argc, char** argv) {
    int c, ret;
    int option_index = 0;
    
    config.configfile = NULL;
    config.daemonize = 0;

    rcd_signal(SIGINT, &rcd_sig_term);
    rcd_signal(SIGTERM, &rcd_sig_term);
    
    rcd_signal(SIGPIPE, &rcd_sig_pipe);
    
    openlog(basename(argv[0]), 0, LOG_DAEMON);

    while ((c = getopt_long(argc, argv, "c:Dh", long_options, &option_index)) != -1) {
        switch (c) {
            case 'c':
                config.configfile = optarg;
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

    rcd_config_parse(config.configfile);

    if (config.daemonize)
        rcd_daemon_init();


    pthread_attr_init(&config.t_usb_detect.attr);
    pthread_attr_setdetachstate(&config.t_usb_detect.attr, PTHREAD_CREATE_JOINABLE);

    ret = pthread_create(&config.t_usb_detect.thread, &config.t_usb_detect.attr, rcd_usb_device_connection_init, NULL);

    if (ret) {
        perr("return code from pthread_create() is %d\n", ret);
        exit(-1);
    }

    while (!rcd_exit) {
        pause();
    }

    pthread_attr_destroy(&config.t_usb_detect.attr);
    ret = pthread_join(config.t_usb_detect.thread, &config.t_usb_detect.status);
    if (ret) {
        perr("return code from pthread_join() is %d\n", ret);
        exit(-1);
    }
    pinfo("Main: completed join with Usb Detect Thread having a status of % ld\n",(long)config.t_usb_detect.status);
    
    closelog();
    
    pthread_exit(NULL);
}


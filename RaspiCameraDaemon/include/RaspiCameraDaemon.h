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
 * File:   RaspiCameraDaemon.h
 * Author: rgiannetto
 *
 * Created on 23 luglio 2017, 7.45
 */

#ifndef RASPICAMERADAEMON_H
#define RASPICAMERADAEMON_H

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <ctype.h>

#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <pthread.h>

#include <gphoto2/gphoto2.h>


#define MAXFD 64

static GPPortInfoList *portinfolist = NULL;
static CameraAbilitiesList *abilities = NULL;

extern int rcd_exit;

struct _ThreadData {
    Camera *camera;
    time_t timeout;
    CameraTimeoutFunc func;
};

typedef struct _ThreadData ThreadData;

struct _RcdRunConfig {
    const char *configfile;
    int daemonize;
};

typedef struct _RcdRunConfig RcdRunConfig;

GPContext* create_context();

void (*rcd_signal(int signo, void (*func)(int)))(int);
void rcd_sig_pipe(int signo);
void rcd_sig_term(int signo);

int rcd_daemon_init();
void *rcd_usb_device_connection_init(void *t);
void rcd_config_parse(const char *filename);

#endif /* RASPICAMERADAEMON_H */


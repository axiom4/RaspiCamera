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

#include <syslog.h>

#include <gphoto2/gphoto2.h>

/* #define */
#define MAXFD 64
#define MAXLINE 4096

/* datatypes */
struct _ThreadData {
    Camera *camera;
    time_t timeout;
    CameraTimeoutFunc func;
};

struct _UsbThreadData {
    pthread_t thread;
    pthread_attr_t attr;
    void *status;
};

typedef struct _UsbThreadData UsbThreadData;

typedef struct _ThreadData ThreadData;

struct RcdDaemonConfig {
    char *log_facility;
};

struct RcdCameraConfig {
    int camera_timeout;
};

typedef struct camera_elem {
    char *camera_port;
    Camera *camera;
    
    struct camera_elem *next;
} camera_elem; 

typedef struct camera_elem * camera_list;

struct _RcdRunConfig {
    const char *configfile;
    const char *app_name;
    int daemonize;

    struct RcdDaemonConfig rcd_config;
    struct RcdCameraConfig camera_config;

    UsbThreadData t_usb_detect;
    
    camera_list *camera_list;
};

typedef struct _RcdRunConfig RcdRunConfig;

/* global declare */
extern int rcd_exit;
extern RcdRunConfig config;
static GPPortInfoList *portinfolist = NULL;
static CameraAbilitiesList *abilities = NULL;


/* camera-utils.c */
GPContext* create_context();

/* signal.c */
void (*rcd_signal(int signo, void (*func)(int)))(int);
void rcd_sig_pipe(int signo);
void rcd_sig_term(int signo);

/* daemon.c */
int rcd_daemon_init();

/* usb_device.c */
void *rcd_usb_device_connection_init(void *t);

/* config.c */
void rcd_config_parse(const char *filename, RcdRunConfig *config);

/* logging.c */
void pinfo(char *s, ...);
void pdebug(char *s, ...);
void perr(char *s, ...);
void pwarn(char *s, ...);
void rcd_perror(const char *str);

/* io.c */
ssize_t rcd_readline(int fd, void *vptr, size_t maxlen);
ssize_t rcd_writeline(int fd, const char *str);

/* utils.c */
char * rcdStringToLower(const char *string);
char * rcdStringToUpper(const char *string);
int rcdCompareString(const char *s1, const char *s2, long len);
char *rcdStringCat(char *string1, char *string2);
char * rcdTrim(char * string);
void rcdChomp(char *str);

/* camera-list.c */
int add_new_camera(camera_list *list, Camera *camera);
int delete_camera(camera_list *list, camera_elem *elem);
void print_camera_list(camera_list *list);
struct camera_elem *search_camera(camera_list *list, char *camera_port);

#endif /* RASPICAMERADAEMON_H */


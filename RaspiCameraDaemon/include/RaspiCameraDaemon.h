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

#include <sys/socket.h>
#include <sys/un.h>

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

struct _RcdThreadData {
    int thread_exit;
    pthread_t thread;
    pthread_attr_t attr;
    void *status;
    
    pthread_mutex_t mutex;
};

typedef struct _RcdThreadData RcdThreadData;

typedef struct _ThreadData ThreadData;

struct RcdDaemonConfig {
    char *socket_controller_s;
    char *log_facility;
};

struct RcdCameraConfig {
    int camera_timeout;
};

typedef struct camera_object {
    Camera *camera;
    char *camera_port;
    char *camera_name;
    
    GPContext *context;
    
    RcdThreadData t_camera_monitor;    
} RcdCameraObj;

typedef struct camera_list_elem {
    RcdCameraObj *camera;
    
    struct camera_list_elem *next;
} camera_list_elem; 

typedef struct camera_list_elem camera_list;

struct _RcdRunConfig {
    const char *configfile;
    const char *app_name;
    int daemonize;
    int debug;

    struct RcdDaemonConfig rcd_config;
    struct RcdCameraConfig camera_config;

    RcdThreadData t_usb_detect;
    
    camera_list *camera_list;
    
    int controller_socket;

    GPContext* context;
};

typedef struct _RcdRunConfig RcdRunConfig;

/* global declare */
extern int rcd_exit;
extern RcdRunConfig config;

/* signal.c */
void (*rcd_signal(int signo, void (*func)(int)))(int);
void rcd_sig_pipe(int signo);
void rcd_sig_term(int signo);

/* daemon.c */
int rcd_daemon_init();

/* usb_device.c */
void *rcd_usb_device_connection_init(void *t);

/* config.c */
void config_init(RcdRunConfig *config);
void rcd_config_parse(const char *filename, RcdRunConfig *config);
void config_free(RcdRunConfig *config);

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
int add_new_camera_list(camera_list **list, RcdCameraObj *camera);
int delete_camera_list(camera_list **list, camera_list_elem *elem);
void print_camera_list(camera_list **list);
struct camera_list_elem *search_camera_list(camera_list **list, char *camera_port);

/* camera-utils.c */
GPContext* create_context();
int gphoto_init();
void gphoto_free();
RcdCameraObj * newCamera(int idVendor, int productId, char *camera_port);
void freeCamera(RcdCameraObj *camera);
void free_camera_list(camera_list **list);

/* rcd-controller */
void controller_socket_init();

#endif /* RASPICAMERADAEMON_H */


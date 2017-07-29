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

typedef struct CameraObject {
    Camera *camera;
    char *camera_port;
    char *camera_name;
    
    GPContext *context;
    
    RcdThreadData t_camera_monitor;    
} RcdCameraObj;

typedef struct CameraListElem {
    RcdCameraObj *camera;
    
    struct CameraListElem *next;
} CameraListElem; 

typedef struct CameraListElem CameraList;

struct _RcdRunConfig {
    const char *configfile;
    const char *app_name;
    int daemonize;
    int debug;

    struct RcdDaemonConfig rcd_config;
    struct RcdCameraConfig camera_config;

    RcdThreadData t_usb_detect;
    
    CameraList *camera_list;
    
    int controller_socket;

    GPContext* context;
};

typedef struct _RcdRunConfig RcdRunConfig;

/* global declare */
extern int rcd_exit;
extern RcdRunConfig config;

/* signal.c */
void (*rcdSignal(int signo, void (*func)(int)))(int);
void rcdSignalPipe(int signo);
void rcdSignalTerm(int signo);

/* daemon.c */
int rcdDaemonInit();

/* usb_device.c */
void *rcdUsbDeviceConnectionInit(void *t);

/* config.c */
void rcdConfigInit(RcdRunConfig *config);
void rcdConfigParse(const char *filename, RcdRunConfig *config);
void rcdConfigFree(RcdRunConfig *config);

/* logging.c */
void pinfo(char *s, ...);
void pdebug(char *s, ...);
void perr(char *s, ...);
void pwarn(char *s, ...);
void rcd_perror(const char *str);

/* io.c */
ssize_t rcdReadline(int fd, void *vptr, size_t maxlen);
ssize_t rcdWriteline(int fd, const char *str);
int rcdAccept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
ssize_t rcdReadNBytes(int fd, void *vptr, size_t n);
size_t rcdRead(int fd, void *buf, size_t count);

/* utils.c */
char * rcdStringToLower(const char *string);
char * rcdStringToUpper(const char *string);
int rcdCompareString(const char *s1, const char *s2, long len);
char *rcdStringCat(char *string1, char *string2);
char * rcdTrim(char * string);
void rcdChomp(char *str);

/* camera-list.c */
int addNewCameraList(CameraList **list, RcdCameraObj *camera);
int deleteCameraList(CameraList **list, CameraListElem *elem);
void printCameraList(CameraList **list);
struct CameraListElem *searchCameraList(CameraList **list, char *camera_port);

/* camera-utils.c */
GPContext* createContext();
int gphotoInit();
void gphotoFree();
RcdCameraObj * newCamera(int idVendor, int productId, char *camera_port);
void freeCamera(RcdCameraObj *camera);
void freeCameraList(CameraList **list);

/* rcd-controller */
void controllerSocketInit();
void controllerAccept();
void controllerSocketFree();

#endif /* RASPICAMERADAEMON_H */


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

<<<<<<< HEAD
int addNewCameraList(RcdCameraList **list, RcdCameraObj *camera) {
    RcdCameraListElem *elem, *ptr;

    if (!(elem = malloc(sizeof (RcdCameraListElem)))) {
=======
int addNewCameraList(CameraList **list, RcdCameraObj *camera) {
    CameraListElem *elem, *ptr;

    if (!(elem = malloc(sizeof (CameraListElem)))) {
>>>>>>> ca61faa583a9f4677977a90d6a36d6b63efa9210
        rcd_perror("malloc");

        return -1;
    }

    elem->camera = camera;
    elem->next = NULL;

    if (!*list) {
        *list = elem;
    } else {
        ptr = *list;

        while (ptr->next) {
            ptr = ptr->next;
        }

        ptr->next = elem;
    }

    return 0;
}

<<<<<<< HEAD
void printCameraList(RcdCameraList **list) {
    RcdCameraListElem *ptr = *list;
=======
void printCameraList(CameraList **list) {
    CameraListElem *ptr = *list;
>>>>>>> ca61faa583a9f4677977a90d6a36d6b63efa9210

    while (ptr) {
        pdebug("Camera list element: %s - %s", ptr->camera->camera_name, ptr->camera->camera_port);
        ptr = ptr->next;
    }
}

<<<<<<< HEAD
RcdCameraListElem *searchCameraList(RcdCameraList **list, char *camera_port) {
    RcdCameraListElem *ptr = *list;
=======
CameraListElem *searchCameraList(CameraList **list, char *camera_port) {
    CameraListElem *ptr = *list;
>>>>>>> ca61faa583a9f4677977a90d6a36d6b63efa9210

    while (ptr) {
        if (!rcdCompareString(ptr->camera->camera_port, camera_port, strlen(ptr->camera->camera_port)))
            return ptr;

        ptr = ptr->next;
    }

    return NULL;
}

<<<<<<< HEAD
int deleteCameraList(RcdCameraList **list, RcdCameraListElem *elem) {
    RcdCameraListElem *ptr = *list;
    RcdCameraListElem *prev = NULL;
=======
int deleteCameraList(CameraList **list, CameraListElem *elem) {
    CameraListElem *ptr = *list;
    CameraListElem *prev = NULL;
>>>>>>> ca61faa583a9f4677977a90d6a36d6b63efa9210

    while (ptr && ptr != elem) {
        prev = ptr;
        ptr = ptr->next;
    }
    
    if(ptr) {        
        if(!prev) {
            *list = ptr->next;
        } else {
            prev->next = ptr->next;
        }
        
        if(ptr->camera)
            free(ptr->camera);
        
        free(ptr);
    }

    return 0;
}

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

int add_new_camera_list(camera_list **list, RcdCameraObj *camera) {
    camera_list_elem *elem, *ptr;

    if (!(elem = malloc(sizeof (camera_list_elem)))) {
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

void print_camera_list(camera_list **list) {
    camera_list_elem *ptr = *list;

    while (ptr) {
        pdebug("Camera list element: %s - %s", ptr->camera->camera_name, ptr->camera->camera_port);
        ptr = ptr->next;
    }
}

camera_list_elem *search_camera_list(camera_list **list, char *camera_port) {
    camera_list_elem *ptr = *list;

    while (ptr) {
        if (!rcdCompareString(ptr->camera->camera_port, camera_port, strlen(ptr->camera->camera_port)))
            return ptr;

        ptr = ptr->next;
    }

    return NULL;
}

int delete_camera_list(camera_list **list, camera_list_elem *elem) {
    camera_list_elem *ptr = *list;
    camera_list_elem *prev = NULL;

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

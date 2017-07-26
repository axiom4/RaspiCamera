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

GPPortInfoList *portinfolist = NULL;
CameraAbilitiesList *abilities = NULL;
GPContext* context = NULL;

static void ctx_error_func(GPContext *context, const char *str, void *data) {
    fprintf(stderr, "\n*** Contexterror ***              \n%s\n", str);
    fflush(stderr);
}

static void ctx_status_func(GPContext *context, const char *str, void *data) {
    fprintf(stderr, "%s\n", str);
    fflush(stderr);
}

GPContext* create_context() {
    GPContext *context;

    /* This is the mandatory part */
    context = gp_context_new();

    /* All the parts below are optional! */
    gp_context_set_error_func(context, ctx_error_func, NULL);
    gp_context_set_status_func(context, ctx_status_func, NULL);

    /* also:
    gp_context_set_cancel_func    (p->context, ctx_cancel_func,  p);
    gp_context_set_message_func   (p->context, ctx_message_func, p);
    if (isatty (STDOUT_FILENO))
            gp_context_set_progress_funcs (p->context,
                    ctx_progress_start_func, ctx_progress_update_func,
                    ctx_progress_stop_func, p);
     */
    return context;
}

int rcd_autodetect(CameraList *list, GPContext *context) {
    gp_list_reset(list);
    return gp_camera_autodetect(list, context);
}

static void
thread_cleanup_func(void *data) {
    ThreadData *td = data;

    free(td);
}

static void *
thread_func(void *data) {
    ThreadData *td = data;
    time_t t, last;

    pthread_cleanup_push(thread_cleanup_func, td);

    last = time(NULL);
    while (1) {
        t = time(NULL);
        if (t - last > td->timeout) {
            td->func(td->camera, NULL);
            last = t;
        }
        pthread_testcancel();
    }

    pthread_cleanup_pop(1);
}

static unsigned int start_timeout_func(Camera *camera, unsigned int timeout, CameraTimeoutFunc func, void *data) {
    pthread_t tid;
    ThreadData *td;
    printf("init start_timeout_func\n");
    td = malloc(sizeof (ThreadData));
    if (!td)
        return 0;
    memset(td, 0, sizeof (ThreadData));
    td->camera = camera;
    td->timeout = timeout;
    td->func = func;

    pthread_create(&tid, NULL, thread_func, td);

    return (tid);
}

static void stop_timeout_func(Camera *camera, unsigned int id, void *data) {
    pthread_t tid = id;

    pthread_cancel(tid);
    pthread_join(tid, NULL);
}

void folder_list_files(Camera *camera, const char *folder, GPContext *context) {
    CameraList *list;
    int i;

    gp_list_new(&list);

    printf("%s:\n", folder);

    gp_camera_folder_list_files(camera, folder, list, context);

    for (i = 0; i < gp_list_count(list); i++) {
        char *name = NULL;
        gp_list_get_name(list, i, (const char **) &name);

        printf("%s\n", name);
    }

    printf("\n\n");

    gp_list_free(list);
}

void folder_list_folders(Camera *camera, const char *folder, GPContext *context) {
    CameraList *list;
    int i;

    if (*folder == '\0')
        return;

    gp_list_new(&list);

    folder_list_files(camera, folder, context);
    gp_camera_folder_list_folders(camera, folder, list, context);

    for (i = 0; i < gp_list_count(list); i++) {
        char *name = NULL;
        char *subfolder = NULL;

        gp_list_get_name(list, i, (const char **) &name);

        if (!strcmp(folder, "/")) {
            asprintf(&subfolder, "%s%s", folder, name);
        } else {
            asprintf(&subfolder, "%s/%s", folder, name);
        }

        folder_list_folders(camera, subfolder, context);

        free(subfolder);
    }

    gp_list_free(list);
}

RcdCameraObj * newCamera(int idVendor, int productId, char *camera_port) {
    int i, count;
    RcdCameraObj *newCamera = NULL;

    CameraList *tmpCameraList = NULL;
    GPContext *context = create_context();

    const char *c_name = NULL;
    const char *c_port = NULL;

    if (gp_list_new(&tmpCameraList) == GP_OK) {
        count = rcd_autodetect(tmpCameraList, context);

        if (count > 0) {
            for (i = 0; i < count; i++) {
                gp_list_get_name(tmpCameraList, i, &c_name);
                gp_list_get_value(tmpCameraList, i, &c_port);

                if (!rcdCompareString(c_port, camera_port, strlen(c_port))) {
                    pinfo("Found camera: %s (%s)", c_name, c_port);
                    
                    newCamera = malloc(sizeof(RcdCameraObj));
                                        
                    asprintf(&newCamera->camera_name, "%s", c_name);
                    asprintf(&newCamera->camera_port, "%s", c_port);
                                        
                    break;
                }
            }
        }
    }

    if (tmpCameraList)
        gp_list_free(tmpCameraList);

    if (context)
        gp_context_unref(context);

    return newCamera;
}

int init_gphoto_camera_list() {
    int ret;

    context = gp_context_new();

    if (!portinfolist) {

        ret = gp_port_info_list_new(&portinfolist);

        if (ret < GP_OK)
            return ret;

        ret = gp_port_info_list_load(portinfolist);

        if (ret < 0)
            return ret;

        ret = gp_port_info_list_count(portinfolist);

        if (ret < 0)
            return ret;

        return GP_OK;
    }

    if (!abilities) {
        /* Load all the camera drivers we have... */
        ret = gp_abilities_list_new(&abilities);

        if (ret < GP_OK)
            return ret;

        ret = gp_abilities_list_load(abilities, context);

        if (ret < GP_OK)
            return ret;
    }

}

void free_gphoto_camera_list() {
    if (portinfolist)
        gp_port_info_list_free(portinfolist);

    if (abilities)
        gp_abilities_list_free(abilities);

    portinfolist = NULL;
    abilities = NULL;

    gp_context_unref(context);
}

void free_camera_list(camera_list **list) {
    camera_list_elem *ptr = *list;
    
    while(ptr) {
        free(ptr->camera->camera_name);
        free(ptr->camera->camera_port);
        
        delete_camera(list, ptr);
        ptr = *list;
    }
}
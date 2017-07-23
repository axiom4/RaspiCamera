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

/*
 * This function opens a camera depending on the specified model and port.
 */
int open_camera(Camera ** camera, const char *model, const char *port, GPContext *context) {
    int ret, m, p;
    CameraAbilities a;
    GPPortInfo pi;

    ret = gp_camera_new(camera);
    if (ret < GP_OK) return ret;

    if (!abilities) {
        /* Load all the camera drivers we have... */
        ret = gp_abilities_list_new(&abilities);

        if (ret < GP_OK) return ret;

        ret = gp_abilities_list_load(abilities, context);

        if (ret < GP_OK) return ret;
    }

    /* First lookup the model / driver */
    m = gp_abilities_list_lookup_model(abilities, model);
    if (m < GP_OK) return ret;
    ret = gp_abilities_list_get_abilities(abilities, m, &a);
    if (ret < GP_OK) return ret;
    ret = gp_camera_set_abilities(*camera, a);
    if (ret < GP_OK) return ret;

    if (!portinfolist) {
        /* Load all the port drivers we have... */
        ret = gp_port_info_list_new(&portinfolist);
        if (ret < GP_OK) return ret;
        ret = gp_port_info_list_load(portinfolist);
        if (ret < 0) return ret;
        ret = gp_port_info_list_count(portinfolist);
        if (ret < 0) return ret;
    }

    /* Then associate the camera with the specified port */
    p = gp_port_info_list_lookup_path(portinfolist, port);
    switch (p) {
        case GP_ERROR_UNKNOWN_PORT:
            fprintf(stderr, "The port you specified "
                    "('%s') can not be found. Please "
                    "specify one of the ports found by "
                    "'gphoto2 --list-ports' and make "
                    "sure the spelling is correct "
                    "(i.e. with prefix 'serial:' or 'usb:').",
                    port);
            break;
        default:
            break;
    }
    if (p < GP_OK) return p;

    ret = gp_port_info_list_get_info(portinfolist, p, &pi);
    if (ret < GP_OK) return ret;
    ret = gp_camera_set_port_info(*camera, pi);
    if (ret < GP_OK) return ret;
    return GP_OK;
}

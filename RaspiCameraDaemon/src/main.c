/*
 * Copyright (C) 2017 Riccardo Giannetto <rgiannetto@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */


/* 
 * File:   main.c
 * Author: Riccardo Giannetto <rgiannetto@gmail.com>
 *
 * Created on 16 luglio 2017, 19.00
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <gphoto2/gphoto2.h>

#include <pthread.h>

static GPPortInfoList *portinfolist = NULL;
static CameraAbilitiesList *abilities = NULL;

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

typedef struct _ThreadData ThreadData;

struct _ThreadData {
    Camera *camera;
    time_t timeout;
    CameraTimeoutFunc func;
};

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

sample_autodetect(CameraList *list, GPContext *context) {
    gp_list_reset(list);
    return gp_camera_autodetect(list, context);
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

/*
 * 
 */
int main(int argc, char** argv) {
    Camera **camera_array = NULL;
    GPContext *context = NULL;
    CameraText text;
    int i;
    int count;

    CameraList *list = NULL;
    gp_list_new(&list);

    int ret;

    context = create_context();

    ret = gp_camera_autodetect(list, context);

    if (ret < GP_OK) {
        fprintf(stderr, "Camera failed retrieving summary.\n");
        gp_context_unref(context);

        return EXIT_FAILURE;
    }

    count = gp_list_count(list);

    if (count) {

        camera_array = calloc(sizeof (Camera*), count);
        
        bzero(camera_array, sizeof (Camera*) * count);
        
        for (i = 0; i < count; i++) {
            const char *name, *value;
            gp_list_get_name(list, i, &name);
            gp_list_get_value(list, i, &value);

            ret = open_camera(&camera_array[i], name, value, context);

            if (ret < GP_OK)
                fprintf(stderr, "Camera %s on port %s failed to open\n", name, value);

            gp_camera_init(camera_array[i], context);

            ret = gp_camera_get_summary(camera_array[i], &text, context);
            if (ret < GP_OK) {
                fprintf(stderr, "Camera failed retrieving summary.\n");
                gp_context_unref(context);

                return EXIT_FAILURE;
            }

            printf("Summary:\n%s\n", text.text);

            CameraStorageInformation *info;

            int c;
            int store_count;

            gp_camera_get_storageinfo(camera_array[i], &info, &store_count, context);

            for (c = 0; c < store_count; c++) {
                folder_list_folders(camera_array[i], info[c].basedir, context);
            }

            free(info);
        }
    }

    gp_list_free(list);

    for (i = 0; i < count; i++) {
        gp_camera_exit(camera_array[i], context);
        gp_camera_unref(camera_array[i]);
    }

    free(camera_array);

    gp_port_info_list_free(portinfolist);
    gp_abilities_list_free(abilities);

    gp_context_unref(context);
    /*
        for (i = 0; i < count; i++) {
            char *name = NULL;
            char *value = NULL;

            gp_list_get_name(list, i, (const char **) &name);
            gp_list_get_value(list, i, (const char **) &value);

            printf("%s - %s\n", name, value);
        }

        gp_list_free(list);

        gp_abilities_list_new(&cAbilitiesList);

        gp_abilities_list_free(cAbilitiesList);

        gp_camera_new(&camera);

        if (gp_camera_init(camera, context) < GP_OK) {
            fprintf(stderr, "No camera Found!\n");
            gp_camera_unref(camera);
            gp_context_unref(context);

            return EXIT_FAILURE;
        }
        gp_camera_set_timeout_funcs(camera, (CameraTimeoutStartFunc) start_timeout_func, (CameraTimeoutStopFunc) stop_timeout_func, NULL);

        ret = gp_camera_get_summary(camera, &text, context);
        if (ret < GP_OK) {
            fprintf(stderr, "Camera failed retrieving summary.\n");
            gp_camera_unref(camera);
            gp_context_unref(context);

            return EXIT_FAILURE;
        }

        printf("Summary:\n%s\n", text.text);

        //folder_list_folders(camera, "/", context);

        gp_camera_exit(camera, context);
        gp_camera_unref(camera);
        gp_context_unref(context);
     */
    return (EXIT_SUCCESS);
}


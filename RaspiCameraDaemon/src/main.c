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

void rdc_usage(int argc, char**argv) {
    printf(
            "Usage: %s [ -c config ] [ -l log ] [ -h ]\n"
            "\t-c [config]\tconfig file\n"
            "\t-l [log]\tlog file\n"
            "\t-h\t\thelp\n", argv[0]);
    exit(1);
}

int main(int argc, char** argv) {
    Camera **camera_array = NULL;
    GPContext *context = NULL;
    CameraText text;
    int i;
    int count;
    int c;

    const char *configfile = NULL;
    const char *logfile = NULL;

    while ((c = getopt(argc, argv, "c:l:h")) != -1) {
        switch (c) {
            case 'c':
                configfile = optarg;
                break;
            case 'l':
                logfile = optarg;
                break;
            case 'h':
                rdc_usage(argc, argv);
                break;
            default:
                rdc_usage(argc, argv);
                break;
        }
    }
    //rcd_daemon_init();

    rdc_usb_device_connection_init();
    
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


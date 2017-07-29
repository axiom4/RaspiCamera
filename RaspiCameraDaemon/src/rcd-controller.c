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

void controllerSocketInit() {
    int ret;
    struct sockaddr_un name;

    unlink(config.rcd_config.socket_controller_s);

    config.controller_socket = socket(AF_UNIX, SOCK_SEQPACKET, 0);

    if (config.controller_socket < 0) {
        rcd_perror("controller socket");
        exit(EXIT_FAILURE);
    }

    bzero(&name, sizeof (struct sockaddr_un));

    name.sun_family = AF_UNIX;
    strncpy(name.sun_path, config.rcd_config.socket_controller_s, sizeof (name.sun_path) - 1);

    ret = bind(config.controller_socket, (const struct sockaddr_un *) &name, sizeof (struct sockaddr_un));
    if (ret < 0) {
        rcd_perror("controller socket bind");
        exit(EXIT_FAILURE);
    }

    ret = listen(config.controller_socket, 20);
    if (ret < 0) {
        perror("controller socket listen");
        exit(EXIT_FAILURE);
    }

}

void controllerAccept() {
    int data_socket;
    int result;
    char buffer[4096];
    /* Wait for incoming connection. */

    data_socket = rcdAccept(config.controller_socket, NULL, NULL);
    if (data_socket < 0) {
        rcd_perror("accept");
        return;
    }

    while (1) {

        result = rcdReadline(data_socket, buffer, 4096);

        if (result > 0)
            pinfo("%s", buffer);
        else if (result == 0) {
            close(data_socket);
            break;
        }
    }
}

void controllerSocketFree() {
    if (config.controller_socket > 0) {
        close(config.controller_socket);
    }
}
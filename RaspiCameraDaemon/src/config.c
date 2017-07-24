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

void rcd_config_parse(const char *filename) {
    int config_fd;
    struct stat sb;

    if (stat(filename, &sb) == -1) {
        rdc_perror("config file open");
        exit(errno);
    }

    if(sb.st_mode & S_IFMT != S_IFREG) {
        perr("%s: is not regular file\n", filename);
        exit(EXIT_FAILURE);
    }
    
    if ((config_fd = open(filename, 0, O_RDONLY)) < 0) {
        rdc_perror("config file open");
        exit(errno);
    }
    
    if(config_fd)
        close(config_fd);
}
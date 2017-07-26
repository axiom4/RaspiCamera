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
#include <stdarg.h>

void pdebug(char *s, ...) {
    va_list ap;
    char buf[1024];
    char *buf1 = NULL;
    time_t rawtime;
    struct tm * timeinfo;

    if (config.debug) {
        time(&rawtime);
        timeinfo = localtime(&rawtime);
        strftime(buf, 1023, "%b %d %H:%M:%S", timeinfo);

        va_start(ap, s);
        vasprintf(&buf1, s, ap);
        va_end(ap);

        if (config.daemonize) {
            syslog(LOG_DEBUG, "%s", buf1);
        } else {
            fprintf(stderr, "%s %s: DEBUG - %s\n", buf, config.app_name, buf1);
        }

        free(buf1);
    }
}

void pinfo(char *s, ...) {
    va_list ap;
    char buf[1024];
    char *buf1;
    time_t rawtime;
    struct tm * timeinfo;

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(buf, 1023, "%b %d %H:%M:%S", timeinfo);

    va_start(ap, s);
    vasprintf(&buf1, s, ap);
    va_end(ap);

    if (config.daemonize) {
        syslog(LOG_INFO, "%s", buf1);
    } else {
        fprintf(stderr, "%s %s: INFO - %s\n", buf, config.app_name, buf1);
    }

    free(buf1);
}

void perr(char *s, ...) {
    va_list ap;
    char buf[1024];
    char *buf1;
    time_t rawtime;
    struct tm * timeinfo;

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(buf, 1023, "%b %d %H:%M:%S", timeinfo);

    va_start(ap, s);
    vasprintf(&buf1, s, ap);
    va_end(ap);

    if (config.daemonize) {
        syslog(LOG_ERR, "%s", buf1);
    } else {
        fprintf(stderr, "%s %s: ERROR - %s\n", buf, config.app_name, buf1);
    }

    free(buf1);
}

void pwarn(char *s, ...) {
    va_list ap;
    char buf[1024];
    char *buf1;
    time_t rawtime;
    struct tm * timeinfo;

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(buf, 1023, "%b %d %H:%M:%S", timeinfo);

    va_start(ap, s);
    vasprintf(&buf1, s, ap);
    va_end(ap);

    if (config.daemonize) {
        syslog(LOG_WARNING, "%s", buf1);
    } else {
        fprintf(stderr, "%s %s: WARN - %s\n", buf, config.app_name, buf1);
    }

    free(buf1);
}

void rcd_perror(const char *str) {
    perr("%s: %s", str, strerror(errno));
}

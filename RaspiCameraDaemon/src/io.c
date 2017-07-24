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

/* readline */
static pthread_key_t rl_key;
static pthread_once_t rl_once = PTHREAD_ONCE_INIT;

typedef struct {
    int rl_cnt; /* initialize to 0 */
    char *rl_bufptr; /* initialize to rl_buf */
    char rl_buf[MAXLINE];
} rline;

static void rcd_readline_destructor(void *ptr) {
    free(ptr);
}

static void rcd_readline_once(void) {
    if (pthread_key_create(&rl_key, rcd_readline_destructor)) {
        rcd_perror("error in pthread_key_create");
        exit(errno);
    }

    return;
}

static ssize_t rcd_read(rline *tsd, int fd, char *ptr) {
    if (tsd->rl_cnt <= 0) {
again:
        if ((tsd->rl_cnt = read(fd, tsd->rl_buf, MAXLINE)) < 0) {
            if (errno == EINTR)
                goto again;
            return (-1);
        } else if (tsd->rl_cnt == 0)
            return (0);
        tsd->rl_bufptr = tsd->rl_buf;
    }

    tsd->rl_cnt--;
    *ptr = *tsd->rl_bufptr++;
    return (1);
}

ssize_t rcd_readline(int fd, void *vptr, size_t maxlen) {
    size_t n, rc;
    char c, *ptr;
    rline *tsd;

    if (pthread_once(&rl_once, rcd_readline_once)) {
        rcd_perror("error in pthread_once");
        exit(errno);
    }

    if ((tsd = pthread_getspecific(rl_key)) == NULL) {
        if (!(tsd = calloc(1, sizeof (rline)))) {
            rcd_perror("readline calloc()"); /* init to 0 */
            exit(errno);
        }
        if (pthread_setspecific(rl_key, tsd)) {
            rcd_perror("error in pthread_setspecific");
            exit(errno);
        }
    }

    ptr = vptr;
    for (n = 1; n < maxlen; n++) {
        if ((rc = rcd_read(tsd, fd, &c)) == 1) {
            *ptr++ = c;
            if (c == '\n')
                break;
        } else if (rc == 0) {
            *ptr = 0;
            return (n - 1); /* EOF, n - 1 bytes read */
        } else
            return (-1); /* error, errno set by read() */
    }

    *ptr = 0;
    return (n);
}

/* socket recive */
ssize_t rcd_read_n_bytes(int fd, void *vptr, size_t n) {
    size_t nleft;
    ssize_t nread;
    char *ptr;

    ptr = vptr;
    nleft = n;
    while (nleft > 0) {
        if ((nread = read(fd, ptr, nleft)) < 0) {
            if (errno == EINTR)
                nread = 0; /* and call read() again */
            else
                return (-1);
        } else if (nread == 0)
            break; /* EOF */

        nleft -= nread;
        ptr += nread;
    }
    return (n - nleft); /* return >= 0 */
}

/* socket send */
ssize_t rcd_write_n_bytes(int fd, void *vptr, size_t n) {
    size_t nleft;
    ssize_t nwrite;
    char *ptr;

    ptr = vptr;
    nleft = n;
    while (nleft > 0) {
        if ((nwrite = write(fd, ptr, nleft)) < 0) {
            if (errno == EINTR)
                nwrite = 0; /* and call read() again */
            else
                return (-1);
        } else if (nwrite == 0)
            break; /* EOF */

        nleft -= nwrite;
        ptr += nwrite;
    }
    return (n - nleft); /* return >= 0 */
}

ssize_t rcd_writeline(int fd, const char *str) {

    if (str)
        return rcd_write_n_bytes(fd, (void *) str, strlen(str));

    return -1;
}

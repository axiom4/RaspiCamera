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

char * rcdStringToLower(const char *string) {
    char *l_string;
    char *ptr;
    int len;

    if (!string)
        return NULL;
    len = strlen(string) + 1;
    if (!(l_string = malloc(len))) {
        rcd_perror("strtolower malloc");
        exit(errno);
    }

    bzero(l_string, len);
    ptr = l_string;

    while (*string) {
        *ptr = tolower((int) *string++);
        ptr++;
    }

    return l_string;
}

char * rcdStringToUpper(const char *string) {
    char *u_string;
    char *ptr;

    if (!string)
        return NULL;

    if (!(u_string = malloc(strlen(string) + 1))) {
        rcd_perror("strtolower malloc");
        exit(errno);
    }

    ptr = u_string;

    while (*string) {
        *ptr = tolower((int) *string++);
        ptr++;
    }

    ptr = '\0';

    return u_string;
}

int rcdCompareString(const char *s1, const char *s2, long len) {
    for (; (*s1 == *s2) && *s1 && len > 0; s1++, s2++, len--)
        ;
    return (len == 0 ? 0 : *s1 - *s2);
}

char *rcdStringCat(char *string1, char *string2) {
    char *dest_str;

    asprintf(&dest_str, "%s%s", string1, string2);

    return dest_str;
}

char * rcdTrim(char * string) {
    char *ptr = NULL;

    if (!string)
        return NULL;

    ptr = string;

    while (*ptr == ' ' || *ptr == '\t') {
        ptr++;
    }
    string = ptr;

    ptr = string + strlen(string) - 1;

    while (*ptr == ' ' || *ptr == '\t') {
        *ptr = '\0';
        ptr--;
    }

    return string;

}

void rcdChomp(char *str) {
    char *ptr;

    if (!str)
        return;

    ptr = str;

    while (*ptr) {
        if (*ptr == '\n' || *ptr == '\r')
            *ptr = '\0';

        ptr++;
    }

    return;
}

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
#include <parser.h>

char * rcdReadToken(const char *line, struct token *tk) {
    int c = 0;
    char *ptr = (char *) line;

    tk->token = (char *) line;
    tk->nexttoken = NULL;

    if (!ptr)
        return NULL;

    while ((c = (int) *ptr)) {
        if (c == ' ' || c == '\t') {
            *ptr = '\0';
            ptr++;

            while (*ptr == ' ' || *ptr == '\t') {
                *ptr = '\0';
                ptr++;
            }

            if (*ptr == '\r')
                tk->nexttoken = NULL;
            else
                tk->nexttoken = ptr;

            break;
        } else if (c == '\r' || c == '\n') {
            *ptr = '\0';
            tk->nexttoken = NULL;
            break;
        }
        ptr++;
    }

    return tk->token;
}

int rcdParseToken(const char *cmd, const char *token) {
    ssize_t len = 0;
    char *str;

    if (cmd && token) {
        str = rcdStringToLower((char *) cmd);
        len = rcdCompareString(str, token, strlen(str));
        free(str);
    }

    return (!len ? 1 : 0);
}

int rcdParseTokenSection(const char *cmd, const char *token) {
    ssize_t len = 0;
    char *str;

    if (cmd && token) {
        str = rcdStringToLower((char *) cmd);
        len = rcdCompareString(str, token, strlen(token));
        free(str);
    }

    return (!len ? 1 : 0);
}
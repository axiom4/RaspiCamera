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

struct token {
    char *token;
    char *nexttoken;
};

typedef enum {
    UNKNOWN,
    RCD,
    CAMERA
} rcd_section;

void rcdConfigInit(RcdRunConfig *config) {
    config->configfile = NULL;
    config->debug = 0;
    config->daemonize = 0;
    config->app_name = NULL;
    config->rcd_config.log_facility = NULL;
    config->rcd_config.socket_controller_s = NULL;
    config->camera_config.camera_timeout = 0;
    config->camera_list = NULL;
    config->controller_socket = 0;
    config->context = NULL;
}

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
        len = strcmp(str, token);
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

void rcdConfigParse(const char *filename, RcdRunConfig *config) {
    int config_fd;
    struct stat sb;
    char buffer[MAXLINE];

    int ret;

    char *ptr;
    int line = 0;
    rcd_section section = UNKNOWN;

    int parse_error = 0;

    struct token tk;

    if (stat(filename, &sb) == -1) {
        rcd_perror("config file open");
        exit(errno);
    }

    if (sb.st_mode & S_IFMT != S_IFREG) {
        perr("%s: is not regular file", filename);
        exit(EXIT_FAILURE);
    }

    if ((config_fd = open(filename, 0, O_RDONLY)) < 0) {
        rcd_perror("config file open");
        exit(errno);
    }

    bzero(buffer, MAXLINE);

    while ((ret = rcdReadline(config_fd, buffer, MAXLINE)) > 0) {
        rcdChomp(buffer);
        rcdTrim(buffer);
        line++;

        tk.token = rcdReadToken(buffer, &tk);

        if (rcdParseToken(tk.token, "") || tk.token[0] == '#' || tk.token[0] == ';') {
            continue;
        } else if (tk.token[0] == '[' && tk.token[strlen(tk.token) - 1] == ']') {

            ptr = tk.token;

            ptr++;
            ptr[strlen(ptr) - 1] = 0;

            if (rcdParseTokenSection(ptr, "rcd")) {
                section = RCD;
            } else if (rcdParseTokenSection(ptr, "camera")) {
                section = CAMERA;
            } else {
                section = UNKNOWN;
                perr("Invalid config file section: %s", ptr);
                parse_error = 1;
                break;
            }
        } else if (rcdParseToken(tk.token, "facility") && section == RCD) {
            if (!(tk.token = rcdReadToken(tk.nexttoken, &tk))) {
                perr("invalid line: %d", line);
                parse_error = 1;
                break;
            }

            if (rcdParseToken(tk.token, "=")) {

                if (!(tk.token = rcdReadToken(tk.nexttoken, &tk)) && tk.nexttoken) {
                    perr("invalid line: %d", line);
                    parse_error = 1;
                    break;
                }

                ret = strlen(tk.token);

                if (!(config->rcd_config.log_facility = malloc(ret + 1))) {
                    rcd_perror("malloc");
                    parse_error = 1;
                    break;
                }
                bzero(config->rcd_config.log_facility, ret + 1);
                strncpy(config->rcd_config.log_facility, tk.token, ret);
            }
        } else if (rcdParseToken(tk.token, "controller_socket") && section == RCD) {
            if (!(tk.token = rcdReadToken(tk.nexttoken, &tk))) {
                perr("invalid line: %d", line);
                parse_error = 1;
                break;
            }

            if (rcdParseToken(tk.token, "=")) {

                if (!(tk.token = rcdReadToken(tk.nexttoken, &tk)) && tk.nexttoken) {
                    perr("invalid line: %d", line);
                    parse_error = 1;
                    break;
                }

                ret = strlen(tk.token);

                if (!(config->rcd_config.socket_controller_s = malloc(ret + 1))) {
                    rcd_perror("malloc");
                    parse_error = 1;
                    break;
                }
                bzero(config->rcd_config.socket_controller_s, ret + 1);
                strncpy(config->rcd_config.socket_controller_s, tk.token, ret);
            }
        } else if (rcdParseToken(tk.token, "timeout") && section == CAMERA) {
            if (!(tk.token = rcdReadToken(tk.nexttoken, &tk))) {
                perr("invalid line: %d", line);
                parse_error = 1;
                break;
            }

            if (rcdParseToken(tk.token, "=")) {

                if (!(tk.token = rcdReadToken(tk.nexttoken, &tk)) && tk.nexttoken) {
                    perr("invalid line: %d", line);

                    parse_error = 1;

                    break;
                }

                ret = strtol(tk.token, NULL, 10);

                if (ret <= 0 || ret > 65535) {
                    perr("invalid uid_length (line: %d)", line);
                    parse_error = 1;
                    break;
                }

                config->camera_config.camera_timeout = ret;
            }
        } else {
            perr("Error: unknown keyword: %s at line: %d", tk.token, line);
            parse_error = 1;

            break;
        }

        bzero(buffer, MAXLINE);
    }

    if (parse_error) {
        rcdConfigFree(config);
        
        rcd_exit = 1;
    }

    if (config_fd)
        close(config_fd);
}

void rcdConfigFree(RcdRunConfig *config) {
    if (config->rcd_config.log_facility) {
        free(config->rcd_config.log_facility);
        config->rcd_config.log_facility = NULL;
    }
    if (config->rcd_config.socket_controller_s) {
        free(config->rcd_config.socket_controller_s);
        config->rcd_config.socket_controller_s = NULL;
    }

}
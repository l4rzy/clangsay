/*
 * clangsay -  The classic cowsay program, written in C.
 *
 * msg.c
 *
 * Copyright (c) 2015 sasairc
 * This work is free. You can redistribute it and/or modify it under the
 * terms of the Do What The Fuck You Want To Public License, Version 2,
 * as published by Sam Hocevar.HocevarHocevar See the COPYING file or http://www.wtfpl.net/
 * for more details.
 */

#include "./config.h"
#include "./msg.h"
#include "./cow.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>

#ifdef  WITH_SHARED
#include <benly/memory.h>
#include <benly/string.h>
#include <benly/file.h>
#else
#include <libbenly/src/memory.h>
#include <libbenly/src/string.h>
#include <libbenly/src/file.h>
/* WITH_SHARED */
#endif

#ifdef  WITH_REGEX
#include <regex.h>
/* WITH_REGEX */
#endif

/* options */
extern clangsay_opt_t opt;

static int read_msg(MSG** msg, int argc, int optind, char** argv);
static int print_msg(MSG* msg);
static int recursive_msg(MSG** msg, int n);
static int read_recursive_msg(MSG** msg, int fd);
static void release_msg(MSG* msg);
static void strunsecs(MSG** msg);

int init_msg(MSG** msg)
{
    int     status  = 0;

    MSG*    tmp     = NULL;

    if ((tmp = (MSG*)
                malloc(sizeof(MSG))) == NULL) {
        status = -1; goto ERR;
    }
    do {
        tmp->data       = NULL;
        tmp->lines      = 0;
        tmp->read       = read_msg;
        tmp->print      = print_msg;
        tmp->recursive  = recursive_msg;
        tmp->release    = release_msg;
    } while (0);
    *msg = tmp;

    return 0;

ERR:
    switch (status) {
        case    -1:
            fprintf(stderr, "%s: malloc(): %s\n",
                    PROGNAME, strerror(errno));
            break;
    }

    return status;
}

static
int read_msg(MSG** msg, int argc, int optind, char** argv)
{
    int     status  = 0;

    if ((*msg)->data != NULL) {
        free2d((*msg)->data, (*msg)->lines);
        (*msg)->lines = 0;
        (*msg)->data = NULL;
    }
    if (optind < argc) {
        if (((*msg)->data = (char**)
                    smalloc(sizeof(char*) * (argc - optind), NULL)) == NULL) {
            status = -1; goto ERR;
        }
        while (optind < argc) {
            if ((*((*msg)->data + (*msg)->lines) = (char*)
                    smalloc(sizeof(char) * (strlen(*(argv + optind)) + 1), NULL)) == NULL) {
                status = -2; goto ERR;
            }
            memcpy(*((*msg)->data + (*msg)->lines),
                    *(argv + optind), strlen(*(argv + optind)) + 1);
            (*msg)->lines++;
            optind++;
        }
    } else {
        if (((*msg)->lines =
                    load_file_to_array(&(*msg)->data, TH_LINES, TH_LENGTH, stdin)) < 0) {
            status = -3; goto ERR;
        }
    }
    /* remove escape sequence */
    strunsecs(msg);

    return 0;

ERR:
    switch (status) {
        case    -1:
        case    -2:
            break;
        case    -3:
            fprintf(stderr, "%s: read_string(): load_file_to_array(): %s\n",
                    PROGNAME, strerror(errno));
            break;
    }

    return status;
}

static
int print_msg(MSG* msg)
{
    int         i       = 0,
                j       = 0,
                len     = 0,
                maxlen  = 0;

#ifdef  WITH_REGEX
    int     status      = 0;

    char    errstr[128] = {'\0'};

    regex_t reg;

    /* compile regex */
    if ((status = regcomp(&reg, ANSI_ESCSEQ, REG_EXTENDED)) != 0) {
        fprintf(stdout, "error");
        regerror(status, &reg, errstr, sizeof(errstr));
        fprintf(stderr, "%s: regcomp(): %s\n",
                PROGNAME, errstr);

        return -1;
    }
    /* get max length */
    maxlen = strmax_with_regex(msg->lines, msg->data, &reg);
#else
    maxlen = strmax(msg->lines, msg->data);
/* WITH_REGEX */
#endif

    /*
     * first line
     */
    wprintf(L"%lc", opt.border->top_left);

    i = maxlen;
    while (i >= 0) {
        wprintf(L"%lc", opt.border->top);
        i--;
    }
    wprintf(L"%lc%lc", opt.border->top, opt.border->top_right);

    /*
     * multi line
     */
    i = 0;
    while (i < msg->lines) {
#ifdef  WITH_REGEX
        len = mbstrlen_with_regex(*(msg->data + i), &reg);
#else
        len = mbstrlen(*(msg->data + i));
/* WITH_REGEX */
#endif
        /* a normal line */
        wprintf(L"\n%lc %s ", opt.border->left, *(msg->data + i));

        j = maxlen - len;
        while (j > 0) {
            wprintf(L" ");
            j--;
        }
        wprintf(L"%lc", opt.border->right);

        i++;
        /* end a normal line */
    }

    /* last line */
    wprintf(L"\n%lc", opt.border->bottom_left);
    while (maxlen >= 0) {
        wprintf(L"%lc", opt.border->bottom);
        maxlen--;
    }
    wprintf(L"%lc%lc\n", opt.border->bottom, opt.border->bottom_right);

#ifdef  WITH_REGEX
    regfree(&reg);
/* WITH_REGEX */
#endif

    return 0;
}

static
void release_msg(MSG* msg)
{
    int     i   = 0,
            j   = 0;

    if (msg != NULL) {
        if (msg->data != NULL) {
            i = 0;
            j = msg->lines - 1;
            while (i <= j) {
                if (*(msg->data + i) != NULL) {
                    free(*(msg->data + i));
                    *(msg->data + i) = NULL;
                }
                if (*(msg->data + j) != NULL) {
                    free(*(msg->data + j));
                    *(msg->data + j) = NULL;
                }
                i++;
                j--;
            }
            free(msg->data);
            msg->data = NULL;
        }
        free(msg);
        msg = NULL;
    }

    return;
}

static
int recursive_msg(MSG** msg, int n)
{
    int     status  = 0,
            fd[2]   = {0};

    while (n > 1) {
        if (pipe(fd) < 0) {
            status = -1; goto ERR;
        }
        switch (fork()) {
            case    -1:
                status = -2; goto ERR;
            case    0:
                close(fd[0]);
                dup2(fd[1], STDOUT_FILENO);
                close(fd[1]);
                (*msg)->print(*msg);
                (*msg)->release(*msg);
                exit(0);
            default:
                close(fd[1]);
                if (read_recursive_msg(&(*msg), fd[0]) < 0) {
                    status = -3; goto ERR;
                }
                close(fd[0]);
        }
        wait(&status);
        n--;
    }

    return 0;

ERR:
    switch (status) {
        case    -1:
            fprintf(stderr, "%s: pipe(): %s\n",
                    PROGNAME, strerror(errno));
            break;
        case    -2:
            fprintf(stderr, "%s: fork(): %s\n",
                    PROGNAME, strerror(errno));
            break;
        case    -3:
            break;
    }

    return status;
}

static
int read_recursive_msg(MSG** msg, int fd)
{
    int     status  = 0;

    FILE*   fp      = NULL;

    if ((*msg)->data != NULL) {
        free2d((*msg)->data, (*msg)->lines);
        (*msg)->lines = 0;
        (*msg)->data = NULL;
    }
    if ((fp = fdopen(fd, "r")) == NULL) {
        status = -1; goto ERR;
    }
    if (((*msg)->lines =
                load_file_to_array(&(*msg)->data, TH_LINES, TH_LENGTH, fp)) < 0) {
        status = -2; goto ERR;
    }
    fclose(fp);

    return 0;

ERR:
    switch (status) {
        case    -1:
            fprintf(stderr, "%s: read_recursive_msg(): fdopen() %s\n",
                    PROGNAME, strerror(errno));
            break;
        case    -2:
            fprintf(stderr, "%s: read_recursive_msg(): load_file_to_array(): %s\n",
                    PROGNAME, strerror(errno));
            break;
    }
    if (fp != NULL)
        fclose(fp);

    return status;
}

static
void strunsecs(MSG** msg)
{
    int i   = 0,
        j   = (*msg)->lines - 1;

    while (i <= j && i < (*msg)->lines) {
        strunesc(*((*msg)->data + i));
        strunesc(*((*msg)->data + j));
        i++;
        j--;
    }

    return;
}

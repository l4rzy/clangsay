/*
 * clangsay -  The classic cowsay program, written in C.
 *
 * clangsay.c
 *
 * Copyright (c) 2015 sasairc
 * This work is free. You can redistribute it and/or modify it under the
 * terms of the Do What The Fuck You Want To Public License, Version 2,
 * as published by Sam Hocevar.HocevarHocevar See the COPYING file or http://www.wtfpl.net/
 * for more details.
 */

#include "./config.h"
#include "./clangsay.h"
#include "./cow.h"
#include "./msg.h"
#include "./info.h"
#include "./list.h"
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>

#ifdef  WITH_SHARED
#include <benly/string.h>
#else
#include <libbenly/src/string.h>
/* WITH_SHARED */
#endif

/* border sets
 */
const border_t default_border = {
    L' ', L'_', L' ',
    L'<',       L'>',
    L' ', L'-', L' '
};
const border_t unicode_border = {
    L'┌', L'─', L'┐',
    L'│',       L'│',
    L'└', L'─', L'┘'
};
const border_t bold_border    = {
    L'┏', L'━', L'┓',
    L'┃',       L'┃',
    L'┗', L'━', L'┛'
};

const border_t rounded_border = {
    L'╭', L'─', L'╮',
    L'│',       L'│',
    L'╰', L'─', L'╯'
};

/* global config struct 
 */
clangsay_opt_t opt = {
    0, //unicode
    &default_border, //border
    {
        0,
        NULL,
        NULL,
    },
};

static void release(COW* cow, MSG* msg);

int main(int argc, char* argv[])
{
    int     res     = 0,
            index   = 0,
            rarg    = 0,
            status  = 0;

    char*   cowfile = NULL;

    COW*    cow     = NULL;

    MSG*    msg     = NULL;


    /* check for runtime name
     */
    if (!memcmp(argv[0], COMPAT_NAME_SAY, strlen(COMPAT_NAME_SAY))) {
        opt.cow.mode |= MODE_SAY;
    }
    if (!memcmp(argv[0], COMPAT_NAME_THINK, strlen(COMPAT_NAME_THINK))) {
        opt.cow.mode |= MODE_THINK;
    }

    /* option for getopt_long() */
    struct  option opts[] = {
        {"eye",       required_argument, NULL, 'e'},
        {"tongue",    required_argument, NULL, 'T'},
        {"file",      required_argument, NULL, 'f'},
        {"recursive", required_argument, NULL, 'R'},
        {"nowrap",    no_argument,       NULL, 'n'},
        {"borg",      no_argument,       NULL, 'b'},
        {"dead",      no_argument,       NULL, 'd'},
        {"greedy",    no_argument,       NULL, 'g'},
        {"paranoid",  no_argument,       NULL, 'p'},
        {"stoned",    no_argument,       NULL, 's'},
        {"tired",     no_argument,       NULL, 't'},
        {"wired",     no_argument,       NULL, 'w'},
        {"youthful",  no_argument,       NULL, 'y'},
        {"list",      no_argument,       NULL, 'l'},
        {"border",    required_argument, NULL, 'B'},
        {"say",       no_argument,       NULL,  0 },
        {"think",     no_argument,       NULL,  1 },
        {"help",      no_argument,       NULL,  2 },
        {"version",   no_argument,       NULL,  3 },
        {0, 0, 0, 0},
    };

    /* processing of arguments */
    while ((res = getopt_long(argc, argv, "nbdgpstwye:T:f:R:B:l", opts, &index)) != -1) {
        switch (res) {
            case    'e':
                opt.cow.mode |= MODE_M_EYE;
                opt.cow.eye= optarg;
                break;
            case    'T':
                opt.cow.mode |= MODE_M_TONGUE;
                opt.cow.tongue = optarg;
                break;
            case    'f':
                cowfile = optarg;
                break;
            case    'R':
                opt.cow.mode |= MODE_MSG_RECURSIVE;
                if (strisdigit(optarg) < 0) {
                    fprintf(stderr, "%s: %s: invalid argument\n",
                            PROGNAME, optarg);
                    return -1;
                }
                rarg = atoi(optarg);
                break;
            case    'b':
                opt.cow.mode |= MODE_BORG;
                break;
            case    'd':
                opt.cow.mode |= MODE_DEAD;
                break;
            case    'g':
                opt.cow.mode |= MODE_GREEDY;
                break;
            case    'p':
                opt.cow.mode |= MODE_PARANOID;
                break;
            case    's':
                opt.cow.mode |= MODE_STONED;
                break;
            case    't':
                opt.cow.mode |= MODE_TIRED;
                break;
            case    'w':
                opt.cow.mode |= MODE_WIRED;
                break;
            case    'y':
                opt.cow.mode |= MODE_YOUTHFUL;
                break;
            case    'l':
                list_cowfiles();
                break;
            case    'B':
                if (!memcmp(optarg, "default", 7)) {
                    opt.border = &default_border;
                    opt.unicode = 0;
                }
                else if (!memcmp(optarg, "unicode", 7)) {
                    opt.border = &unicode_border;
                    opt.unicode = 1;
                }
                else if (!memcmp(optarg, "bold", 4)) {
                    opt.border = &bold_border;
                    opt.unicode = 1;
                }
                else if (!memcmp(optarg, "rounded", 7)) {
                    opt.border = &rounded_border;
                    opt.unicode = 1;
                }
                else {
                    fprintf(stderr, "%s: %s: invalid argument\n",
                            PROGNAME, optarg);
                    return -1;
                }
                break;
            case    0:
                opt.cow.mode |= MODE_SAY;
                break;
            case    1:
                opt.cow.mode |= MODE_THINK;
                break;
            case    2:
                print_usage();
                break;
            case    3:
                print_version();
                break;
            case    '?':
                return -1;
        }
    }

    /* check env $DEFAULT_COWFILE */
    if (cowfile == NULL) {
        if ((cowfile = getenv("DEFAULT_COWFILE")) == NULL)
            cowfile = DEFAULT_COWFILE;
    }

    /* initialize cow */
    if (init_cow(&cow) < 0) {
        status = 1; goto ERR;
    }

    /* open cow file */
    if (cow->open(&cow, cowfile) < 0) {
        status = 2; goto ERR;
    }

    /* reading cow file to array */
    if (cow->read(&cow) < 0) {
        status = 3; goto ERR;
    }

    /* initialize msg */
    if (init_msg(&msg) < 0) {
        status = 4; goto ERR;
    }

    /* read argv or stdin */
    if (msg->read(&msg, argc, optind, argv) < 0) {
        status = 5; goto ERR;
    }

    /* <<<< recursive message box >>>> */
    if (opt.cow.mode & MODE_MSG_RECURSIVE) {
        if (msg->recursive(&msg, rarg) < 0) {
            status = 6; goto ERR;
        }
    }

    /* print string */
    if (msg->print(msg) < 0) {
        status = 7; goto ERR;
    }

    /* print cow */
    cow->print(cow);

    /* release memory */
    release(cow, msg);

    return 0;

ERR:
    release(cow, msg);

    return status;
}

static
void release(COW* cow, MSG* msg)
{
    if (cow != NULL)
        cow->release(cow);

    if (msg != NULL)
        msg->release(msg);

    return;
}

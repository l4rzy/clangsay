/*
 * clangsay -  The classic cowsay program, written in C.
 *
 * clangsay.h
 *
 * Copyright (c) 2015 sasairc
 * This work is free. You can redistribute it and/or modify it under the
 * terms of the Do What The Fuck You Want To Public License, Version 2,
 * as published by Sam Hocevar.HocevarHocevar See the COPYING file or http://www.wtfpl.net/
 * for more details.
 */

#ifndef CLANGSAY_H
#define CLANGSAY_H
#ifdef  __cplusplus
extern "C" {
/* __cplusplus */
#endif

#include <wchar.h>
#include "cow.h"

#define MODE_MSG_RECURSIVE  (1 << 13)

#define COMPAT_NAME_SAY      "cowsay"
#define COMPAT_NAME_THINK    "cowthink"

typedef struct {
    wchar_t top_left;
    wchar_t top;
    wchar_t top_right;
    wchar_t left;
    wchar_t right;
    wchar_t bottom_left;
    wchar_t bottom;
    wchar_t bottom_right;
} border_t;

typedef struct clangsay_opt_t {
    int               unicode;
    const border_t*   border;
    COWOPT           cow;
} clangsay_opt_t;

/* border sets
 */
extern const border_t default_border;
extern const border_t unicode_border;
extern const border_t bold_border;
extern const border_t rounded_border;

#ifdef  __cplusplus
}
/* __cplusplus */
#endif
/* CLANGSAY_H */
#endif

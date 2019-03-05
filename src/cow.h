/*
 * clangsay -  The classic cowsay program, written in C.
 *
 * cow.h
 *
 * Copyright (c) 2015 sasairc
 * This work is free. You can redistribute it and/or modify it under the
 * terms of the Do What The Fuck You Want To Public License, Version 2,
 * as published by Sam Hocevar.HocevarHocevar See the COPYING file or http://www.wtfpl.net/
 * for more details.
 */

#ifndef COW_H
#define COW_H
#ifdef  __cplusplus
extern "C" {
/* __cplusplus */
#endif

/*
 * # reptarg (haystack)
 * struct reptarg {
 *     char*  haystack;
 * };
 */
#define EYES            "$eyes"
#define THOUGHTS        "$thoughts"
#define TONGUE          "$tongue"

#define DEFAULT_EYES    "oo"
#define DEFAULT_TONGUE  "  "

#define SAY_THOUGHTS    "\\"
#define THINK_THOUGHTS  "o"

/*
 * # reptarg (needle)
 * struct reptarg {
 *     char*  needle;
 * };
 */
#define BORG_EYES       "=="
#define DEAD_EYES       "XX"
#define GREEDY_EYES     "$$"
#define PARANOID_EYES   "@@"
#define STONED_EYES     "**"
#define TIRED_EYES      "--"
#define WIRED_EYES      "OO"
#define YOUTHFUL_EYES   ".."
#define DEAD_TONGUE     "U "

/*
 * # mode
 * struct reptarg {
 *     int    mode;
 * };
 * typedef struct {
 *     int    mode;
 * } COWOPT;
 */
#define MODE_SAY        (1 << 1)
#define MODE_THINK      (1 << 2)
#define MODE_BORG       (1 << 3)
#define MODE_DEAD       (1 << 4)
#define MODE_GREEDY     (1 << 5)
#define MODE_PARANOID   (1 << 6)
#define MODE_STONED     (1 << 7)
#define MODE_TIRED      (1 << 8)
#define MODE_WIRED      (1 << 9)
#define MODE_YOUTHFUL   (1 << 10)
#define MODE_M_EYE      (1 << 11)
#define MODE_M_TONGUE   (1 << 12)

#define COMPAT_NAME_SAY      "cowsay"
#define COMPAT_NAME_THINK    "cowthink"

#include <stdio.h>
#include <wchar.h>

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

typedef struct {
    int               mode;
    const border_t*   border;
    char*             eye;
    char*             tongue;
} COWOPT;

typedef struct COW {
    FILE*   fp;
    char**  data;
    int     lines;
    int     (*open)(struct COW** cow, char* file);
    int     (*read)(struct COW** cow);
    int     (*print)(struct COW* cow, COWOPT* clsay);
    void    (*release)(struct COW* cow);
} COW;

extern int init_cow(COW** cow);

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
/* COW_H */
#endif

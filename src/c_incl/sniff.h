/*
 *      cook - file construction tool
 *      Copyright (C) 1993-1995, 1997-1999, 2001, 2002, 2006, 2007 Peter Miller;
 *      All rights reserved.
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 3 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program. If not, see
 *      <http://www.gnu.org/licenses/>.
 */

#ifndef C_INCL_SNIFF_H
#define C_INCL_SNIFF_H

#include <common/main.h>

enum
{
        absent_ignore,
        absent_mention,
        absent_error
};

typedef struct option_ty option_ty;
struct option_ty
{
        int     o_verbose;
        int     o_absent_local;
        int     o_absent_system;
        int     o_absent_program;
        int     no_src_rel_inc;
        int     absolute;
        int     recursive;
        int     stripdot;
        int     escape_newline;
        int     quote_filenames;
        char    *output;
};
extern option_ty option;

struct input_ty;
struct string_list_ty;

typedef struct sniff_ty sniff_ty;
struct sniff_ty
{
        int (*scan)(struct input_ty *, struct string_list_ty *,
                struct string_list_ty *);
        void (*prepare)(void);
};

void sniff(char *);
void sniff_include(char *);
void sniff_include_cut(void);
long sniff_include_count(void);
void sniff_use_this(char *);
void sniff_use_this_cut(void);
long sniff_use_this_count(void);
void sniff_prepare(void);
void sniff_language(sniff_ty *);

void sniff_remove_leading_path(char *);
void sniff_substitute_leading_path(char *, char *);

int absolute_filename_test(char *);

void sniff_prefix_set(char *);
void sniff_suffix_set(char *);
void sniff_exclude(char *);

#endif /* C_INCL_SNIFF_H */

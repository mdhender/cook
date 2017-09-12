/*
 *      cook - file construction tool
 *      Copyright (C) 199, 1992-1995, 1997, 2001, 2006-2008 Peter Miller
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

#ifndef STR_H
#define STR_H

#include <common/ac/stddef.h>
#include <common/ac/stdarg.h>
#include <common/main.h>
#include <common/format_print.h>

typedef unsigned long str_hash_ty;

typedef struct string_ty string_ty;
struct string_ty
{
        str_hash_ty     str_hash;
        string_ty       *str_next;
        long            str_references;
        size_t          str_length;
        char            str_text[1];
};

extern string_ty *str_true;
extern string_ty *str_false;

void str_initialize(void);
string_ty *str_from_c(const char *);
string_ty *str_n_from_c(const char *, size_t);
string_ty *str_copy(string_ty *);
void str_free(string_ty *);
string_ty *str_catenate(string_ty *, string_ty *);
string_ty *str_cat_three(string_ty *, string_ty *, string_ty *);
int str_bool(string_ty *);
string_ty *str_upcase(string_ty *);
string_ty *str_downcase(string_ty *);
string_ty *str_substitute(string_ty *from, string_ty *to, string_ty *to_me);

#ifdef DEBUG
int str_valid(string_ty *);
#endif

string_ty *str_field(string_ty *s, int sep, int fldnum);
string_ty *str_format(const char *, ...)                    FORMAT_PRINTF(1, 2);
string_ty *str_vformat(const char *, va_list)               FORMAT_VPRINTF(1);

#define str_equal(s1, s2) ((s1) == (s2))

string_ty *str_quote_shell(string_ty *);
string_ty *str_quote_cook(string_ty *, int);

#endif /* STR_H */

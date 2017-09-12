/*
 *      cook - file construction tool
 *      Copyright (C) 1998, 2006-2008 Peter Miller
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

#include <common/str.h>
#include <common/stracc.h>


/*
 * NAME
 *      str_catenate - join two strings
 *
 * SYNOPSIS
 *      string_ty *str_catenate(string_ty *, string_ty *);
 *
 * DESCRIPTION
 *      The str_catenate function is used to concatenate two strings to form a
 *      new string.
 *
 * RETURNS
 *      string_ty * - a pointer to a string in dynamic memory.
 *      Use str_free when finished with.
 *
 * CAVEAT
 *      The contents of the structure pointed to MUST NOT be altered.
 */

string_ty *
str_catenate(string_ty *s1, string_ty *s2)
{
    static stracc   sa;

    sa_open(&sa);
    sa_chars(&sa, s1->str_text, s1->str_length);
    sa_chars(&sa, s2->str_text, s2->str_length);
    return sa_close(&sa);
}

/*
 *      cook - file construction tool
 *      Copyright (C) 1998, 2001, 2006-2008 Peter Miller
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

#include <common/ac/ctype.h>

#include <common/str.h>
#include <common/stracc.h>


/*
 * NAME
 *      str_upcase - upcase a string
 *
 * SYNOPSIS
 *      string_ty *str_upcase(string_ty *);
 *
 * DESCRIPTION
 *      The str_upcase function is used to form a string which is an upper case
 *      form of the supplied string.
 *
 * RETURNS
 *      string_ty * - a pointer to a string in dynamic memory.
 *      Use str_free when finished with.
 *
 * CAVEAT
 *      The contents of the structure pointed to MUST NOT be altered.
 */

string_ty *
str_upcase(string_ty *s)
{
    static stracc   sa;
    char            *cp1;

    sa_open(&sa);
    for (cp1 = s->str_text; *cp1; ++cp1)
    {
        int             c;

        c = (unsigned char)*cp1;
        if (islower(c))
            c = toupper(c);
        sa_char(&sa, c);
    }
    return sa_close(&sa);
}

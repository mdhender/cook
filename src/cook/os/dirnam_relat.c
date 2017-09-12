/*
 *      cook - file construction tool
 *      Copyright (C) 1999, 2001, 2006, 2007 Peter Miller;
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

#include <common/ac/string.h>

#include <cook/os_interface.h>
#include <common/trace.h>


/*
 * NAME
 *      os_dirname_relative - take path apart
 *
 * SYNOPSIS
 *      string_ty *os_dirname_relative(string_ty *path);
 *
 * DESCRIPTION
 *      os_dirname_relative is used to extract the directory part
 *      of a pathname.
 *
 * RETURNS
 *      pointer to dynamically allocated string.
 *      A null pointer is returned on error.
 *
 * CAVEAT
 *      Use str_free() when you are done with the value returned.
 */

string_ty *
os_dirname_relative(string_ty *path)
{
    char            *cp;

    trace(("os_dirname_relative(path = %08lX)\n{\n", path));
    trace_string(path->str_text);
    cp = strrchr(path->str_text, '/');
    if (cp)
    {
        if (cp > path->str_text)
        {
            path = str_n_from_c(path->str_text, cp - path->str_text);
        }
        else
            path = str_from_c("/");
    }
    else
        path = str_from_c(".");
    trace_string(path->str_text);
    trace(("return %08lX;\n", path));
    trace(("}\n"));
    return path;
}

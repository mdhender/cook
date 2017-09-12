/*
 *      cook - file construction tool
 *      Copyright (C) 1999, 2006, 2007 Peter Miller;
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

#include <cook/fingerprint/filename.h>
#include <common/progname.h>
#include <common/str.h>


/*
 * NAME
 *      fp_filename
 *
 * SYNOPSIS
 *      string_ty *fp_filename(void);
 *
 * DESCRIPTION
 *      The fp_filename function is used to determine the name of the
 *      fingerprint cache file in each directory.
 */

string_ty *
fp_filename(void)
{
    static string_ty *s;

    if (!s)
        s = str_format(".%.10s.fp", progname_get());
    return s;
}

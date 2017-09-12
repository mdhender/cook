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

#include <common/input/null.h>
#include <common/input/private.h>
#include <common/str.h>


static void
destruct(input_ty *this)
{
    (void)this;
}


static long
iread(input_ty *this, void *data, long len)
{
    (void)this;
    (void)data;
    (void)len;
    return 0;
}


static int
get(input_ty *this)
{
    (void)this;
    return INPUT_EOF;
}


static string_ty *
filename(input_ty *this)
{
    static string_ty *s;

    (void)this;
    if (!s)
        s = str_from_c("/dev/null");
    return s;
}


static input_vtbl_ty vtbl =
{
    sizeof(input_ty),
    destruct,
    iread,
    get,
    filename,
};


input_ty *
input_null(void)
{
    return input_new(&vtbl);
}

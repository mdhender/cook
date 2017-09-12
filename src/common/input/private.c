/*
 *      cook - file construction tool
 *      Copyright (C) 1999, 2006-2008 Peter Miller
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

#include <common/input/private.h>
#include <common/mem.h>


input_ty *
input_new(input_vtbl_ty *vptr)
{
    input_ty        *result;

    result = mem_alloc(vptr->size);
    result->vptr = vptr;
    result->pushback_buf = 0;
    result->pushback_len = 0;
    result->pushback_max = 0;
    return result;
}


long
input_generic_read(input_ty *fp, void *data_v, long len)
{
    char            *data;
    long            result;
    int             c;

    data = data_v;
    for (result = 0; result < len; ++result)
    {
        c = fp->vptr->get(fp);
        if (c < 0)
            break;
        *data++ = c;
    }
    return result;
}

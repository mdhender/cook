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
#include <common/str.h>


#ifdef input_filename
#undef input_filename
#endif

string_ty *
input_filename(input_ty *fp)
{
    return fp->vptr->filename(fp);
}


#ifdef input_read
#undef input_read
#endif

long
input_read(input_ty *fp, void *data, long len)
{
    if (len <= 0)
        return 0;
    if (fp->pushback_len > 0)
    {
        fp->pushback_len--;
        *(char *)data = fp->pushback_buf[fp->pushback_len];
        return 1;
    }
    return fp->vptr->read(fp, data, len);
}


#ifdef input_getc
#undef input_getc
#endif

int
input_getc(input_ty *fp)
{
    if (fp->pushback_len > 0)
    {
        fp->pushback_len--;
        return fp->pushback_buf[fp->pushback_len];
    }
    return fp->vptr->get(fp);
}


#ifdef input_ungetc
#undef input_ungetc
#endif

void
input_ungetc(input_ty *fp, int c)
{
    if (c < 0)
        return;
    if (fp->pushback_len >= fp->pushback_max)
    {
        fp->pushback_max = 16 + 2 * fp->pushback_max;
        fp->pushback_buf = mem_change_size(fp->pushback_buf, fp->pushback_max);
    }
    fp->pushback_buf[fp->pushback_len++] = c;
}


void
input_delete(input_ty *fp)
{
    if (fp->vptr->destruct)
        fp->vptr->destruct(fp);
    if (fp->pushback_buf)
        mem_free(fp->pushback_buf);
    fp->pushback_buf = 0;
    fp->pushback_len = 0;
    fp->pushback_max = 0;
    fp->vptr = 0; /* paranoia */
    mem_free(fp);
}

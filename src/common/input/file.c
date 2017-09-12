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

#include <common/error_intl.h>
#include <common/input/file.h>
#include <common/input/private.h>
#include <common/input/stdin.h>
#include <common/str.h>
#include <common/sub.h>

typedef struct input_file_ty input_file_ty;
struct input_file_ty
{
    input_ty        inherited;
    FILE            *fp;
    string_ty       *fn;
};


static void
destruct(input_ty *p)
{
    input_file_ty   *this;

    this = (input_file_ty *)p;
    fclose_and_check(this->fp, this->fn->str_text);
    str_free(this->fn);
    this->fp = 0;
    this->fn = 0;
}


static long
iread(input_ty *p, void *data, long len)
{
    input_file_ty   *this;
    long            result;

    if (len < 0)
        return 0;
    this = (input_file_ty *)p;
    result = fread(data, (size_t) 1, (size_t) len, this->fp);
    if (result <= 0 && ferror(this->fp))
        fatal_intl_read(this->fn->str_text);
    return result;
}


static int
get(input_ty *p)
{
    input_file_ty   *this;
    int             c;

    this = (input_file_ty *)p;
    c = getc(this->fp);
    if (c == EOF)
    {
        if (ferror(this->fp))
            fatal_intl_read(this->fn->str_text);
        return INPUT_EOF;
    }
    return c;
}


static string_ty *
filename(input_ty *p)
{
    input_file_ty   *this;

    this = (input_file_ty *)p;
    return this->fn;
}


static input_vtbl_ty vtbl =
{
    sizeof(input_file_ty),
    destruct,
    iread,
    get,
    filename,
};


input_ty *
input_file_open(string_ty *fn)
{
    input_ty        *result;
    input_file_ty   *this;

    if (!fn)
        return input_stdin();
    result = input_new(&vtbl);
    this = (input_file_ty *) result;
    this->fp = fopen_and_check(fn->str_text, "rb");
    this->fn = str_copy(fn);
    return result;
}

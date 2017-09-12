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

#include <common/ac/stdio.h>

#include <common/error_intl.h>
#include <common/input/private.h>
#include <common/input/stdin.h>
#include <common/sub.h>
#include <common/str.h>


static string_ty *
standard_input(void)
{
    static string_ty *name;
    sub_context_ty  *scp;

    if (!name)
    {
        scp = sub_context_new();
        name = subst_intl(scp, i18n("standard input"));
        sub_context_delete(scp);
    }
    return name;
}


static void
destruct(input_ty *this)
{
    (void)this;
}


static long
iread(input_ty *this, void *data, long len)
{
    long            result;

    (void)this;
    if (len <= 0)
        return 0;
    result = fread(data, 1, len, stdin);
    if (result <= 0 && ferror(stdin))
        fatal_intl_read(standard_input()->str_text);
    return result;
}


static int
get(input_ty *this)
{
    int             c;

    (void)this;
    c = getchar();
    if (c == EOF)
    {
        if (ferror(stdin))
            fatal_intl_read(standard_input()->str_text);
        return INPUT_EOF;
    }
    return c;
}


static string_ty *
filename(input_ty *this)
{
    (void)this;
    return standard_input();
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
input_stdin(void)
{
    return input_new(&vtbl);
}

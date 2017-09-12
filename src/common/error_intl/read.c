/*
 *      cook - file construction tool
 *      Copyright (C) 1999, 2002, 2006, 2007 Peter Miller;
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

#include <common/error_intl.h>


void
error_intl_read(const char *fn)
{
    sub_context_ty  *scp;

    scp = sub_context_new();
    sub_errno_set(scp);
    sub_var_set(scp, "File_Name", "%s", fn);
    error_intl(scp, i18n("read $filename: $errno"));
    sub_context_delete(scp);
}


void
fatal_intl_read(const char *fn)
{
    sub_context_ty  *scp;

    scp = sub_context_new();
    sub_errno_set(scp);
    sub_var_set(scp, "File_Name", "%s", fn);
    fatal_intl(scp, i18n("read $filename: $errno"));
    /* NOTREACHED */
}

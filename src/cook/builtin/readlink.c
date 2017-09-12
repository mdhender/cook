/*
 *      cook - file construction tool
 *      Copyright (C) 1996-1999, 2006, 2007 Peter Miller;
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

#include <common/ac/unistd.h>

#include <cook/builtin/readlink.h>
#include <common/error_intl.h>
#include <cook/expr/position.h>
#include <common/str_list.h>
#include <common/trace.h>


/*
 * NAME
 *      builtin_readlink - builtin function for reading symbolic links
 *
 * SYNOPSIS
 *      int builtin_readlink(string_list_ty *result, string_list_ty *args);
 *
 * DESCRIPTION
 *      The builtin_readlink function is used to implement the
 *      "readlink" builtin function of cook to read symbolic links.
 *
 * RETURNS
 *      int; 0 on success, -1 on any error
 *
 * CAVEAT
 *      This function is designed to be used as a "builtin" function.
 */

static int
interpret(string_list_ty *result, const string_list_ty *args,
    const expr_position_ty *pp, const struct opcode_context_ty *ocp)
{
    size_t          j;
    string_ty       *s;
    int             nbytes;
    char            buffer[2000];

    trace(("readlink(result = %08X, args = %08X)\n{\n", result, args));
    (void)ocp;
    for (j = 1; j < args->nstrings; ++j)
    {
        s = args->string[j];
        nbytes = readlink(s->str_text, buffer, sizeof(buffer));
        if (nbytes < 0)
        {
            sub_context_ty  *scp;

            scp = sub_context_new();
            sub_errno_set(scp);
            sub_var_set_string(scp, "File_Name", s);
            error_with_position
                (pp, scp, i18n("readlink \"$filename\": $errno"));
            sub_context_delete(scp);
            return -1;
        }
        s = str_n_from_c(buffer, nbytes);
        string_list_append(result, s);
        str_free(s);
    }
    trace(("return 0;\n"));
    trace(("}\n"));
    return 0;
}


builtin_ty builtin_readlink =
{
    "readlink",
    interpret,
    interpret,                  /* script */
};

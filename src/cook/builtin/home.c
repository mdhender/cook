/*
 *      cook - file construction tool
 *      Copyright (C) 1994, 1997-1999, 2006-2008 Peter Miller
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

#include <common/ac/stdlib.h>
#include <pwd.h>

#include <cook/builtin/home.h>
#include <common/error_intl.h>
#include <cook/expr/position.h>
#include <common/home_directo.h>
#include <common/str_list.h>
#include <common/trace.h>


static int
interpret(string_list_ty *result, const string_list_ty *arg,
    const expr_position_ty *pp, const struct opcode_context_ty *ocp)
{
    string_ty       *s;
    size_t          j;
    struct passwd   *pw;

    trace(("home\n"));
    (void)ocp;
    if (arg->nstrings < 2)
    {
        const char      *cp;

        cp = home_directory();
        assert(cp);
        s = str_from_c(cp);
        string_list_append(result, s);
        str_free(s);
        return 0;
    }
    for (j = 1; j < arg->nstrings; ++j)
    {
        s = arg->string[j];
        pw = getpwnam(s->str_text);
        if (!pw)
        {
            sub_context_ty  *scp;

            scp = sub_context_new();
            sub_var_set_string(scp, "Name", s);
            error_with_position(pp, scp, i18n("user \"$name\" unknown"));
            sub_context_delete(scp);
            return -1;
        }
        s = str_from_c(pw->pw_dir);
        string_list_append(result, s);
        str_free(s);
    }
    return 0;
}


builtin_ty builtin_home =
{
    "home",
    interpret,
    interpret,                  /* script */
};

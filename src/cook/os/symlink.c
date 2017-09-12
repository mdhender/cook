/*
 *      cook - file construction tool
 *      Copyright (C) 2007, 2008 Peter Miller
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

#include <common/ac/errno.h>
#include <common/ac/unistd.h>

#include <common/error_intl.h>
#include <cook/os_interface.h>


int
os_symlink(string_ty *from, string_ty *to, int echo)
{
    int             err;

    err = symlink(from->str_text, to->str_text);
    if (err)
    {
        char link_content[2000];

        if (errno != EEXIST)
        {
            sub_context_ty  *scp;

            whine:
            scp = sub_context_new();
            sub_var_set_string(scp, "File_Name_1", from);
            sub_var_set_string(scp, "File_Name_2", to);
            error_intl(scp, i18n("symlink($filename1, $filename2): $errno"));
            sub_context_delete(scp);
            return -1;
        }
        err = readlink(to->str_text, link_content, sizeof(link_content));
        if (err < 0)
        {
            if (errno != EINVAL)
            {
                sub_context_ty  *scp;

                scp = sub_context_new();
                sub_var_set_string(scp, "File_Name", to);
                error_intl(scp, i18n("readlink \"$filename\": $errno"));
                sub_context_delete(scp);
                return -1;
            }
        }
        else
        {
            string_ty       *s2;
            int             ok;

            s2 = str_n_from_c(link_content, err);
            ok = str_equal(s2, from);
            str_free(s2);
            if (ok)
                return 0;
        }
        err = unlink(to->str_text);
        if (err < 0)
        {
            error_intl_unlink(to->str_text);
            return -1;
        }

        if (echo)
        {
            sub_context_ty  *scp;

            scp = sub_context_new();
            sub_var_set_string(scp, "File_Name", to);
            error_intl(scp, i18n("rm $filename"));
            sub_context_delete(scp);
        }

        err = symlink(from->str_text, to->str_text);
        if (err < 0)
            goto whine;
    }
    if (echo)
    {
        sub_context_ty  *scp;

        scp = sub_context_new();
        sub_var_set_string(scp, "File_Name_1", from);
        sub_var_set_string(scp, "File_Name_2", to);
        error_intl(scp, i18n("ln -s $filename1 $filename2"));
        sub_context_delete(scp);
    }
    return 0;
}

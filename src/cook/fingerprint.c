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

#include <cook/fingerprint.h>
#include <cook/fingerprint/find.h>
#include <cook/fingerprint/record.h>
#include <cook/fingerprint/subdir.h>
#include <common/trace.h>


/*
 * NAME
 *      fp_search
 *
 * SYNOPSIS
 *      fp_value_ty *fp_search(string_ty *path);
 *
 * DESCRIPTION
 *      The fp_search function is used to search for a cached fingerprint.
 *      It returns a pointer to a fingerprint value if this file is known,
 *      or the NULL pointer if not.
 *
 *      If necessary, the cache files will be read in if they have not
 *      been read already.
 */

fp_value_ty *
fp_search(string_ty *path)
{
    fp_record_ty    *p;

    trace(("fp_search(path = \"%s\")\n{\n", path->str_text));
    p = fp_find_record(path);
    if (!p->exists)
    {
        trace(("return NULL;\n"));
        trace(("}\n"));
        return 0;
    }
    trace(("return %08lX;\n", (long)&p->value));
    trace(("}\n"));
    return &p->value;
}


/*
 * NAME
 *      fp_assign
 *
 * SYNOPSIS
 *      void fp_assign(string_ty *path, fp_value_ty *value);
 *
 * DESCRIPTION
 *      The fp_assign function is used to assign a fingerprint value
 *      into the fingerprint cache.
 *
 *      When Cook terminates, the cache will be written out again,
 *      to preserve the fingerprints for the next Cook run.
 */

void
fp_assign(string_ty *path, fp_value_ty *fp)
{
    fp_record_ty    *p;

    trace(("fp_assign(path = \"%s\", fp = %08lX)\n{\n",
            path->str_text, (long)fp));
    p = fp_find_record(path);
    fp_record_update(p, fp);
    trace(("}\n"));
}


/*
 * NAME
 *      fp_delete
 *
 * SYNOPSIS
 *      void fp_delete(string_ty *path);
 *
 * DESCRIPTION
 *      The fp_delete function is used to delete a fingerprint value
 *      from the cache.
 *
 *      When Cook terminates, the cache will be written out again,
 *      to preserve the fingerprints for the next Cook run.
 */

void
fp_delete(string_ty *path)
{
    fp_record_ty    *p;

    trace(("fp_delete(path = \"%s\")\n{\n", path->str_text));
    p = fp_find_record(path);
    fp_record_clear(p);
    trace(("}\n"));
}


/*
 * NAME
 *      fp_tweak
 *
 * SYNOPSIS
 *      void fp_tweak(void);
 *
 * DESCRIPTION
 *      The fp_tweak function is used to walk down the directory tree
 *      below ``.'' fingerprinting all of the files and updating the
 *      fingerprint cache files.
 */

void
fp_tweak(void)
{
    static string_ty *dot;

    if (!dot)
        dot = str_from_c(".");
    fp_subdir_tweak(fp_find_subdir(dot, 1));
}

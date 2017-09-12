/*
 *      cook - file construction tool
 *      Copyright (C) 2001, 2006, 2007 Peter Miller;
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
#include <cook/fingerprint/value.h>
#include <common/trace.h>


int
fp_ingredients_fingerprint_differs(string_ty *filename, string_ty *value)
{
    fp_value_ty     *vp;
    int             different = 0;

    trace(("fp_ingredients_fingerprint_differs(filename=\"%s\", "
        "value=\"%s\")\n{\n", filename->str_text, value->str_text));
    vp = fp_search(filename);
    if (vp)
    {
        if
        (
            vp->ingredients_fingerprint
        &&
            !str_equal(value, vp->ingredients_fingerprint)
        )
        {
            fp_value_ty     data;

            different = 1;
            fp_value_constructor4
            (
                &data,
                vp->oldest,
                vp->newest,
                vp->contents_fingerprint,
                value
            );
            fp_assign(filename, &data);
            fp_value_destructor(&data);
        }
    }
    else
    {
        fp_value_ty     data;
        time_t          now;

        time(&now);
        fp_value_constructor4(&data, now, now, str_from_c(""), value);
        fp_assign(filename, &data);
        fp_value_destructor(&data);
    }
    trace(("return %d;\n", different));
    trace(("}\n"));
    return different;
}

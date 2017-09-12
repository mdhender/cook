/*
 *      cook - file construction tool
 *      Copyright (C) 2007 Peter Miller;
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

#include <common/ac/stdlib.h>

#include <cook/id.h>
#include <cook/id/variable.h>
#include <cook/opcode/context.h>
#include <common/trace.h>
#include <common/ac/time.h>
#include <common/ts.h>

/*
 * NAME
 *      ts_granularity
 *
 * SYNOPSIS
 *      ts_granularity(int *granularity)
 *
 * DESCRIPTION
 *      The ts_granularity function is used to get the filesystem
 *      modtime granularity. For most operating systems, this is 1 second
 *      but can vary.
 *
 *      Defaulting and clean-up are done here, also.
 *      If absent, defaults to 1.
 *
 */

time_t
ts_granularity(void)
{
    string_ty         *key;
    id_ty             *idp;
    string_list_ty    wl;
    int               granularity;
    opcode_context_ty *ocp;

    /*
     * make sure the variable exists
     */
    trace(("ts_granularity()\n{\n"));

    key = str_from_c("timestamp_granularity");
    ocp = opcode_context_new(0, 0);
    idp = opcode_context_id_search(ocp, key);

    granularity = 1;

    if (idp)
    {
        id_variable_query(idp, &wl);
        if (wl.nstrings == 1)
        {
            granularity = atoi(wl.string[0]->str_text);
            if (granularity < 1)
                granularity = 1;
        }
        string_list_destructor(&wl);
    }

    trace(("}\n"));

    return (time_t)granularity;
}

/*
 *      cook - file construction tool
 *      Copyright (C) 2000, 2001, 2006, 2007 Peter Miller;
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

#include <common/ac/string.h>

#include <cook/graph/edge_type.h>


static string_ty *
ends_with(string_ty *s1, const char *s2)
{
    size_t          len2;

    len2 = strlen(s2);
    if
    (
        s1->str_length > len2
    &&
        0 == memcmp(s1->str_text + s1->str_length - len2, s2, len2)
    )
        return str_n_from_c(s1->str_text, s1->str_length - len2);
    return 0;
}


void
edge_type_extract(string_ty *edgename, string_ty **filename_p,
    edge_type_ty *etp)
{
    string_ty       *s;

    s = ends_with(edgename, "(strict)");
    if (s)
    {
        *filename_p = s;
        *etp = edge_type_strict;
        return;
    }

    s = ends_with(edgename, "(weak)");
    if (s)
    {
        *filename_p = s;
        *etp = edge_type_weak;
        return;
    }

    s = ends_with(edgename, "(exists)");
    if (s)
    {
        *filename_p = s;
        *etp = edge_type_exists;
        return;
    }

    *filename_p = str_copy(edgename);
    *etp = edge_type_default;
}


const char *
edge_type_name(edge_type_ty et)
{
    if (et & edge_type_strict)
        return "(strict)";
    if (et & edge_type_weak)
        return "(weak)";
    if (et & edge_type_exists)
        return "(exists)";
    return "(strict)";
}

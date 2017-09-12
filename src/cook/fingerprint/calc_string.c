/*
 *      cook - file construction tool
 *      Copyright (C) 2001, 2005-2009 Peter Miller
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
#include <common/fp/combined.h>
#include <common/str.h>
#include <common/trace.h>


/*
 * NAME
 *      fp_calculate_string
 *
 * SYNOPSIS
 *      string_ty *fp_calculate_string(string_ty *value);
 *
 * DESCRIPTION
 *      The fp_calculate_string function is used to calculate the
 *      fingerprint of the given string.  The cryptographically strong
 *      fingerprint is returned in a string.
 */

string_ty *
fp_fingerprint_string(string_ty *value)
{
    string_ty       *result;
    char            buffer[1000];
    fingerprint_ty  *fp;

    trace(("fp_fingerprint_string(value = %p)\n{\n", value));
    fp = fingerprint_new(&fp_combined);
    fingerprint_addn(fp, (unsigned char *)value->str_text, value->str_length);
    fingerprint_sum(fp, buffer, sizeof(buffer));
    result = str_from_c(buffer);
    fingerprint_delete(fp);
    trace(("return \"%s\";\n", result->str_text));
    trace(("}\n"));
    return result;
}

/*
 *	cook - file construction tool
 *	Copyright (C) 2001, 2005 Peter Miller;
 *	All rights reserved.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 * MANIFEST: functions to calculate checksum of a string
 */

#include <fingerprint.h>
#include <fp/combined.h>
#include <str.h>
#include <trace.h>


/*
 * NAME
 *	fp_calculate_string
 *
 * SYNOPSIS
 *	string_ty *fp_calculate_string(string_ty *value);
 *
 * DESCRIPTION
 *	The fp_calculate_string function is used to calculate the
 *	fingerprint of the given string.  The cryptographically strong
 *	fingerprint is returned in a string.
 */

string_ty *
fp_fingerprint_string(value)
    string_ty       *value;
{
    string_ty       *result;
    char            buffer[1000];
    fingerprint_ty  *fp;

    trace(("fp_fingerprint_string(value = 08lX)\n{\n", (long)value));
    fp = fingerprint_new(&fp_combined);
    fingerprint_addn(fp, (unsigned char *)value->str_text, value->str_length);
    fingerprint_sum(fp, buffer);
    result = str_from_c(buffer);
    fingerprint_delete(fp);
    trace(("return \"%s\";\n", result->str_text));
    trace(("}\n"));
    return result;
}

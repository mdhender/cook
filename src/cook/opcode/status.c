/*
 *      cook - file construction tool
 *      Copyright (C) 1997, 2006-2008 Peter Miller
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

#include <cook/opcode/status.h>


/*
 * NAME
 *      opcode_status_name
 *
 * SYNOPSIS
 *      void opcode_status_name(void);
 *
 * DESCRIPTION
 *      The opcode_status_name function is used to map an opcode status
 *      enumeration value into a string.  Thsi si used for debugging.
 */

char *
opcode_status_name(opcode_status_ty n)
{
        switch (n)
        {
        case opcode_status_success:
                return "success";

        case opcode_status_interrupted:
                return "interrupted";

        case opcode_status_error:
                return "error";

        case opcode_status_wait:
                return "wait";
        }
        return "unknown";
}

/*
 *	cook - file construction tool
 *	Copyright (C) 1997 Peter Miller;
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
 * MANIFEST: functions to manipulate opcode statii
 */

#include <opcode/status.h>


/*
 * NAME
 *	opcode_status_name
 *
 * SYNOPSIS
 *	void opcode_status_name(void);
 *
 * DESCRIPTION
 *	The opcode_status_name function is used to map an opcode status
 *	enumeration value into a string.  Thsi si used for debugging.
 */

char *
opcode_status_name(n)
	opcode_status_ty n;
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

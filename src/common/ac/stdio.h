/*
 *	cook - file construction tool
 *	Copyright (C) 1997, 1999 Peter Miller;
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
 * MANIFEST: insulate against <stdio.h> vagueries
 */

#ifndef COMMON_AC_STDIO_H
#define COMMON_AC_STDIO_H

#include <config.h>

#include <stdio.h>

/*
 * Ancient pre-ANSI-C systems (e.g. SunOS 4.1.2) fail to define this.
 */
#ifndef SEEK_SET
#define SEEK_SET 0
#endif

#ifndef L_tmpnam
#define L_tmpnam 100
#endif

#endif /* COMMON_AC_STDIO_H */

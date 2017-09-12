/*
 *      cook - file construction tool
 *      Copyright (C) 1997, 1999, 2006, 2007 Peter Miller;
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

#ifndef COMMON_AC_STDIO_H
#define COMMON_AC_STDIO_H

#include <common/config.h>

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

#define sprintf sprintf_is_dangerous__use_snprintf_instead@
#define vsprintf sprintf_is_dangerous__use_vsnprintf_instead@

#endif /* COMMON_AC_STDIO_H */

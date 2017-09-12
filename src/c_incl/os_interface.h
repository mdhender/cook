/*
 *      cook - file construction tool
 *      Copyright (C) 1990-1993, 1997, 1999, 2006-2008 Peter Miller
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
 *
 * This file was originally called c_incl/os.h, but <os.h> is a system
 * include file on some systems, and this caused portability problems.
 */

#ifndef C_INCL_OS_INTERFACE_H
#define C_INCL_OS_INTERFACE_H

#include <common/main.h>

int os_exists(char *);

#endif /* C_INCL_OS_INTERFACE_H */

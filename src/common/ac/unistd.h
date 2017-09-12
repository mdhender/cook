/*
 *      cook - file construction tool
 *      Copyright (C) 1994, 1997, 1998, 2006-2008 Peter Miller
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

#ifndef COMMON_AC_UNISTD_H
#define COMMON_AC_UNISTD_H

#include <common/config.h>

/*
 * Need to define _BSD_SOURCE on Linux to get prototypes for the symlink
 * and readlink functions.
 */
#ifdef __linux__
#ifndef _BSD_SOURCE
#define _BSD_SOURCE 1
#endif
#ifndef __USE_BSD
#define __USE_BSD 1
#endif
#endif

#ifdef HAVE_UNISTD_H
#include <sys/types.h>
#include <unistd.h>
#endif

#ifndef SEEK_SET
#define SEEK_SET 0
#endif
#ifndef F_OK
#define F_OK 0
#endif

#endif /* COMMON_AC_UNISTD_H */

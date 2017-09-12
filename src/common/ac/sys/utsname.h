/*
 *      cook - file construction tool
 *      Copyright (C) 1996, 1997, 2006-2008 Peter Miller
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

#ifndef COMMON_AC_SYS_UTSNAME_H
#define COMMON_AC_SYS_UTSNAME_H

#include <common/config.h>

#ifdef HAVE_SYS_UTSNAME_H
#include <sys/utsname.h>
#else

struct utsname
{
        char sysname[10];
        char nodename[50];
        char release[10];
        char version[10];
        char machine[10];
};

#endif

#endif /* COMMON_AC_SYS_UTSNAME_H */

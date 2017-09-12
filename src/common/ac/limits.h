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

#ifndef COMMON_AC_LIMITS_H
#define COMMON_AC_LIMITS_H

#include <common/config.h>

#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif

/*
 * fake a few of the values
 *      (the *_MIN things only work on 2s compliment machines,
 *      the *SHRT* things do not work on non-compliant compilers)
 */
#ifndef USHRT_MAX
#define USHRT_MAX       ((unsigned short)~(unsigned)0)
#endif
#ifndef SHRT_MAX
#define SHRT_MAX        ((short)(USHRT_MAX >> 1))
#endif
#ifndef SHRT_MIN
#define SHRT_MIN        ((short)(~(unsigned short)SHRT_MAX))
#endif
#ifndef UINT_MAX
#define UINT_MAX        (~(unsigned)0)
#endif
#ifndef INT_MAX
#define INT_MAX         ((int)(UINT_MAX >> 1))
#endif
#ifndef INT_MIN
#define INT_MIN         ((int)(~(unsigned)INT_MAX))
#endif
#ifndef ULONG_MAX
#define ULONG_MAX       (~(unsigned long)0)
#endif
#ifndef LONG_MAX
#define LONG_MAX        ((long)(ULONG_MAX >> 1))
#endif
#ifndef LONG_MIN
#define LONG_MIN        ((long)(~(unsigned long)LONG_MAX))
#endif

#ifndef CHAR_BIT
#define CHAR_BIT 8
#endif
#ifndef MB_LEN_MAX
#define MB_LEN_MAX 1
#endif

#ifndef UCHAR_MAX
#define UCHAR_MAX 255
#endif

#endif /* COMMON_AC_LIMITS_H */

/*
 *      cook - file construction tool
 *      Copyright (C) 1994, 1997-1999, 2006-2008 Peter Miller
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

#ifndef COMMON_AC_STDLIB_H
#define COMMON_AC_STDLIB_H

#include <common/config.h>

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#else

#ifndef _WCHAR_T
#ifndef _T_WCHAR_
#ifndef _T_WCHAR
#ifndef __WCHAR_T
#ifndef _WCHAR_T_
#ifndef _WCHAR_T_H
#ifndef ___int_wchar_t_h
#ifndef __INT_WCHAR_T_H
#ifndef _GCC_WCHAR_T
#define _WCHAR_T
#define _T_WCHAR_
#define _T_WCHAR
#define __WCHAR_T
#define _WCHAR_T_
#define _WCHAR_T_H
#define ___int_wchar_t_h
#define __INT_WCHAR_T_H
#define _GCC_WCHAR_T
#ifndef __WCHAR_TYPE__
#define __WCHAR_TYPE__ int
#endif
typedef __WCHAR_TYPE__ wchar_t;
typedef int wchar_t;
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif

#ifndef MB_LEN_MAX
#define MB_LEN_MAX 1
#endif

#ifndef MB_CUR_MAX
#define MB_CUR_MAX 1
#endif

#endif

/*
 * On Linux, the proptotype for exit isn't quite correct.
 *      (__NORETURN is the old gcc way, __NORETURN2 is the new gcc way.)
 */
#ifdef __linux__
#ifdef __NORETURN
#ifdef __NORETURN2
extern __NORETURN void exit __P((int)) __NORETURN2;
#endif
#endif
#endif

#endif /* COMMON_AC_STDLIB_H */

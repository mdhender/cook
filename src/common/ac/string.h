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

#ifndef COMMON_AC_STRING_H
#define COMMON_AC_STRING_H

#include <common/config.h>

/*
 * We could have used __USE_BSD, but that defines prototypes for the
 * index, rindex, bcmp, bzero and bcopy functions, and we don't want
 * them.  This prototype does not conflict, however.
 */
#if !defined(HAVE_STRCASECMP) || defined(__linux__)
# if __STDC__ >= 1
#  ifdef __USE_BSD
#   undef __USE_BSD
#  endif
   int strcasecmp(const char *, const char *);
# endif
#endif

/*
 * We could have used __USE_GNU, but that defines prototypes for
 * too many other things.  This prototype does not conflict, however.
 */
#if !defined(HAVE_STRSIGNAL) || defined(__linux__)
# if __STDC__ >= 1
   char *strsignal(int);
# endif
#endif

const char *safe_strsignal(int);

#if STDC_HEADERS || HAVE_STRING_H
#  include <string.h>
   /* An ANSI string.h and pre-ANSI memory.h might conflict.  */
#  if !STDC_HEADERS && HAVE_MEMORY_H
#    include <memory.h>
#  endif
#else
   /* memory.h and strings.h conflict on some systems.  */
#  include <strings.h>
#endif


/*
 * Cook is built with the -frequire-prototypes flag, but this can cause
 * problems with dumb systems.  Linux is usually pretty good, but...
 *
 * The Linux string.h does not declare these, because then they can
 * be inlined.  It should use the OPTIMIZE define, and only not declare
 * them when not optimizing.
 */
#if defined(linux) || defined(__linux__)
__ptr_t memcpy __P ((__ptr_t, __const __ptr_t, size_t));
int memcmp __P ((__const __ptr_t, __const __ptr_t, size_t));
#endif

#if !HAVE_STRLCPY
size_t strlcpy(char *dst, const char *src, size_t size);
#endif
#if !HAVE_STRLCAT
size_t strlcat(char *dst, const char *src, size_t size);
#endif

#if !HAVE_STRENDCPY
/**
  * The strendcpy function is a buffer-overrun-safe replacement for
  * strcpy, strcat, and a more efficient replacement for strlcpy and
  * strlcat.
  *
  * Unless there is no space left in the buffer (dst >= end), the result
  * will always be NUL terminated.
  *
  * @param dst
  *     The position within the destination string buffer to be copied into.
  * @param src
  *     The string to be copied into the buffer.
  * @param end
  *     The end of the string buffer being copied into.  In most cases
  *     this is of the form "buffer + sizeof(buffer)", a constant which
  *     may be calculated at compile time.
  * @returns
  *     A pointer into the buffer where at the NUL terminator of the
  *     string in the buffer.  EXCEPT when an overrun would occur, in
  *     which case the \a end parameter is returned.
  *
  * @note
  * The return value is where the next string would be written into the
  * buffer.  For example, un-safe code such as
  *
  *     strcat(strcpy(buffer, "Hello, "), "World\n");
  *
  * can be safely replaced by
  *
  *     strendcpy(strendcpy(buffer, "Hello, ", buffer + sizeof(buffer)),
  *         "World\n", buffer + sizeof(buffer));
  *
  * and overruns will be handled safely.  Similarly, more complex string
  * manipulations can be written
  *
  *     char buffer[100];
  *     char *bp = buffer;
  *     bp = strendcpy(bp, "Hello, ", buffer + sizeof(buffer));
  *     bp = strendcpy(bp, "World!\n", buffer + sizeof(buffer));
  *
  * all that is required to test for an overrun is
  *
  *     if (bp == buffer + sizeof(buffer))
  *         fprintf(stderr, "Overrun!\n");
  *
  * On the plus side, there is only one functionto remember, not two,
  * replacing both strcpy and strcat.
  *
  * There have been some quite viable replacements for strcpy and strcat
  * in the BSD strlcpy and strlcat functions.  These functions are
  * indeed buffer-ovrrun-safe but they suffer from doing too much work
  * (and touching too much memory) in the case of overruns.
  *
  * Code such as
  *
  *     strlcpy(buffer, "Hello, ", sizeof(buffer));
  *     strlcat(buffer, "World!\n", sizeof(buffer));
  *
  * suffers from O(n**2) problem, constantly re-tracing the initial
  * portions of the buffer.  In addition, in the case of overruns, the
  * BSD versions of these functions return how big the buffer should
  * have been.  This functionality is rarely used, but still requires
  * the \a src to be traversed all the way to the NUL (and it could be
  * megabytes away) before they can return.  The strendcpy function does
  * not suffer from either of these performance problems.
  */
char *strendcpy(char *dst, const char *src, const char *end);
#endif

#undef strcat
#define strcat strcat_is_unsafe__use_strendcpy_instead@
#undef strcpy
#define strcpy strcpy_is_unsafe__use_strendcpy_instead@

#undef strlcat
#define strlcat strlcat_is_inefficient__use_strendcpy_instead@
#undef strlcpy
#define strlcpy strlcpy_is_inefficient__use_strendcpy_instead@

#endif /* COMMON_AC_STRING_H */

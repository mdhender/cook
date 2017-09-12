/*
 *	cook - file construction tool
 *	Copyright (C) 1994, 1997, 1998 Peter Miller;
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
 * MANIFEST: insulate against <string.h> vs <strings.h> differences
 */

#ifndef COMMON_AC_STRING_H
#define COMMON_AC_STRING_H

#include <config.h>

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

#endif /* COMMON_AC_STRING_H */

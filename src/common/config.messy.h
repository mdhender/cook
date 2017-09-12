/*
 *      cook - file construction tool
 *      Copyright (C) 1999, 2003, 2007 Peter Miller;
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
 *
 * This file is included by the generated ``common/config.h'' file.
 * These actions are performed here, to insulate them from the attentions
 * of ./configure (config.status) which is a little over-zealous about
 * nuking the #undef lines.
 */

#ifndef COMMON_CONFIG_MESSY_H
#define COMMON_CONFIG_MESSY_H

/*
 * Make sure Solaris includes POSIX extensions.
 */
#if (defined(__sun) || defined(__sun__) || defined(sun)) && \
        (defined(__svr4__) || defined(svr4))

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 1
#endif

#ifndef __EXTENSIONS__
#define __EXTENSIONS__ 1
#endif

/*
 * fix a glitch in Solaris's <sys/time.h>
 * which only show's up when you turn __EXTENSIONS__ on
 */
#define _timespec timespec      /* fix 2.4 */
#define _tv_sec tv_sec          /* fix 2.5.1 */

/*
 * The Solaris implementation of wait4() is broken.
 * Don't use it.
 */
#undef HAVE_WAIT4

#endif

/*
 * normalize the wide character support
 */
#if defined(HAVE_WCTYPE_H) && !defined(HAVE_ISWPRINT)
# undef HAVE_WCTYPE_H
#endif
#if !defined(HAVE_WCTYPE_H) && defined(HAVE_ISWPRINT)
# undef HAVE_ISWPRINT
#endif
#if defined(HAVE_WIDEC_H) && !defined(HAVE_WCTYPE_H)
# undef HAVE_WIDEC_H
#endif

/*
 * Need to define these on Linux to get
 * fileno, lstat, etc.
 */
#ifdef __linux__
#define _POSIX_SOURCE
#define _GNU_SOURCE
#define _BSD_SOURCE
#endif

#endif /* COMMON_CONFIG_MESSY_H */

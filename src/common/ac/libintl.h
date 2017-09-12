/*
 *      cook - file construction tool
 *      Copyright (C) 1997, 1999, 2006-2008 Peter Miller
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

#ifndef COMMON_AC_LIBINTL_H
#define COMMON_AC_LIBINTL_H

#include <common/config.h>

/*
 * if the libintl.h include file is available, include it
 */
#ifdef HAVE_LIBINTL_H
#include <libintl.h>
#else
#ifdef HAVE_LIBGETEXT_H
#include <libgettext.h>
#else

/*
 * otherwise, provide a prototype and nothing else
 */
#include <common/main.h>
char *gettext(const char *);
#endif /* !HAVE_LIBGETTEXT_H */
#endif /* !HAVE_LIBINTL_H */

#endif /* COMMON_AC_LIBINTL_H */

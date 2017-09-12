/*
 *	cook - file construction tool
 *	Copyright (C) 1993, 1994, 1995, 1997, 1998 Peter Miller;
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
 * MANIFEST: insulate against <varargs.h> vs <stdarg.h> differences
 */

#ifndef COMMON_AC_STDARG_H
#define COMMON_AC_STDARG_H

/*
 * This file contains insulation from whether <varargs.h> is being used
 * or whether <stdarg.h> is being used.  Incompatibilities are hidden behind
 * three macros:
 *	sva_last	- last argument in variable arg func defn
 *	sva_last_decl	- declaration for last arg
 *	sva_start	- hides whether nth is used or not
 * These macros are non-syntactic (ugh!) but they sure make things prettier.
 *
 * Do not directly include either <stdarg.h> or <varargs.h> in the code,
 * always use this header <ac/stdarg.h> instead.
 *
 * Never refer directly to va_start, va_arg, or va_dcl directly in the code.
 * Direct references to va_list, va_arg and va_end are OK.
 */

#include <config.h>

#ifndef HAVE_STDARG_H

#include <varargs.h>

#define sva_last ,va_alist
#define sva_last_decl va_dcl
#define sva_init(ap, nth) va_start(ap)

#else

#include <stdarg.h>

#define sva_last
#define	sva_last_decl
#define	sva_init(ap, nth) va_start(ap, nth)

#endif

#endif /* COMMON_AC_STDARG_H */

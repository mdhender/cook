/*
 *	cook - file construction tool
 *	Copyright (C) 1997-1999, 2002 Peter Miller;
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
 * MANIFEST: insulate against <wctype.h> presence or absence
 */

#ifndef COMMON_AC_WCTYPE_H
#define COMMON_AC_WCTYPE_H

/*
 * Often needed, if <wctype.h> is implemented in terms of <ctype.h>.
 * Not strictly ANSI C standard conforming.
 */
#include <ac/ctype.h>

/*
 * Often needed, particularly to implement the dummy functions if real
 * ones aren't present.  Not strictly ANSI C standard conforming.
 */
#include <ac/stddef.h>

/* Solaris bug 1250837: include wchar.h before widec.h */
#include <ac/wchar.h>

/*
 * Silicon Graphics
 */
#ifdef HAVE_WIDEC_H
#include <widec.h>
#endif

#ifdef HAVE_WCTYPE_H
#include <wctype.h>
#endif

/*
 * The ANSI C standard states that wint_t symbol shall be defined by
 * <wchar.h> and <wctype.h>.  The GNU people also define it in <stddef.h>,
 * but this is incorrect.
 */
#ifndef HAVE_WINT_T
#define HAVE_WINT_T
typedef wchar_t wint_t;
#endif

#ifndef HAVE_WCTYPE_H
#include <main.h>
int iswalnum _((wint_t));
int iswdigit _((wint_t));
int iswlower _((wint_t));
int iswprint _((wint_t));
int iswpunct _((wint_t));
int iswspace _((wint_t));
int iswupper _((wint_t));
wint_t towlower _((wint_t));
wint_t towupper _((wint_t));
#endif

/*
 * HAVE_ISWPRINT is only set if (a) there is a have_iswprint function,
 * and (b) it works for ascii.  It is assumed that if iswprint is absent
 * or brain-dead, then so are the rest.
 *
 * This code copes with the case where (a) it exists, (b) it is broken,
 * and (c) it is defined in <wctype.h>
 */
#ifndef HAVE_ISWPRINT

#ifdef iswprint
#undef iswprint
#endif

#ifdef iswspace
#undef iswspace
#endif

#ifdef iswpunct
#undef iswpunct
#endif

#ifdef iswupper
#undef iswupper
#endif

#ifdef iswlower
#undef iswlower
#endif

#ifdef iswdigit
#undef iswdigit
#endif

#ifdef iswalnum
#undef iswalnum
#endif

#ifdef towupper
#undef towupper
#endif

#ifdef towlower
#undef towlower
#endif

#endif /* !HAVE_ISWPRINT */

#endif /* COMMON_AC_WCTYPE_H */

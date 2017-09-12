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
 * MANIFEST: insulate against <wchar.h> presence or absence
 */

#ifndef COMMON_AC_WCHAR_H
#define COMMON_AC_WCHAR_H

/*
 * From the ``believe it or not'' category: The Dec Alpha OSF/1
 * <wchar.h> includes <time.h>, but we need to make sure our own
 * <ac/time.h> is invoked first.
 */
#ifdef __alpha___
#include <ac/time.h>
#endif

/*
 * Cygwin's wchar.h has a missing size_t dependency, so include that first.
 */
#include <ac/stddef.h>

#ifdef HAVE_WCHAR_H
#include <wchar.h>
#else

#include <ac/stddef.h>
#include <main.h>
typedef int mbstate_t;
#ifndef WEOF
#define WEOF (wchar_t)(-1);
#endif
int mbsinit _((const mbstate_t *));
size_t wcslen _((const wchar_t *));
size_t mbrlen _((const char *, size_t, mbstate_t *));
size_t mbrtowc _((wchar_t *, const char *, size_t, mbstate_t *));
size_t wcrtomb _((char *, wchar_t, mbstate_t *));
size_t mbsrtowcs _((wchar_t *, const char **, size_t, mbstate_t *));
size_t wcsrtombs _((char *, const wchar_t **, size_t, mbstate_t *));

#endif


/*
 * HAVE_ISWPRINT is only set if (a) there is a have_iswprint function,
 * and (b) it works for ascii.  It is assumed that if iswprint is absent
 * or brain-dead, then so are the rest.
 *
 * This code copes with the case where (a) it exists, (b) it is broken,
 * and (c) it is defined in <wchar.h>, of all places!
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

/*
 * The ANSI C standard states that wint_t symbol shall be defined by
 * <wchar.h> and <wctype.h>.  The GNU people also define it in <stddef.h>,
 * but this is incorrect.
 */
#ifndef HAVE_WINT_T
#define HAVE_WINT_T
typedef wchar_t wint_t;
#endif

#endif /* COMMON_AC_WCHAR_H */

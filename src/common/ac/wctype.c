/*
 *	cook - file construction tool
 *	Copyright (C) 1997, 1998 Peter Miller;
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
 * MANIFEST: impliment missing functions from <wctype.h>
 *
 * The use of the long variable is to cope with the possibility of an
 * unusual data type having been defined for wchar_t.  Unusual types
 * have been observed, and often cause unnecessary warnings from
 * compilers.  The long takes care of it.
 *
 * Also, the HAVE_ISWPRINT can be false (e.g. on Solaris) but iswprint
 * is still available as a macro.  Hence, we need to test for the macro
 * as well as the function.
 */

#include <ac/ctype.h>
#include <ac/wctype.h>
#include <ac/limits.h>


#ifndef HAVE_ISWPRINT

#ifndef iswprint

int
iswprint(wc)
	wint_t		wc;
{
	long		c;

	/*
	 * Assume characters over 256 are printable.  Assume characters
	 * under 256 are either ASCII or Latin-1.  These are dumb
	 * assumptions, but real i18n support will provide a real
	 * iswprint function.
	 */
	c = wc;
	return (c > UCHAR_MAX || (c >= 0 && isprint((unsigned char)c)));
}

#endif

#ifndef iswspace

int
iswspace(wc)
	wint_t		wc;
{
	long		c;

	/*
	 * Assume characters over 256 are letters.  Assume characters
	 * under 256 are either ASCII or Latin-1.  These are dumb
	 * assumptions, but real i18n support will provide a real
	 * iswspace function.
	 */
	c = wc;
	return (c >= 0 && c <= UCHAR_MAX && isspace((unsigned char)c));
}

#endif

#ifndef iswpunct

int
iswpunct(wc)
	wint_t		wc;
{
	long		c;

	/*
	 * Assume characters over 256 are letters.  Assume characters
	 * under 256 are either ASCII or Latin-1.  These are dumb
	 * assumptions, but real i18n support will provide a real
	 * iswpunct function.
	 */
	c = wc;
	return (c >= 0 && c <= UCHAR_MAX && ispunct((unsigned char)c));
}

#endif

#ifndef iswupper

int
iswupper(wc)
	wint_t		wc;
{
	long		c;

	c = wc;
	return (c >= 0 && c <= UCHAR_MAX && isupper((unsigned char)c));
}

#endif

#ifndef iswlower

int
iswlower(wc)
	wint_t		wc;
{
	long		c;

	c = wc;
	return (c >= 0 && c <= UCHAR_MAX && islower((unsigned char)c));
}

#endif

#ifndef iswdigit

int
iswdigit(wc)
	wint_t		wc;
{
	long		c;

	c = wc;
	return (c >= 0 && c <= UCHAR_MAX && isdigit((unsigned char)c));
}

#endif

#ifndef iswalnum

int
iswalnum(wc)
	wint_t		wc;
{
	long		c;

	c = wc;
	return (c >= 0 && c <= UCHAR_MAX && isalnum((unsigned char)c));
}

#endif

#ifndef towupper

wint_t
towupper(wc)
	wint_t		wc;
{
	long		c;

	c = wc;
	if (c >= 0 && c <= UCHAR_MAX && islower((unsigned char)c))
		return toupper((unsigned char)c);
	return c;
}

#endif

#ifndef towlower

wint_t
towlower(wc)
	wint_t		wc;
{
	long		c;

	c = wc;
	if (c >= 0 && c <= UCHAR_MAX && isupper((unsigned char)c))
		return tolower((unsigned char)c);
	return c;
}

#endif

#endif /* !HAVE_ISWPRINT */

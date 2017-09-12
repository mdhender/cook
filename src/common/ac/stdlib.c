/*
 *	cook - file construction tool
 *	Copyright (C) 1997, 1999 Peter Miller;
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
 * MANIFEST: implement missing functions from <stdlib.h>
 */

#include <ac/stdlib.h>
#include <ac/errno.h>


#ifndef HAVE_MBLEN

/* either mblen not available, or it is broken */

#undef mblen

int
mblen(s, n)
	const char	*s;
	size_t		n;
{
	return (s && *s);
}


#undef mbtowc

int
mbtowc(pwc, s, n)
	wchar_t		*pwc;
	const char	*s;
	size_t		n;
{
	if (!s)
		return 0;
	if (pwc)
		*pwc = *(unsigned char *)s;
	return (*s != 0);
}


#undef wctomb

int
wctomb(s, wc)
	char		*s;
	wchar_t		wc;
{
	if (!s)
		return 0;
	*s = wc;
	return 1;
}

#endif /* !HAVE_MBLEN */

#ifndef HAVE_STRTOL

long
strtol(nptr, endptr, base)
	const char	*nptr;
	char		**endptr;
	int		base;
{
	const char	*s;
	int		neg;
	long		n, n2;
	int		ndigits;
	int		c;

	/*
	 * This is not an ANSI C conforming implementation.
	 * Don't use it if you have a choice.
	 */
	neg = 0;
	s = nptr;
	for (;;)
	{
		c = (unsigned char)*s++;
		if (!isspace(c))
			break;
	}
	if (c == '-')
	{
		neg = 1;
		c = (unsigned char)*s++;
	}
	else if (c == '+')
		c = (unsigned char)*s++;
	if ((base == 0 || base == 16) && c == '0' && (*s == 'x' || *s == 'X'))
	{
		++s;
		c = (unsigned char)*s++;
		base = 16;
	}
	if (base == 0)
		base = (c == '0' ? 8 : 10);
	n = 0;
	ndigits = 0;
	for (;;)
	{
		if (isdigit(c))
                        c -= '0';
		else if (isupper(c))
			c -= 'A' - 10;
                else if (islower(c))
                        c += 'a' - 10;
                else
                        break;
                if (c >= base)
                        break;
		n2 = n * base + c;
		if (n2 < n)
		{
			/*
			 * This is a hack.  A real C library will provide
			 * a much better function than this.
			 * E.g. take a look at P.J.Plaugher's book.
			 */
			n = 0;
			errno = ERANGE;
			break;
		}
		n = n2;
		++ndigits;
		c = (unsigned char)*s++;
        }
        if (endptr)
                *endptr = (char *)(ndigits ? s - 1 : nptr);
        return (neg ? -n : n);
}

#endif /* !HAVE_STRTOL */

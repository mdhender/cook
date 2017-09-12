/*
 *	cook - file construction tool
 *	Copyright (C) 1995, 1997, 1999 Peter Miller;
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
 * MANIFEST: functions to manipulate identifier fingerprints
 */

#include <ac/stdio.h>
#include <ac/string.h>

#include <fp/ident.h>
#include <fp/combined.h>
#include <fp/crc32.h>
#include <trace.h>

typedef struct ident_ty ident_ty;
struct ident_ty
{
	FINGERPRINT_BASE_CLASS
	fingerprint_ty	*combined;
	fingerprint_ty	*crc32;
};


static void ident_constructor _((fingerprint_ty *));

static void
ident_constructor(p)
	fingerprint_ty	*p;
{
	ident_ty	*f;

	trace(("constructor\n"));
	f = (ident_ty *)p;
	f->combined = fingerprint_new(&fp_combined);
	f->crc32 = fingerprint_new(&fp_crc32);
}


static void ident_destructor _((fingerprint_ty *));

static void
ident_destructor(p)
	fingerprint_ty	*p;
{
	ident_ty	*f;

	f = (ident_ty *)p;
	fingerprint_delete(f->combined);
	fingerprint_delete(f->crc32);
}


static void ident_addn _((fingerprint_ty *, unsigned char *, int));

static void
ident_addn(p, s, n)
	fingerprint_ty	*p;
	unsigned char	*s;
	int		n;
{
	ident_ty	*f;

	f = (ident_ty *)p;
	fingerprint_addn(f->combined, s, n);
}


static int ident_hash _((fingerprint_ty *, unsigned char *));

static int
ident_hash(p, h)
	fingerprint_ty	*p;
	unsigned char	*h;
{
	ident_ty	*f;
	int		nbytes;
	unsigned char	t[1024];

	f = (ident_ty *)p;
	nbytes = fingerprint_hash(f->combined, t);
	fingerprint_addn(f->crc32, t, nbytes);
	nbytes = fingerprint_hash(f->crc32, h);
	return(nbytes);
}


static void ident_sum _((fingerprint_ty *, char *));

static void
ident_sum(p, s)
	fingerprint_ty	*p;
	char		*s;
{
	unsigned char	h[1024];
	unsigned long	x;
	int		nbytes;
	static char	digits[] = "0123456789";
	char		*cp;

	nbytes = ident_hash(p, h);
	assert(nbytes == 4);

	/*
	 * watch out for 64-bit longs vs 32-bit ints
	 */
	x =
		(
			(unsigned long)h[0]
		|
			((unsigned long)h[1] << 8)
		|
			((unsigned long)h[2] << 16)
		|
			((unsigned long)h[3] << 24)
		);
	sprintf(s, "%8.8lx", x);

	/*
	 * some older stdio implementations don't grok the above format
	 * string, so hunt down and kill and spaces.
	 */
	for (cp = s; *cp; ++cp)
		if (*cp == ' ')
			*cp = '0';

	/*
	 * This forces the first character to be a letter, so
	 * the result is a valid identifier in most computer
	 * languages. The strchr makes it not ASCII specific.
	 */
	cp = strchr(digits, *s);
	if (cp)
		*s = "ghijklmnop"[cp - digits];
}


fingerprint_methods_ty fp_ident =
{
	sizeof(ident_ty),
	"identifier",
	ident_constructor,
	ident_destructor,
	ident_addn,
	ident_hash,
	ident_sum
};

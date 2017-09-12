/*
 *	cook - file construction tool
 *	Copyright (C) 1994, 1997, 1999 Peter Miller;
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
 * MANIFEST: functions to manipulate length fingerprints
 */

#include <ac/stdio.h>

#include <fp/len.h>


typedef struct len_ty len_ty;
struct len_ty
{
	FINGERPRINT_BASE_CLASS
	unsigned long	len;
};


static void len_constructor _((fingerprint_ty *));

static void
len_constructor(p)
	fingerprint_ty	*p;
{
	len_ty		*f;

	f = (len_ty *)p;
	f->len = 0;
}


static void len_destructor _((fingerprint_ty *));

static void
len_destructor(p)
	fingerprint_ty	*p;
{
}


static void len_addn _((fingerprint_ty *, unsigned char *, int));

static void
len_addn(p, s, n)
	fingerprint_ty	*p;
	unsigned char	*s;
	int		n;
{
	len_ty		*f;

	f = (len_ty *)p;
	f->len += n;
}


static int len_hash _((fingerprint_ty *, unsigned char *));

static int
len_hash(p, h)
	fingerprint_ty	*p;
	unsigned char	*h;
{
	len_ty		*f;
	unsigned long	n;
	int		j;

	/*
	 * return length as 5 bytes (yes, five)
	 */
	f = (len_ty *)p;
	n = f->len;
	f->len = 0;
	for (j = 0; j < 5; ++j)
	{
		h[j] = n & 255;
		n >>= 8;
	}
	return 5;
}


static void len_sum _((fingerprint_ty *, char *));

static void
len_sum(p, obuf)
	fingerprint_ty	*p;
	char		*obuf;
{
	len_ty		*f;

	f = (len_ty *)p;
	sprintf(obuf, "%8lu", f->len);
}


fingerprint_methods_ty fp_len =
{
	sizeof(len_ty),
	"length",
	len_constructor,
	len_destructor,
	len_addn,
	len_hash,
	len_sum
};

/*
 *	cook - file construction tool
 *	Copyright (C) 1994, 1997 Peter Miller;
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
 * MANIFEST: functions to manipulate POSIX cksum fingerprints
 */

#include <ac/string.h>

#include <fp/cksum.h>
#include <fp/crc32.h>
#include <fp/len.h>

typedef struct cksum_ty cksum_ty;
struct cksum_ty
{
	FINGERPRINT_BASE_CLASS
	fingerprint_ty	*crc32;
	fingerprint_ty	*len;
};


static void cksum_constructor _((fingerprint_ty *));

static void
cksum_constructor(p)
	fingerprint_ty	*p;
{
	cksum_ty	*f;

	f = (cksum_ty *)p;
	f->crc32 = fingerprint_new(&fp_crc32);
	f->len = fingerprint_new(&fp_len);
}


static void cksum_destructor _((fingerprint_ty *));

static void
cksum_destructor(p)
	fingerprint_ty	*p;
{
	cksum_ty	*f;

	f = (cksum_ty *)p;
	fingerprint_delete(f->crc32);
	fingerprint_delete(f->len);
}


static void cksum_addn _((fingerprint_ty *, unsigned char *, int));

static void
cksum_addn(p, s, n)
	fingerprint_ty	*p;
	unsigned char	*s;
	int		n;
{
	cksum_ty	*f;

	f = (cksum_ty *)p;
	fingerprint_addn(f->crc32, s, n);
	fingerprint_addn(f->len, s, n);
}


static int cksum_hash _((fingerprint_ty *, unsigned char *));

static int
cksum_hash(p, h)
	fingerprint_ty	*p;
	unsigned char	*h;
{
	cksum_ty	*f;
	int		nbytes;
	unsigned char	*obuf;

	f = (cksum_ty *)p;
	obuf = h;
	nbytes = fingerprint_hash(f->crc32, h);
	h += nbytes;
	nbytes = fingerprint_hash(f->len, h);
	h += nbytes;
	return (h - obuf);
}


static void cksum_sum _((fingerprint_ty *, char *));

static void
cksum_sum(p, s)
	fingerprint_ty	*p;
	char		*s;
{
	cksum_ty	*f;

	f = (cksum_ty *)p;
	fingerprint_sum(f->crc32, s);
	s += strlen(s);
	*s++ = ' ';
	fingerprint_sum(f->len, s);
}


fingerprint_methods_ty fp_cksum =
{
	sizeof(cksum_ty),
	"cksum",
	cksum_constructor,
	cksum_destructor,
	cksum_addn,
	cksum_hash,
	cksum_sum
};

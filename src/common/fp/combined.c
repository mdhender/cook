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
 * MANIFEST: functions to manipulate combined fingerprints
 *
 * Derived from code marked:
 *	Daniel J. Bernstein, brnstnd@nyu.edu.
 *	930708: fprintfile 0.95. Public domain.
 *	930708: Added fingerprintfile_addn.
 *	930622: Split off fprintfmt.c.
 *	930601: Baseline, fprintfile 0.8. Public domain.
 *	No known patent problems.
 */

#include <fp/combined.h>
#include <fp/crc32.h>
#include <fp/len.h>
#include <fp/md5.h>
#include <fp/snefru.h>
#include <trace.h>

typedef struct combined_ty combined_ty;
struct combined_ty
{
	FINGERPRINT_BASE_CLASS
	fingerprint_ty	*snefru;
	fingerprint_ty	*md5;
	fingerprint_ty	*crc32;
	fingerprint_ty	*len;
};


static void combined_constructor _((fingerprint_ty *));

static void
combined_constructor(p)
	fingerprint_ty	*p;
{
	combined_ty	*f;

	trace(("constructor\n"));
	f = (combined_ty *)p;
	f->snefru = fingerprint_new(&fp_snefru);
	f->md5 = fingerprint_new(&fp_md5);
	f->crc32 = fingerprint_new(&fp_crc32);
	f->len = fingerprint_new(&fp_len);
}


static void combined_destructor _((fingerprint_ty *));

static void
combined_destructor(p)
	fingerprint_ty	*p;
{
	combined_ty	*f;

	f = (combined_ty *)p;
	fingerprint_delete(f->snefru);
	fingerprint_delete(f->md5);
	fingerprint_delete(f->crc32);
	fingerprint_delete(f->len);
}


static void combined_addn _((fingerprint_ty *, unsigned char *, int));

static void
combined_addn(p, s, n)
	fingerprint_ty	*p;
	unsigned char	*s;
	int		n;
{
	combined_ty	*f;

	f = (combined_ty *)p;
	fingerprint_addn(f->snefru, s, n);
	fingerprint_addn(f->md5, s, n);
	fingerprint_addn(f->crc32, s, n);
	fingerprint_addn(f->len, s, n);
}


static int combined_hash _((fingerprint_ty *, unsigned char *));

static int
combined_hash(p, h)
	fingerprint_ty	*p;
	unsigned char	*h;
{
	combined_ty	*f;
	int		nbytes;
	unsigned char	*obuf;

	f = (combined_ty *)p;
	obuf = h;
	nbytes = fingerprint_hash(f->snefru, h);
	h += nbytes;
	nbytes = fingerprint_hash(f->md5, h);
	h += nbytes;
	nbytes = fingerprint_hash(f->crc32, h);
	h += nbytes;
	nbytes = fingerprint_hash(f->len, h);
	h += nbytes;
	return (h - obuf);
}


static char base64sane[] =
	"0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ:.";


static void combined_sum _((fingerprint_ty *, char *));

static void
combined_sum(p, s)
	fingerprint_ty	*p;
	char		*s;
{
	unsigned char	h[1024];
	int		i;
	unsigned long	x;
	int		nbytes;

	nbytes = combined_hash(p, h);
	assert(nbytes == 57);

	for (i = 0; i < 19; ++i)
	{
		x =
			(
				h[3 * i]
			+
					256L
				*
					(
						h[3 * i + 1]
					+
						256L * h[3 * i + 2]
					)
			);
		s[(12 * i) % 76] = base64sane[x & 63];
		x /= 64;
		s[(12 * i + 41) % 76] = base64sane[x & 63];
		x /= 64;
		s[(12 * i + 6) % 76] = base64sane[x & 63];
		x /= 64;
		s[(12 * i + 47) % 76] = base64sane[x & 63];
	}
	s[76] = 0;
}


fingerprint_methods_ty fp_combined =
{
	sizeof(combined_ty),
	"fingerprint",
	combined_constructor,
	combined_destructor,
	combined_addn,
	combined_hash,
	combined_sum
};

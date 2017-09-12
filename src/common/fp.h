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
 * MANIFEST: interface definition for common/fp.c
 */

#ifndef COMMON_FP_H
#define COMMON_FP_H

#include <main.h>

struct fingerprint_methods_ty; /* forward */

#define FINGERPRINT_BASE_CLASS \
	struct fingerprint_methods_ty *method;

typedef struct fingerprint_ty fingerprint_ty;
struct fingerprint_ty
{
	FINGERPRINT_BASE_CLASS
};

typedef struct fingerprint_methods_ty fingerprint_methods_ty;
struct fingerprint_methods_ty
{
	long		size;
	char		*name;
	void (*constructor)_((fingerprint_ty *));
	void (*destructor)_((fingerprint_ty *));
	void (*addn)_((fingerprint_ty *, unsigned char *, int));
	int (*hash)_((fingerprint_ty *, unsigned char *));
	void (*sum)_((fingerprint_ty *, char *));
};

fingerprint_ty *fingerprint_new _((fingerprint_methods_ty *));
void fingerprint_delete _((fingerprint_ty *));
void fingerprint_add _((fingerprint_ty *, int));
int fingerprint_file_hash _((fingerprint_ty *, char *, unsigned char *));
int fingerprint_file_sum _((fingerprint_ty *, char *, char *));

#define fingerprint_addn(p, s, n) \
	(p)->method->addn((p), (s), (n))
#define fingerprint_hash(p, s) \
	(p)->method->hash((p), (s))
#define fingerprint_sum(p, s) \
	(p)->method->sum((p), (s))

#endif /* COMMON_FP_H */

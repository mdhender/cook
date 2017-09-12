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
 * MANIFEST: interface definition for cook/archive.c
 */

#ifndef COOK_ARCHIVE_H
#define COOK_ARCHIVE_H

struct stat;
struct utimbuf;
struct fingerprint_ty;

#include <str.h>

int archive_stat _((string_ty *, struct stat *));
int archive_utime _((string_ty *, struct utimbuf *));
int archive_fingerprint _((struct fingerprint_ty *fp, string_ty *, char *));

#endif /* COOK_ARCHIVE_H */

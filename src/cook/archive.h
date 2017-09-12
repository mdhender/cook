/*
 *      cook - file construction tool
 *      Copyright (C) 1994, 1997, 2006, 2007 Peter Miller;
 *      All rights reserved.
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 3 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program. If not, see
 *      <http://www.gnu.org/licenses/>.
 */

#ifndef COOK_ARCHIVE_H
#define COOK_ARCHIVE_H

struct stat;
struct utimbuf;
struct fingerprint_ty;

#include <common/str.h>

int archive_stat(string_ty *, struct stat *);
int archive_utime(string_ty *, struct utimbuf *);
int archive_fingerprint(struct fingerprint_ty *fp, string_ty *name,
    char *buf, size_t buf_len);

#endif /* COOK_ARCHIVE_H */

/*
 *      cook - file construction tool
 *      Copyright (C) 1998, 2006-2008 Peter Miller
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

#ifndef COOK_MANIFEST_SNIFF_H
#define COOK_MANIFEST_SNIFF_H

#include <common/main.h>

void sniff_directory(char *);
void sniff_ignore(char *);
int sniff_prefix(char *);
int sniff_suffix(char *);

void sniff(char *, char *);

#endif /* COOK_MANIFEST_SNIFF_H */

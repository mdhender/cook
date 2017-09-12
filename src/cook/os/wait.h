/*
 *      cook - file construction tool
 *      Copyright (C) 2000, 2006, 2007 Peter Miller;
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

#ifndef COOK_OS_WAIT_H
#define COOK_OS_WAIT_H

#include <common/main.h>

struct rusage; /* existence */

int os_wait(int *);
int os_waitpid(int, int *);
int os_wait3(int *, int, struct rusage *);
int os_wait4(int, int *, int, struct rusage *);

#endif /* COOK_OS_WAIT_H */

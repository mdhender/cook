/*
 *      cook - file construction tool
 *      Copyright (C) 1997-1999, 2002, 2006-2008 Peter Miller
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

#ifndef COMMON_ERROR_INTL_H
#define COMMON_ERROR_INTL_H

#include <common/sub.h> /* always needed */
#include <common/noreturn.h>

/*
 * internationalized error messages
 * (also the keywords necessary to pull out the msgid strings)
 */
void error_intl(sub_context_ty *, char *);
void fatal_intl(sub_context_ty *, char *) NORETURN;
void verbose_intl(sub_context_ty *, char *);

/* common comnbinations of functions and errors */
void fatal_intl_open(const char *) NORETURN;
void error_intl_open(const char *);
void fatal_intl_opendir(const char *) NORETURN;
void fatal_intl_close(const char *) NORETURN;
void error_intl_close(const char *);
void fatal_intl_write(const char *) NORETURN;
void error_intl_write(const char *);
void fatal_intl_read(const char *) NORETURN;
void error_intl_read(const char *);
void fatal_intl_stat(const char *) NORETURN;
void error_intl_stat(const char *);
void fatal_intl_unlink(const char *) NORETURN;
void error_intl_unlink(const char *);
void warning_intl_unlink(const char *);

#include <common/ac/stdio.h>
FILE *fopen_and_check(const char *, const char *);
void fclose_and_check(FILE *, const char *);
void fflush_and_check(FILE *, const char *);

#endif /* COMMON_ERROR_INTL_H */

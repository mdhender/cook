/*
 *	cook - file construction tool
 *	Copyright (C) 1997-1999, 2002 Peter Miller;
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
 * MANIFEST: interface definition for common/error_intl.c
 */

#ifndef COMMON_ERROR_INTL_H
#define COMMON_ERROR_INTL_H

#include <sub.h> /* always needed */
#include <noreturn.h>

/*
 * internationalized error messages
 * (also the keywords necessary to pull out the msgid strings)
 */
void error_intl _((sub_context_ty *, char *));
void fatal_intl _((sub_context_ty *, char *)) NORETURN;
void verbose_intl _((sub_context_ty *, char *));

/* common comnbinations of functions and errors */
void fatal_intl_open _((const char *)) NORETURN;
void error_intl_open _((const char *));
void fatal_intl_opendir _((const char *)) NORETURN;
void fatal_intl_close _((const char *)) NORETURN;
void error_intl_close _((const char *));
void fatal_intl_write _((const char *)) NORETURN;
void error_intl_write _((const char *));
void fatal_intl_read _((const char *)) NORETURN;
void error_intl_read _((const char *));
void fatal_intl_stat _((const char *)) NORETURN;
void error_intl_stat _((const char *));
void fatal_intl_unlink _((const char *)) NORETURN;
void error_intl_unlink _((const char *));
void warning_intl_unlink _((const char *));

#include <ac/stdio.h>
FILE *fopen_and_check _((const char *, const char *));
void fclose_and_check _((FILE *, const char *));
void fflush_and_check _((FILE *, const char *));

#endif /* COMMON_ERROR_INTL_H */

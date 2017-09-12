/*
 *	cook - file construction tool
 *	Copyright (C) 1997, 1998 Peter Miller;
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
 * MANIFEST: interface definition for common/sub.c
 */

#ifndef COMMON_SUB_H
#define COMMON_SUB_H

#include <main.h>

typedef struct sub_context_ty sub_context_ty;

sub_context_ty *sub_context_new _((void));
void sub_context_delete _((sub_context_ty *));
void sub_var_clear _((sub_context_ty *));
void sub_var_set _((sub_context_ty *, const char *name, const char *fmt, ...));
void sub_var_optional _((sub_context_ty *, const char *));
void sub_var_append_if_unused _((sub_context_ty *, const char *));
void sub_var_override _((sub_context_ty *, const char *));
void sub_var_resubstitute _((sub_context_ty *, const char *));
void sub_errno_set _((sub_context_ty *));
void sub_errno_setx _((sub_context_ty *, int));
struct string_ty *substitute _((sub_context_ty *, struct string_ty *));
struct string_ty *subst_intl _((sub_context_ty *, const char *));
struct wstring_ty *subst_intl_wide _((sub_context_ty *, const char *));

/*
 * This macro does nothing by itself, but it serves as a keyword for the
 * xgettext program, when extracting internationalized msgid keys.
 */
#define i18n(x) (x)

#endif /* COMMON_SUB_H */

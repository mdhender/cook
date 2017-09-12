/*
 *      cook - file construction tool
 *      Copyright (C) 1997, 1998, 2006, 2007 Peter Miller;
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

#ifndef COMMON_SUB_H
#define COMMON_SUB_H

#include <common/main.h>
#include <common/format_print.h>

struct string_ty; /* forward */
typedef struct sub_context_ty sub_context_ty;

sub_context_ty *sub_context_new(void);
void sub_context_delete(sub_context_ty *);
void sub_var_clear(sub_context_ty *);
void sub_var_set(sub_context_ty *, const char *name, const char *fmt, ...)
    FORMAT_PRINTF(3, 4);
void sub_var_set_charstar(sub_context_ty *, const char *name,
    const char *value);
void sub_var_set_string(sub_context_ty *, const char *name,
    struct string_ty *value);
void sub_var_set_long(sub_context_ty *, const char *name, long value);
void sub_var_optional(sub_context_ty *, const char *);
void sub_var_append_if_unused(sub_context_ty *, const char *);
void sub_var_override(sub_context_ty *, const char *);
void sub_var_resubstitute(sub_context_ty *, const char *);
void sub_errno_set(sub_context_ty *);
void sub_errno_setx(sub_context_ty *, int);
struct string_ty *substitute(sub_context_ty *, struct string_ty *);
struct string_ty *subst_intl(sub_context_ty *, const char *);
struct wstring_ty *subst_intl_wide(sub_context_ty *, const char *);

/*
 * This macro does nothing by itself, but it serves as a keyword for the
 * xgettext program, when extracting internationalized msgid keys.
 */
#define i18n(x) (x)

#endif /* COMMON_SUB_H */

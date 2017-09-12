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
 * MANIFEST: interface definition for make2cook/stmt/rule.c
 */

#ifndef MAKE2COOK_STMT_RULE_H
#define MAKE2COOK_STMT_RULE_H

#include <blob.h>
#include <stmt.h>

extern int stmt_rule_default_history;

stmt_ty *stmt_rule_alloc _((blob_list_ty *lhs, int op, blob_list_ty *rhs,
	blob_list_ty *flags, blob_list_ty *predicate,
	blob_list_ty *single_thread));
void stmt_rule_body _((stmt_ty *, stmt_ty *));
void stmt_rule_context _((stmt_ty *));
stmt_ty *stmt_rule_default _((int));

#endif /* MAKE2COOK_STMT_RULE_H */

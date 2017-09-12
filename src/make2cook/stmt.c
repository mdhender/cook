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
 * MANIFEST: functions to manipulate statements
 */

#include <mem.h>
#include <stmt.h>
#include <trace.h>


stmt_ty *
stmt_alloc(mp)
	stmt_method_ty	*mp;
{
	stmt_ty		*result;

	trace(("stmt_alloc(mp = %08lX)\n{\n"/*}*/, (long)mp));
	result = mem_alloc(mp->size);
	result->method = mp;
	result->white_space = 0;
	string_list_constructor(&result->mdef);
	string_list_constructor(&result->cdef);
	string_list_constructor(&result->ref);
	string_list_constructor(&result->rref);
	if (mp->constructor)
		mp->constructor(result);
	trace(("return %08lX;\n", (long)result));
	trace((/*{*/"}\n"));
	return result;
}


void
stmt_emit(sp)
	stmt_ty		*sp;
{
	trace(("stmt_emit(sp = %08lX)\n{\n"/*}*/, (long)sp));
	if (sp->method->emit)
		sp->method->emit(sp);
	trace((/*{*/"}\n"));
}


void
stmt_free(sp)
	stmt_ty		*sp;
{
	trace(("stmt_free(sp = %08lX)\n{\n"/*}*/, (long)sp));
	if (sp->method->destructor)
		sp->method->destructor(sp);
	string_list_destructor(&sp->mdef);
	string_list_destructor(&sp->cdef);
	string_list_destructor(&sp->ref);
	string_list_destructor(&sp->rref);
	mem_free(sp);
	trace((/*{*/"}\n"));
}


void
stmt_variable_merge(parent, child)
	stmt_ty		*parent;
	stmt_ty		*child;
{
	size_t		j;

	for (j = 0; j < child->mdef.nstrings; ++j)
		string_list_append_unique(&parent->mdef, child->mdef.string[j]);
	for (j = 0; j < child->cdef.nstrings; ++j)
		string_list_append_unique(&parent->cdef, child->cdef.string[j]);
	for (j = 0; j < child->ref.nstrings; ++j)
		string_list_append_unique(&parent->ref, child->ref.string[j]);
	for (j = 0; j < child->rref.nstrings; ++j)
		string_list_append_unique(&parent->rref, child->rref.string[j]);
}


void
stmt_regroup(sp)
	stmt_ty		*sp;
{
	trace(("stmt_regroup(sp = %08lX)\n{\n"/*}*/, (long)sp));
	if (sp->method->regroup)
		sp->method->regroup(sp);
	trace((/*{*/"}\n"));
}


void
stmt_sort(sp)
	stmt_ty		*sp;
{
	trace(("stmt_sort(sp = %08lX)\n{\n"/*}*/, (long)sp));
	if (sp->method->sort)
		sp->method->sort(sp);
	trace((/*{*/"}\n"));
}

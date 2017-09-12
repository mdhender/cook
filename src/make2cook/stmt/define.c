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
 * MANIFEST: functions to manipulate define statements
 */

#include <emit.h>
#include <mem.h>
#include <stmt/define.h>

typedef struct stmt_define_ty stmt_define_ty;
struct stmt_define_ty
{
	STMT
	blob_ty		*first;
	blob_list_ty	*body;
};


static void constructor _((stmt_ty *));

static void
constructor(that)
	stmt_ty		*that;
{
	stmt_define_ty	*this;

	this = (stmt_define_ty *)that;
	this->first = 0;
	this->body = blob_list_alloc();;
}


static void destructor _((stmt_ty *));

static void
destructor(that)
	stmt_ty		*that;
{
	stmt_define_ty	*this;

	this = (stmt_define_ty *)that;
	if (this->first)
		blob_free(this->first);
	blob_list_free(this->body);
}


static void emit _((stmt_ty *));

static void
emit(that)
	stmt_ty		*that;
{
	stmt_define_ty	*this;
	size_t		j;

	this = (stmt_define_ty *)that;
	blob_emit(this->first);
	emit_str(" =\n");

	for (j = 0; j < this->body->length; ++j)
	{
		emit_char('\t');
		blob_emit(this->body->list[j]);
		emit_str(" \"\\n\"\n");
	}
	emit_str("\t;\n");
}


static stmt_method_ty method =
{
	sizeof(stmt_define_ty),
	"define",
	constructor,
	destructor,
	emit,
};


stmt_ty *
stmt_define_alloc(first)
	blob_ty	*first;
{
	stmt_define_ty	*result;

	result = (stmt_define_ty *)stmt_alloc(&method);
	result->first = first;
	return (stmt_ty *)result;
}


void
stmt_define_append(that, lp)
	stmt_ty		*that;
	blob_ty		*lp;
{
	stmt_define_ty	*this;

	this = (stmt_define_ty *)that;
	blob_list_append(this->body, lp);
}

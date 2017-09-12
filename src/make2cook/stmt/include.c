/*
 *	cook - file construction tool
 *	Copyright (C) 1994, 1997, 1998 Peter Miller;
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
 * MANIFEST: functions to manipulate include statements
 */

#include <ac/ctype.h>
#include <ac/string.h>

#include <emit.h>
#include <mem.h>
#include <stmt/include.h>
#include <variable.h>

typedef struct stmt_include_ty stmt_include_ty;
struct stmt_include_ty
{
	STMT
	int		type;
	blob_list_ty	*body;
};


static void destructor _((stmt_ty *));

static void
destructor(that)
	stmt_ty		*that;
{
	stmt_include_ty	*this;

	this = (stmt_include_ty *)that;
	blob_list_free(this->body);
}


static int wildchars _((blob_ty *));

static int
wildchars(s)
	blob_ty		*s;
{
	char		*cp;

	cp = s->text->str_text;
	while (*cp)
	{
		if (strchr("?[*]", *cp))
			return 1;
		++cp;
	}
	return 0;
}


static void emit _((stmt_ty *));

static void
emit(that)
	stmt_ty		*that;
{
	stmt_include_ty	*this;
	long		j;
	int		wild;
	int		wild2;

	this = (stmt_include_ty *)that;
	if (!this->body->length)
		return;
	emit_line_number
	(
		this->body->list[0]->line_number,
		this->body->list[0]->file_name
	);
	wild = 0;
	switch (this->type)
	{
	default:
		emit_str("#include");
		break;

	case 2:
		emit_str("#include-cooked");
		wild = 1;
		break;

	case 3:
		emit_str("#include-cooked-nowarn");
		wild = 1;
		break;
	}
	for (j = 0; j < this->body->length; ++j)
	{
		emit_char(' ');
		wild2 = (wild && wildchars(this->body->list[j]));
		if (wild2)
			emit_str("[wildcard ");
		emit_string(this->body->list[j]->text);
		if (wild2)
			emit_char(']');
	}
	emit_bol();
}


static stmt_method_ty method =
{
	sizeof(stmt_include_ty),
	"include",
	0, /* constructor */
	destructor,
	emit,
	0, /* regroup */
	0, /* sort */
};


stmt_ty *
stmt_include_alloc(body, type)
	blob_list_ty	*body;
	int		type;
{
	stmt_include_ty	*result;
	blob_list_ty	*body2;

	result = (stmt_include_ty *)stmt_alloc(&method);
	body2 = blob_list_alloc();
	variable_rename_list(body, body2, &result->ref, VAREN_QUOTE_SPACES);
	blob_list_free(body);
	result->body = body2;
	result->type = type;
	return (stmt_ty *)result;
}

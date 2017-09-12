/*
 *	cook - file construction tool
 *	Copyright (C) 1999, 2001, 2002 Peter Miller;
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
 * MANIFEST: functions to manipulate values
 */

#include <fingerprint/value.h>
#include <mem.h>
#include <str.h>
#include <trace.h>


/*
 * NAME
 *	fp_value_constructor
 *
 * SYNOPSIS
 *	void fp_value_constructor(fp_value_ty *this);
 *
 * DESCRIPTION
 *	The fp_value_constructor function is used to initialize a
 *	fp_value_ty structure to empty.
 */

void
fp_value_constructor(this)
	fp_value_ty	*this;
{
	trace(("fp_value_constructor(this = %08lX)\n{\n", (long)this));
	this->oldest = 0;
	this->newest = 0;
	this->stat_mod_time = 0;
	this->contents_fingerprint = 0;
	this->ingredients_fingerprint = 0;
	trace(("}\n"));
}


/*
 * NAME
 *	fp_value_constructor_copy
 *
 * SYNOPSIS
 *	void fp_value_constructor_copy(void);
 *
 * DESCRIPTION
 *	The fp_value_constructor_copy function is used to initialize
 *	a fp_value_ty structure with a copy of the value of another
 *	fp_value_ty structure.
 */

void
fp_value_constructor_copy(this, fp)
	fp_value_ty	*this;
	const fp_value_ty *fp;
{
	trace(("fp_value_constructor_copy(this = %08lX, fp = %08lX)\n{\n",
		(long)this, (long)fp));
	this->oldest = fp->oldest;
	this->newest = fp->newest;
	this->stat_mod_time = fp->stat_mod_time;
	this->contents_fingerprint =
		(
			fp->contents_fingerprint
		?
			str_copy(fp->contents_fingerprint)
		:
			0
		);
	this->ingredients_fingerprint =
		(
			fp->ingredients_fingerprint
		?
			str_copy(fp->ingredients_fingerprint)
		:
			0
		);
	trace(("}\n"));
}


/*
 * NAME
 *	fp_value_constructor3
 *
 * SYNOPSIS
 *	void fp_value_constructor3(fp_value_ty *, time_t, time_t, string_ty *);
 *
 * DESCRIPTION
 *	The fp_value_constructor3 function is used to initialize a
 *	fp_value_ty structure with explicit instance variable values.
 */

void
fp_value_constructor3(this, a1, a2, a3)
	fp_value_ty	*this;
	time_t		a1;
	time_t		a2;
	string_ty	*a3;
{
	trace(("fp_value_constructor3(this = %08lX, oldest = %ld, \
youngest = %ld, crypto = \"%s\")\n{\n", (long)this, a1, a2,
		(a3 ? a3->str_text : "")));
	if (a1 > a2)
		a1 = a2;
	this->oldest = a1;
	this->newest = a2;
	this->stat_mod_time = a2;
	this->contents_fingerprint = (a3 ? str_copy(a3) : 0);
	this->ingredients_fingerprint = 0;
	trace(("}\n"));
}


/*
 * NAME
 *	fp_value_constructor4
 *
 * SYNOPSIS
 *	void fp_value_constructor4(fp_value_ty *, time_t, time_t, string_ty *,
 *		string_ty *);
 *
 * DESCRIPTION
 *	The fp_value_constructor4 function is used to initialize a
 *	fp_value_ty structure with explicit instance variable values.
 */

void
fp_value_constructor4(this, a1, a2, a3, a4)
	fp_value_ty	*this;
	time_t		a1;
	time_t		a2;
	string_ty	*a3;
	string_ty	*a4;
{
	trace(("fp_value_constructor4(this = %08lX, oldest = %ld, youngest = \
%ld, cfp = \"%s\", ifp = \"%s\")\n{\n",
		(long)this, a1, a2, (a3 ? a3->str_text : ""),
		(a4 ? a4->str_text : "")));
	if (a1 > a2)
		a1 = a2;
	this->oldest = a1;
	this->newest = a2;
	this->stat_mod_time = a2;
	this->contents_fingerprint = (a3 ? str_copy(a3) : 0);
	this->ingredients_fingerprint = (a4 ? str_copy(a4) : 0);
	trace(("}\n"));
}


/*
 * NAME
 *	fp_value_constructor5
 *
 * SYNOPSIS
 *	void fp_value_constructor5(fp_value_ty *, time_t, time_t, string_ty *,
 *		string_ty *);
 *
 * DESCRIPTION
 *	The fp_value_constructor5 function is used to initialize a
 *	fp_value_ty structure with explicit instance variable values.
 */

void
fp_value_constructor5(this, a1, a2, a3, a4, a5)
	fp_value_ty	*this;
	time_t		a1;
	time_t		a2;
	time_t		a3;
	string_ty	*a4;
	string_ty	*a5;
{
	trace(("fp_value_constructor5(this = %08lX, oldest = %ld, youngest = \
%ld, stat_mod_time = %ld, cfp = \"%s\", ifp = \"%s\")\n{\n",
		(long)this, a1, a2, a3, (a4 ? a4->str_text : ""),
		(a5 ? a5->str_text : "")));
	if (a1 > a2)
		a1 = a2;
	this->oldest = a1;
	this->newest = a2;
	this->stat_mod_time = a3;
	this->contents_fingerprint = (a4 ? str_copy(a4) : 0);
	this->ingredients_fingerprint = (a5 ? str_copy(a5) : 0);
	trace(("}\n"));
}


/*
 * NAME
 *	fp_value_destructor
 *
 * SYNOPSIS
 *	void fp_value_destructor(fp_value_ty *);
 *
 * DESCRIPTION
 *	The fp_value_destructor function is used to release the resources
 *	held by a fp_value_ty structure.
 */

void
fp_value_destructor(this)
	fp_value_ty	*this;
{
	trace(("fp_value_destructor(this = %08lX)\n{\n", (long)this));
	this->oldest = 0;
	this->newest = 0;
	this->stat_mod_time = 0;
	if (this->contents_fingerprint)
		str_free(this->contents_fingerprint);
	this->contents_fingerprint = 0;
	if (this->ingredients_fingerprint)
		str_free(this->ingredients_fingerprint);
	this->ingredients_fingerprint = 0;
	trace(("}\n"));
}


/*
 * NAME
 *	fp_value_ty
 *
 * SYNOPSIS
 *	void fp_value_ty(void);
 *
 * DESCRIPTION
 *	The fp_value_ty function is used to allocate a fp_value_ty
 *	structure on the heap, and initialize it as empty.
 *
 *	Use fo_value_delete when you are done with it.
 */

fp_value_ty *
fp_value_new()
{
	fp_value_ty	*this;

	trace(("fp_value_new()\n{\n"));
	this = mem_alloc(sizeof(fp_value_ty));
	fp_value_constructor(this);
	trace(("return %08lX;\n", (long)this));
	trace(("}\n"));
	return this;
}


/*
 * NAME
 *	fp_value_delete
 *
 * SYNOPSIS
 *	void fp_value_delete(void);
 *
 * DESCRIPTION
 *	The fp_value_delete function is used to release the resources
 *	held by a fp_vaolue_ty structure on the heap.
 */

void
fp_value_delete(this)
	fp_value_ty	*this;
{
	trace(("fp_value_delete(this = %08lX)\n{\n", (long)this));
	fp_value_destructor(this);
	mem_free(this);
	trace(("}\n"));
}


/*
 * NAME
 *	fp_value_copy
 *
 * SYNOPSIS
 *	void fp_value_copy(void);
 *
 * DESCRIPTION
 *	The fp_value_copy function is used to copy the value of one
 *	fp_value_ty structure into another, releasing relaced resourecs
 *	as appropriate.
 */

void
fp_value_copy(to, from)
	fp_value_ty	*to;
	const fp_value_ty *from;
{
	trace(("fp_value_copy(to = %08lX, from = %08lX)\n{\n",
		(long)to, (long)from));
	if (from == to)
	{
		trace(("}\n"));
		return;
	}
	to->stat_mod_time = from->stat_mod_time;
	to->newest = from->newest;
	to->oldest = from->oldest;

	if (to->contents_fingerprint)
		str_free(to->contents_fingerprint);
	if (from->contents_fingerprint)
		to->contents_fingerprint = str_copy(from->contents_fingerprint);
	else
		to->contents_fingerprint = 0;

	if (to->ingredients_fingerprint)
		str_free(to->ingredients_fingerprint);
	if (from->ingredients_fingerprint)
		to->ingredients_fingerprint =
			str_copy(from->ingredients_fingerprint);
	else
		to->ingredients_fingerprint = 0;
	trace(("}\n"));
}


/*
 * NAME
 *	fp_value_write
 *
 * SYNOPSIS
 *	void fp_value_write(fp_value_ty *this, string_ty *key, FILE *fp);
 *
 * DESCRIPTION
 *	The fp_value_write function is used to write a fp_value_ty
 *	structure into an on-disk fingerprint cache file.
 *
 *	It will not be printed if it is empty.
 */

void
fp_value_write(this, key, fp)
	fp_value_ty	*this;
	string_ty	*key;
	FILE		*fp;
{
	if (!this->contents_fingerprint && !this->ingredients_fingerprint)
		return;
	trace(("fp_value_write(this = %08lX, key = \"%s\", fp = %08lX)\n{\n",
		(long)this, key->str_text, (long)fp));
	fprintf(fp, "\"%s\" = { %ld", key->str_text, (long)this->oldest);
	if (this->oldest != this->newest || this->newest != this->stat_mod_time)
	{
	    fprintf(fp, " %ld", (long)this->newest);
	    if (this->newest != this->stat_mod_time)
		fprintf(fp, " %ld", (long)this->stat_mod_time);
	}
	fprintf
	(
		fp,
		"\n\"%s\"",
		(
			this->contents_fingerprint
		?
			this->contents_fingerprint->str_text
		:
			""
		)
	);
	if (this->ingredients_fingerprint)
	{
		fprintf
		(
			fp,
			"\n\"%s\"",
			this->ingredients_fingerprint->str_text
		);
	}
	fprintf(fp, " }\n");
	trace(("}\n"));
}


/*
 * NAME
 *	fp_value_equal
 *
 * SYNOPSIS
 *	void fp_value_equal(void);
 *
 * DESCRIPTION
 *	The fp_value_equal function is used to compare two fp_value_ty
 *	structures for equality.
 *
 *	The ingredients_fingerprint is not compared.  This is because
 *	the principal use of this function is to determine if a file
 *	has changed.
 *
 * RETURNS
 *	int; zero if they are not equal, non-zero if they are equal.
 */

int
fp_value_equal(v1, v2)
	const fp_value_ty *v1;
	const fp_value_ty *v2;
{
	/* DON'T compare the ingredients FP */
	return
	(
		str_equal(v1->contents_fingerprint, v2->contents_fingerprint)
	&&
		v1->newest == v2->newest
	&&
		v1->oldest == v2->oldest
	);
}


/*
 * NAME
 *	fp_value_equal_all
 *
 * SYNOPSIS
 *	void fp_value_equal_all(void);
 *
 * DESCRIPTION
 *	The fp_value_equal_all function is used to compare two fp_value_ty
 *	structures for equality.  The principal use of this function is to
 *	determine if the fingerprint cache has changed and needs writing.
 *
 * RETURNS
 *	int; zero if they are not equal, non-zero if they are equal.
 */

int
fp_value_equal_all(v1, v2)
	const fp_value_ty *v1;
	const fp_value_ty *v2;
{
	return
	(
		str_equal(v1->contents_fingerprint, v2->contents_fingerprint)
	&&
		str_equal(v1->ingredients_fingerprint,
			v2->ingredients_fingerprint)
	&&
		v1->newest == v2->newest
	&&
		v1->oldest == v2->oldest
	&&
		v1->stat_mod_time == v2->stat_mod_time
	);
}

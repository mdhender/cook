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
 * MANIFEST: functions to manipulate blobs
 */

#include <blob.h>
#include <emit.h>
#include <error_intl.h>
#include <mem.h>
#include <trace.h>

static blob_efunc notify;


blob_ty *
blob_alloc(text, file_name, line_number)
	string_ty	*text;
	string_ty	*file_name;
	long		line_number;
{
	blob_ty		*this;

	this = mem_alloc(sizeof(blob_ty));
	this->reference_count = 1;
	this->text = text;
	this->file_name = str_copy(file_name);
	this->line_number = line_number;
	return this;
}


void
blob_free(this)
	blob_ty		*this;
{
	this->reference_count--;
	if (this->reference_count > 0)
		return;
	assert(this->reference_count == 0);
	str_free(this->text);
	str_free(this->file_name);
	mem_free(this);
}


blob_ty *
blob_copy(this)
	blob_ty		*this;
{
	this->reference_count++;
	return this;
}


void
blob_error_notify(func)
	blob_efunc	func;
{
	notify = func;
}


void
blob_error(bp, scp, fmt)
	blob_ty		*bp;
	sub_context_ty	*scp;
	char		*fmt;
{
	string_ty	*buffer;
	int		need_to_delete;

	if (scp)
		need_to_delete = 0;
	else
	{
		scp = sub_context_new();
		need_to_delete = 1;
	}

	buffer = subst_intl(scp, fmt);

	/* re-use substitution context */
	sub_var_set(scp, "File_Name", "%S", bp->file_name);
	sub_var_set(scp, "Number", "%ld", bp->line_number);
	sub_var_set(scp, "MeSsaGe", "%S", buffer);
	error_intl(scp, i18n("$filename: $number: $message"));
	str_free(buffer);
	if (notify)
		notify();

	if (need_to_delete)
		sub_context_delete(scp);
}


void
blob_warning(bp, scp, fmt)
	blob_ty		*bp;
	sub_context_ty	*scp;
	char		*fmt;
{
	string_ty	*buffer;
	int		need_to_delete;

	if (scp)
		need_to_delete = 0;
	else
	{
		scp = sub_context_new();
		need_to_delete = 1;
	}

	buffer = subst_intl(scp, fmt);

	/* re-use substitution context */
	sub_var_set(scp, "File_Name", "%S", bp->file_name);
	sub_var_set(scp, "Number", "%ld", bp->line_number);
	sub_var_set(scp, "MeSsaGe", "%S", buffer);
	error_intl(scp, i18n("$filename: $number: warning: $message"));
	str_free(buffer);

	if (need_to_delete)
		sub_context_delete(scp);
}


blob_list_ty *
blob_list_alloc()
{
	blob_list_ty	*lllp;

	lllp = mem_alloc(sizeof(blob_list_ty));
	lllp->length = 0;
	lllp->maximum = 0;
	lllp->list = 0;
	return lllp;
}


void
blob_list_free(lllp)
	blob_list_ty	*lllp;
{
	long		j;

	for (j = 0; j < lllp->length; ++j)
		blob_free(lllp->list[j]);
	if (lllp->list)
		mem_free(lllp->list);
	mem_free(lllp);
}


void
blob_list_append(lllp, llp)
	blob_list_ty	*lllp;
	blob_ty		*llp;
{
	if (lllp->length >= lllp->maximum)
	{
		size_t		nbytes;

		lllp->maximum = lllp->maximum * 2 + 8;
		nbytes = lllp->maximum * sizeof(blob_ty *);
		lllp->list = mem_change_size(lllp->list, nbytes);
	}
	lllp->list[lllp->length++] = llp;
}


void
blob_list_prepend(lllp, llp)
	blob_list_ty	*lllp;
	blob_ty		*llp;
{
	size_t		j;

	if (lllp->length >= lllp->maximum)
	{
		size_t		nbytes;

		lllp->maximum = lllp->maximum * 2 + 8;
		nbytes = lllp->maximum * sizeof(blob_ty *);
		lllp->list = mem_change_size(lllp->list, nbytes);
	}
	for (j = lllp->length; j > 0; --j)
		lllp->list[j] = lllp->list[j - 1];
	lllp->length++;
	lllp->list[0] = llp;
}


void
blob_emit(bp)
	blob_ty		*bp;
{
	trace(("emit\n"));
	emit_line_number(bp->line_number, bp->file_name);
	emit_string(bp->text);
}


void
blob_list_delete(blp, bp)
	blob_list_ty	*blp;
	blob_ty		*bp;
{
	size_t		j, k;

	for (j = 0; j < blp->length; ++j)
		if (blp->list[j] == bp)
			break;
	if (j >= blp->length)
		return;
	blob_free(blp->list[j]);
	for (k = j + 1; k < blp->length; ++k)
		blp->list[k - 1] = blp->list[k];
	blp->length--;
}

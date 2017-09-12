/*
 *	cook - file construction tool
 *	Copyright (C) 1999 Peter Miller;
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
 * MANIFEST: functions to manipulate privates
 */

#include <match/private.h>
#include <mem.h>


match_ty *
match_private_new(vptr)
	match_method_ty	*vptr;
{
	match_ty	*this;

	this = mem_alloc(vptr->size);
	this->vptr = vptr;
	if (this->vptr->constructor)
		this->vptr->constructor(this);
	return this;
}


match_ty *
match_clone(this)
	match_ty	*this;
{
	return match_private_new(this->vptr);
}

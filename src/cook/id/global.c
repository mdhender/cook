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
 * MANIFEST: functions to manipulate globals
 */

#include <id/private.h>
#include <id/global.h>
#include <symtab.h>


static symtab_ty *stp;


void id_global_reap _((void *));

void
id_global_reap(p)
	void		*p;
{
	id_ty		*idp;

	idp = p;
	id_instance_delete(idp);
}


void
id_global_reset()
{
	if (stp)
		symtab_free(stp);
	stp = 0;
}


symtab_ty *
id_global_stp()
{
	if (!stp)
	{
		stp = symtab_alloc(100);
		stp->reap = id_global_reap;
	}
	return stp;
}

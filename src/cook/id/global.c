/*
 *      cook - file construction tool
 *      Copyright (C) 1999, 2006-2008 Peter Miller
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

#include <cook/id/private.h>
#include <cook/id/global.h>
#include <common/symtab.h>


static symtab_ty *stp;


void
id_global_reap(void *p)
{
    id_ty           *idp;

    idp = p;
    id_instance_delete(idp);
}


void
id_global_reset(void)
{
    if (stp)
        symtab_free(stp);
    stp = 0;
}


symtab_ty *
id_global_stp(void)
{
    if (!stp)
    {
        stp = symtab_alloc(100);
        stp->reap = id_global_reap;
    }
    return stp;
}

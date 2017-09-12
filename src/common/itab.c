/*
 *      cook - file construction tool
 *      Copyright (C) 1997, 2001, 2003, 2006, 2007 Peter Miller;
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

#include <common/itab.h>
#include <common/mem.h>
#include <common/trace.h>


/*
 * NAME
 *      itab_alloc
 *
 * SYNOPSIS
 *      itab_ty *itabLalloc(int);
 *
 * DESCRIPTION
 *      The itab_alloc function is used to allocate a new integer table
 *      instance in dynamic memory.
 *
 * RETURNS
 *      itab_ty *; pointer to table
 *
 * CAVEAT
 *      Use itab_free when you are done with it.
 */

itab_ty *
itab_alloc(int size)
{
    itab_ty         *itp;
    itab_key_ty     j;

    trace(("itab_alloc(size = %d)\n{\n", size));
    itp = mem_alloc(sizeof(itab_ty));
    itp->reap = 0;
    itp->hash_modulus = 1 << 2; /* MUST be a power of 2 */
    while (itp->hash_modulus < size)
        itp->hash_modulus <<= 1;
    itp->hash_mask = itp->hash_modulus - 1;
    itp->load = 0;
    itp->hash_table = mem_alloc(itp->hash_modulus * sizeof(itab_row_ty *));
    for (j = 0; j < itp->hash_modulus; ++j)
        itp->hash_table[j] = 0;
    trace(("return %08lX;\n", (long)itp));
    trace(("}\n"));
    return itp;
}


/*
 * NAME
 *      itab_free
 *
 * SYNOPSIS
 *      void itab_free(itab_ty *);
 *
 * DESCRIPTION
 *      The itab_free function is used to release the resources held by
 *      an integer table.
 */

void
itab_free(itab_ty *itp)
{
    itab_key_ty     j;

    trace(("itab_free(itp = %08lX)\n{\n", (long)itp));
    for (j = 0; j < itp->hash_modulus; ++j)
    {
        itab_row_ty     **rpp;

        rpp = &itp->hash_table[j];
        while (*rpp)
        {
            itab_row_ty     *rp;

            rp = *rpp;
            *rpp = rp->overflow;
            if (itp->reap)
                itp->reap(rp->data);
            mem_free(rp);
        }
    }
    mem_free(itp->hash_table);
    mem_free(itp);
    trace(("}\n"));
}


/*
 * NAME
 *      split - reduce symbol table load
 *
 * SYNOPSIS
 *      void split(itab_ty);
 *
 * DESCRIPTION
 *      The split function is used to split symbols in the bucket indicated by
 *      the split point.  The symbols are split between that bucket and the one
 *      after the current end of the table.
 *
 * CAVEAT
 *      It is only sensable to do this when the symbol table load exceeds some
 *      reasonable threshold.  A threshold of 80% is suggested.
 */

static void
split(itab_ty *itp)
{
    itab_key_ty     new_hash_modulus;
    itab_key_ty     new_hash_mask;
    itab_row_ty     **new_hash_table;
    itab_key_ty     j;

    /*
     * double the modulus
     *
     * This is subtle.  If we only increase the modulus by one, the
     * load always hovers around 80%, so we have to do a split for
     * every insert.  I.e. the malloc burden is O(n) for the lifetime
     * of the symbol table.  BUT if we double the modulus, the length of
     * time until the next split also doubles, making the probablity of
     * a split halve, and sigma(2**-n)=1, so the malloc burden becomes
     * O(1) for the lifetime of the symbol table.
     */
    new_hash_modulus = itp->hash_modulus << 1;
    new_hash_mask = new_hash_modulus - 1;
    new_hash_table = mem_alloc(new_hash_modulus * sizeof(itab_row_ty *));

    /*
     * now redistribute the list elements
     *
     * It is important to preserve the order of the links because
     * they can be push-down stacks, and to simply add them to the
     * head of the list will reverse the order of the stack!
     */
    for (j = 0; j < itp->hash_modulus; ++j)
    {
        itab_row_ty     *p;

        new_hash_table[j] = 0;
        new_hash_table[j + itp->hash_modulus] = 0;
        p = itp->hash_table[j];
        while (p)
        {
            itab_row_ty     *p2;
            itab_key_ty     idx;
            itab_row_ty     **ipp;

            p2 = p;
            p = p2->overflow;
            p2->overflow = 0;

            idx = p2->key & new_hash_mask;
            for (ipp = &new_hash_table[idx]; *ipp; ipp = &(*ipp)->overflow)
                ;
            *ipp = p2;
        }
    }

    /*
     * Switch over to the new table and parameters.
     */
    itp->hash_modulus = new_hash_modulus;
    itp->hash_mask = new_hash_mask;
    mem_free(itp->hash_table);
    itp->hash_table = new_hash_table;

    trace(("}\n"));
}


/*
 * NAME
 *      itab_query - search for a variable
 *
 * SYNOPSIS
 *      int itab_query(itab_ty *, string_ty *key);
 *
 * DESCRIPTION
 *      The itab_query function is used to reference a variable.
 *
 * RETURNS
 *      If the variable has been defined, the function returns a non-zero value
 *      and the value is returned through the 'value' pointer.
 *      If the variable has not been defined, it returns zero,
 *      and 'value' is unaltered.
 */

void *
itab_query(itab_ty *itp, itab_key_ty key)
{
    itab_key_ty     idx;
    itab_row_ty     *p;
    void            *result;

    trace(("itab_query(itp = %08lX, key = %ld)\n{\n", (long)itp, (long)key));
    result = 0;
    idx = key & itp->hash_mask;
    for (p = itp->hash_table[idx]; p; p = p->overflow)
    {
        if (key == p->key)
        {
            result = p->data;
            break;
        }
    }
    trace(("return %08lX;\n", (long)result));
    trace(("}\n"));
    return result;
}


/*
 * NAME
 *      itab_assign - assign a variable
 *
 * SYNOPSIS
 *      void itab_assign(itab_ty *, string_ty *key, void *data);
 *
 * DESCRIPTION
 *      The itab_assign function is used to assign
 *      a value to a given variable.
 *
 * CAVEAT
 *      The name is copied, the data is not.
 */

void
itab_assign(itab_ty *itp, itab_key_ty key, void *data)
{
    itab_key_ty     idx;
    itab_row_ty     *p;

    trace(("itab_assign(itp = %08lX, key = %ld, data = %08lX)\n{\n",
        (long)itp, (long)key, (long)data));
    idx = key & itp->hash_mask;

    for (p = itp->hash_table[idx]; p; p = p->overflow)
    {
        if (key == p->key)
        {
            trace(("modify existing entry\n"));
            if (itp->reap)
                itp->reap(p->data);
            p->data = data;
            goto done;
        }
    }

    trace(("new entry\n"));
    p = mem_alloc(sizeof(itab_row_ty));
    p->key = key;
    p->overflow = itp->hash_table[idx];
    p->data = data;
    itp->hash_table[idx] = p;

    itp->load++;
    if (itp->load * 10 >= itp->hash_modulus * 8)
        split(itp);
    done:
    trace(("}\n"));
}


/*
 * NAME
 *      itab_delete - delete a variable
 *
 * SYNOPSIS
 *      void itab_delete(string_ty *name, itab_class_ty class);
 *
 * DESCRIPTION
 *      The itab_delete function is used to delete variables.
 *
 * CAVEAT
 *      The name is freed, the data is reaped.
 *      (By default, reap does nothing.)
 */

void
itab_delete(itab_ty *itp, itab_key_ty key)
{
    itab_key_ty     idx;
    itab_row_ty     **pp;

    trace(("itab_delete(itp = %08lX, key = %ld)\n{\n", (long)itp, (long)key));
    idx = key & itp->hash_mask;

    pp = &itp->hash_table[idx];
    for (;;)
    {
        itab_row_ty     *p;

        p = *pp;
        if (!p)
            break;
        if (key == p->key)
        {
            if (itp->reap)
                itp->reap(p->data);
            *pp = p->overflow;
            mem_free(p);
            itp->load--;
            break;
        }
        pp = &p->overflow;
    }
    trace(("}\n"));
}


/*
 * NAME
 *      itab_walk
 *
 * SYNOPSIS
 *      void itab_walk(itab_ty *, void (*)(itab_ty *, itab_key_ty, void *,
 *              void *), void *);
 *
 * DESCRIPTION
 *      The itab_walk function is used to visit each element of an
 *      integer table, in no particular order.
 */

void
itab_walk(itab_ty *itp, void (*func)(itab_ty *, itab_key_ty, void *, void *),
    void *arg)
{
    itab_key_ty     j;
    itab_row_ty     *rp;

    trace(("itab_walk(itp = %08lX)\n{\n", (long)itp));
    for (j = 0; j < itp->hash_modulus; ++j)
        for (rp = itp->hash_table[j]; rp; rp = rp->overflow)
            func(itp, rp->key, rp->data, arg);
    trace(("}\n"));
}

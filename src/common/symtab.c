/*
 *      cook - file construction tool
 *      Copyright (C) 1990-1994, 1997, 2001, 2003, 2006-2009 Peter Miller
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

#include <common/error.h> /* for debugging */
#include <common/fstrcmp.h>
#include <common/mem.h>
#include <common/symtab.h>
#include <common/trace.h>


symtab_ty *
symtab_alloc(unsigned size)
{
    symtab_ty       *stp;
    str_hash_ty     j;

    trace(("symtab_alloc(size = %d)\n{\n", size));
    stp = mem_alloc(sizeof(symtab_ty));
    stp->reap = 0;
    stp->hash_modulus = 1 << 2; /* MUST be a power of 2 */
    while (stp->hash_modulus < size)
        stp->hash_modulus <<= 1;
    stp->hash_mask = stp->hash_modulus - 1;
    stp->hash_load = 0;
    stp->hash_table =
        mem_alloc(stp->hash_modulus * sizeof(symtab_row_ty *));
    for (j = 0; j < stp->hash_modulus; ++j)
        stp->hash_table[j] = 0;
    trace(("return %p;\n", stp));
    trace(("}\n"));
    return stp;
}


void
symtab_free(symtab_ty *stp)
{
    str_hash_ty     j;

    trace(("symtab_free(stp = %p)\n{\n", stp));
    for (j = 0; j < stp->hash_modulus; ++j)
    {
        symtab_row_ty   **rpp;

        rpp = &stp->hash_table[j];
        while (*rpp)
        {
            symtab_row_ty   *rp;

            rp = *rpp;
            *rpp = rp->overflow;
            if (stp->reap)
                stp->reap(rp->data);
            str_free(rp->key);
            mem_free(rp);
        }
    }
    mem_free(stp->hash_table);
    mem_free(stp);
    trace(("}\n"));
}


/*
 * NAME
 *      split - reduce symbol table load
 *
 * SYNOPSIS
 *      void split(symtab_ty);
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
split(symtab_ty *stp)
{
    str_hash_ty     new_hash_modulus;
    str_hash_ty     new_hash_mask;
    symtab_row_ty   **new_hash_table;
    str_hash_ty     j;

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
    new_hash_modulus = stp->hash_modulus << 1;
    new_hash_mask = new_hash_modulus - 1;
    new_hash_table = mem_alloc(new_hash_modulus * sizeof(symtab_row_ty *));

    /*
     * now redistribute the list elements
     *
     * It is important to preserve the order of the links because
     * they can be push-down stacks, and to simply add them to the
     * head of the list will reverse the order of the stack!
     */
    for (j = 0; j < stp->hash_modulus; ++j)
    {
        symtab_row_ty   *p;

        new_hash_table[j] = 0;
        new_hash_table[stp->hash_modulus + j] = 0;
        p = stp->hash_table[j];
        while (p)
        {
            symtab_row_ty   *p2;
            str_hash_ty     idx;
            symtab_row_ty   **ipp;

            p2 = p;
            p = p2->overflow;
            p2->overflow = 0;

            idx = p2->key->str_hash & new_hash_mask;
            for (ipp = &new_hash_table[idx]; *ipp; ipp = &(*ipp)->overflow)
                ;
            *ipp = p2;
        }
    }

    /*
     * swap them over
     */
    mem_free(stp->hash_table);
    stp->hash_table = new_hash_table;
    stp->hash_modulus = new_hash_modulus;
    stp->hash_mask = new_hash_mask;
    trace(("}\n"));
}


/*
 * NAME
 *      symtab_query - search for a variable
 *
 * SYNOPSIS
 *      int symtab_query(symtab_ty *, string_ty *key);
 *
 * DESCRIPTION
 *      The symtab_query function is used to reference a variable.
 *
 * RETURNS
 *      If the variable has been defined, the function returns a non-zero value
 *      and the value is returned through the 'value' pointer.
 *      If the variable has not been defined, it returns zero,
 *      and 'value' is unaltered.
 */

void *
symtab_query(symtab_ty *stp, string_ty *key)
{
    str_hash_ty     idx;
    symtab_row_ty   *p;
    void            *result;

    trace(("symtab_query(stp = %p, key = \"%s\")\n{\n", stp, key->str_text));
    result = 0;

    idx = key->str_hash & stp->hash_mask;
    for (p = stp->hash_table[idx]; p; p = p->overflow)
    {
        if (str_equal(key, p->key))
        {
            result = p->data;
            break;
        }
    }

    trace(("return %p;\n", result));
    trace(("}\n"));
    return result;
}


static void
symtab_query_fuzzy_inner(symtab_ty *stp, string_ty *key, double *best_weight,
    string_ty **best_key, void **best_result)
{
    str_hash_ty     idx;
    symtab_row_ty   *p;

    trace(("symtab_query_fuzzy_inner(stp = %p, key = \"%s\")\n{\n", stp,
        key->str_text));

    for (idx = 0; idx < stp->hash_modulus; ++idx)
    {
        for (p = stp->hash_table[idx]; p; p = p->overflow)
        {
            double      w;

            w = fstrcmp(key->str_text, p->key->str_text);
            if (w > *best_weight)
            {
                *best_key = p->key;
                *best_weight = w;
                *best_result = p->data;
            }
        }
    }
    trace(("}\n"));
}


void *
symtab_query_fuzzy(symtab_ty *stp, string_ty *key, string_ty **key_used)
{
    double          best_weight;
    void            *best_result;
    string_ty       *best_key;

    trace(("symtab_query(stp = %p, key = \"%s\")\n{\n", stp, key->str_text));
    best_weight = 0.6;
    best_result = 0;
    best_key = 0;

    symtab_query_fuzzy_inner(stp, key, &best_weight, &best_key, &best_result);

    if (key_used && best_key)
        *key_used = best_key;
    trace(("return %p;\n", best_result));
    trace(("}\n"));
    return best_result;
}


void *
symtab_query_fuzzyN(symtab_ty **stp_table, size_t stp_length, string_ty *key,
    string_ty **key_used)
{
    double          best_weight;
    void            *best_result;
    string_ty       *best_key;
    size_t          j;

    trace(("symtab_query_fuzzyN(key = \"%s\")\n{\n", key->str_text));
    best_weight = 0.6;
    best_result = 0;
    best_key = 0;

    for (j = 0; j < stp_length; ++j)
    {
        symtab_query_fuzzy_inner
        (
            stp_table[j],
            key,
            &best_weight,
            &best_key,
            &best_result
        );
    }

    if (key_used && best_key)
        *key_used = best_key;
    trace(("return %p;\n", best_result));
    trace(("}\n"));
    return best_result;
}


/*
 * NAME
 *      symtab_assign - assign a variable
 *
 * SYNOPSIS
 *      void symtab_assign(symtab_ty *, string_ty *key, void *data);
 *
 * DESCRIPTION
 *      The symtab_assign function is used to assign
 *      a value to a given variable.
 *
 * CAVEAT
 *      The name is copied, the data is not.
 */

void
symtab_assign(symtab_ty *stp, string_ty *key, void *data)
{
    str_hash_ty     idx;
    symtab_row_ty   *p;

    trace(("symtab_assign(stp = %p, key = \"%s\", data = %p)\n{\n", stp,
        key->str_text, data));
    idx = key->str_hash & stp->hash_mask;
    for (p = stp->hash_table[idx]; p; p = p->overflow)
    {
        if (str_equal(key, p->key))
        {
            trace(("modify existing entry\n"));
            if (stp->reap)
                stp->reap(p->data);
            p->data = data;
            goto done;
        }
    }

    trace(("new entry\n"));
    p = mem_alloc(sizeof(symtab_row_ty));
    p->key = str_copy(key);
    p->overflow = stp->hash_table[idx];
    p->data = data;
    stp->hash_table[idx] = p;

    stp->hash_load++;
    if (stp->hash_load * 10 >= stp->hash_modulus * 8)
        split(stp);
    done:
    trace(("}\n"));
}


/*
 * NAME
 *      symtab_assign_push - assign a variable
 *
 * SYNOPSIS
 *      void symtab_assign_push(symtab_ty *, string_ty *key, void *data);
 *
 * DESCRIPTION
 *      The symtab_assign function is used to assign
 *      a value to a given variable.
 *      Any previous value will be obscured until this one
 *      is deleted with symtab_delete.
 *
 * CAVEAT
 *      The name is copied, the data is not.
 */

void
symtab_assign_push(symtab_ty *stp, string_ty *key, void *data)
{
    str_hash_ty     idx;
    symtab_row_ty   *p;

    trace(("symtab_assign_push(stp = %p, key = \"%s\", data = %p)\n{\n", stp,
        key->str_text, data));
    idx = key->str_hash & stp->hash_mask;

    p = mem_alloc(sizeof(symtab_row_ty));
    p->key = str_copy(key);
    p->overflow = stp->hash_table[idx];
    p->data = data;
    stp->hash_table[idx] = p;

    stp->hash_load++;
    if (stp->hash_load * 10 >= stp->hash_modulus * 8)
        split(stp);
    trace(("}\n"));
}


/*
 * NAME
 *      symtab_delete - delete a variable
 *
 * SYNOPSIS
 *      void symtab_delete(string_ty *name, symtab_class_ty class);
 *
 * DESCRIPTION
 *      The symtab_delete function is used to delete variables.
 *
 * CAVEAT
 *      The name is freed, the data is reaped.
 *      (By default, reap does nothing.)
 */

void
symtab_delete(symtab_ty *stp, string_ty *key)
{
    str_hash_ty     idx;
    symtab_row_ty   **pp;

    trace(("symtab_delete(stp = %p, key = \"%s\")\n{\n", stp, key->str_text));
    idx = key->str_hash & stp->hash_mask;

    pp = &stp->hash_table[idx];
    for (;;)
    {
        symtab_row_ty   *p;

        p = *pp;
        if (!p)
            break;
        if (str_equal(key, p->key))
        {
            if (stp->reap)
                stp->reap(p->data);
            str_free(p->key);
            *pp = p->overflow;
            mem_free(p);
            stp->hash_load--;
            break;
        }
        pp = &p->overflow;
    }
    trace(("}\n"));
}


/*
 * NAME
 *      symtab_dump - dump symbol table
 *
 * SYNOPSIS
 *      void symtab_dump(symtab_ty *stp, char *caption);
 *
 * DESCRIPTION
 *      The symtab_dump function is used to dump the contents of the
 *      symbol table.  The caption will be used to indicate why the
 *      symbol table was dumped.
 *
 * CAVEAT
 *      This function is only available when symbol DEBUG is defined.
 */

#ifdef DEBUG

void
symtab_dump(symtab_ty *stp, char *caption)
{
    str_hash_ty     j;
    symtab_row_ty   *p;

    error_raw("symbol table %s = {", caption);
    for (j = 0; j < stp->hash_modulus; ++j)
    {
        for (p = stp->hash_table[j]; p; p = p->overflow)
        {
            error_raw("key = \"%s\", data = %p", p->key->str_text, p->data);
        }
    }
    error_raw("}");
}

#endif


void
symtab_walk(symtab_ty *stp, void (*func)(symtab_ty *, string_ty *, void *,
    void *), void *arg)
{
    str_hash_ty     j;
    symtab_row_ty   *rp;

    trace(("symtab_walk(stp = %p)\n{\n", stp));
    for (j = 0; j < stp->hash_modulus; ++j)
        for (rp = stp->hash_table[j]; rp; rp = rp->overflow)
            func(stp, rp->key, rp->data, arg);
    trace(("}\n"));
}

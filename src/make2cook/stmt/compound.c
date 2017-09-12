/*
 *      cook - file construction tool
 *      Copyright (C) 1994, 1997, 2006, 2007 Peter Miller;
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

#include <common/mem.h>
#include <make2cook/stmt/compound.h>
#include <common/trace.h>


typedef struct stmt_compound_ty stmt_compound_ty;
struct stmt_compound_ty
{
    STMT
    size_t          nlines;
    size_t          nlines_max;
    stmt_ty         **line;
};


static void
constructor(stmt_ty *that)
{
    stmt_compound_ty *this;

    trace(("compound::constructor()\n{\n"));
    this = (stmt_compound_ty *)that;
    this->nlines = 0;
    this->nlines_max = 0;
    this->line = 0;
    trace(("}\n"));
}


static void
destructor(stmt_ty *that)
{
    stmt_compound_ty *this;
    size_t          j;

    trace(("compound::destructor()\n{\n"));
    this = (stmt_compound_ty *) that;
    for (j = 0; j < this->nlines; ++j)
        stmt_free(this->line[j]);
    if (this->line)
        mem_free(this->line);
    this->nlines = 0;
    this->nlines_max = 0;
    this->line = 0;
    trace(("}\n"));
}


static void
emit(stmt_ty *that)
{
    stmt_compound_ty *this;
    size_t          j;

    trace(("compound::emit()\n{\n"));
    this = (stmt_compound_ty *) that;
    if (this->nlines < 1)
        return;
    for (j = 0; j < this->nlines; ++j)
        stmt_emit(this->line[j]);
    trace(("}\n"));
}


static void
regroup(stmt_ty *that)
{
    stmt_compound_ty *this;
    stmt_ty         **line;
    size_t          nlines;
    size_t          j;

    trace(("compound::regroup()\n{\n"));
    this = (stmt_compound_ty *)that;
    line = this->line;
    nlines = this->nlines;
    this->line = 0;
    this->nlines = 0;
    this->nlines_max = 0;
    for (j = 0; j < nlines; ++j)
        stmt_regroup(line[j]);

    j = 0;
    while (j < nlines)
    {
        if (line[j]->white_space)
        {
            stmt_ty         *sp;

            sp = stmt_compound_alloc();
            for (;;)
            {
                stmt_compound_append(sp, line[j]);
                ++j;
                if (!line[j - 1]->white_space)
                    break;
                if (j >= nlines)
                    break;
            }
            stmt_compound_append(that, sp);
        }
        else
        {
            stmt_compound_append(that, line[j]);
            ++j;
        }
    }
    if (line)
        mem_free(line);
    trace(("}\n"));
}


static int
intersect(string_list_ty *a, string_list_ty *b)
{
    size_t          j;

    for (j = 0; j < a->nstrings; ++j)
        if (string_list_member(b, a->string[j]))
            return 1;
    return 0;
}


static void
sort(stmt_ty *that)
{
    stmt_compound_ty *this;
    size_t          j;

    trace(("compound::sort()\n{\n"));
    this = (stmt_compound_ty *)that;

    /*
     * sort each of the inner statements
     */
    for (j = 0; j < this->nlines; ++j)
        stmt_sort(this->line[j]);

    /*
     * sort the list of statements
     */
    j = 0;
    while (j < this->nlines)
    {
        stmt_ty         *sp;
        size_t          k;
        size_t          m;

        /*
         * look for statements which override macros
         */
        sp = this->line[j];
        if (!sp->mdef.nstrings)
        {
            ++j;
            continue;
        }

        /*
         * look for earlier statements which reference
         * those macros
         */
        for (k = 0; k < j && !intersect(&this->line[k]->ref, &sp->mdef); ++k)
            ;
        if (k >= j)
        {
            ++j;
            continue;
        }

        /*
         * move the definition to before the reference
         */
        for (m = j; m > k; --m)
            this->line[m] = this->line[m - 1];
        this->line[k] = sp;
        j = 0;

        /*
         * infinite loop possible:
         * assume this is a valid Makefile
         */
    }
    trace(("}\n"));
}


static stmt_method_ty method =
{
    sizeof(stmt_compound_ty),
    "compound",
    constructor,
    destructor,
    emit,
    regroup,
    sort,
};


stmt_ty *
stmt_compound_alloc(void)
{
    return stmt_alloc(&method);
}


void
stmt_compound_prepend(stmt_ty *that, stmt_ty *lp)
{
    stmt_compound_ty *this;
    size_t          j;

    trace(("stmt_compound_prepend()\n{\n"));
    this = (stmt_compound_ty *)that;
    if (this->nlines >= this->nlines_max)
    {
        size_t          nbytes;

        this->nlines_max = this->nlines_max * 2 + 4;
        nbytes = this->nlines_max * sizeof(stmt_ty *);
        this->line = mem_change_size(this->line, nbytes);
    }
    for (j = this->nlines; j > 0; --j)
        this->line[j] = this->line[j - 1];
    this->nlines++;
    this->line[0] = lp;
    stmt_variable_merge(that, lp);
    trace(("}\n"));
}


static void
append_n(stmt_compound_ty *this, stmt_ty **spp, size_t n)
{
    size_t          j;

    while (this->nlines + n > this->nlines_max)
    {
        size_t          nbytes;

        this->nlines_max = this->nlines_max * 2 + 4;
        nbytes = this->nlines_max * sizeof(stmt_ty *);
        this->line = mem_change_size(this->line, nbytes);
    }
    for (j = 0; j < n; ++j)
    {
        this->line[this->nlines++] = spp[j];
        stmt_variable_merge((stmt_ty *) this, spp[j]);
    }
}


void
stmt_compound_append(stmt_ty *that, stmt_ty *lp)
{
    stmt_compound_ty *this;

    trace(("stmt_compound_append()\n{\n"));
    this = (stmt_compound_ty *)that;
    if (lp->method == &method)
    {
        stmt_compound_ty *lp2;

        /*
         * Treat appended compound statements differently, so
         * that they can be regrouped better.
         */
        trace(("mark\n"));
        lp2 = (stmt_compound_ty *)lp;
        append_n(this, lp2->line, lp2->nlines);
        lp2->nlines = 0;        /* only free the array, not the body */
        stmt_free(lp);
    }
    else
        append_n(this, &lp, (size_t) 1);
    trace(("}\n"));
}

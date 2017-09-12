/*
 *      cook - file construction tool
 *      Copyright (C) 1997, 2006, 2007 Peter Miller;
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
#include <cook/opcode/label.h>
#include <common/trace.h>


/*
 * NAME
 *      opcode_label_new
 *
 * SYNOPSIS
 *      opcode_label_ty *opcode_label_new(void);
 *
 * DESCRIPTION
 *      The opcode_label_new function is used to allocate a new label
 *      instance in dynamic memory, and initialize it in preparation for
 *      use.
 *
 * CAVEAT
 *      Use opcode_label_delete when you are done with it.
 */

opcode_label_ty *
opcode_label_new(void)
{
    opcode_label_ty *lp;

    trace(("opcode_label_new()\n{\n"));
    lp = mem_alloc(sizeof(opcode_label_ty));
    lp->pc = (size_t) (-1);
    lp->npending = 0;
    lp->npending_max = 0;
    lp->pending = 0;
    trace(("return %08lX;\n", (long)lp));
    trace(("}\n"));
    return lp;
}


/*
 * NAME
 *      opcode_label_delete
 *
 * SYNOPSIS
 *      void opcode_label_delete(opcode_label_ty *);
 *
 * DESCRIPTION
 *      The opcode_label_delete function is used to release the
 *      resources held by an opcode structure.
 */

void
opcode_label_delete(opcode_label_ty *lp)
{
    trace(("opcode_label_delete(lp = %08lX)\n{\n", (long)lp));
    if (lp->pending)
        mem_free(lp->pending);
    lp->pc = (size_t) (-1);
    lp->npending = 0;
    lp->npending_max = 0;
    lp->pending = 0;
    mem_free(lp);
    trace(("}\n"));
}


/*
 * NAME
 *      opcode_label_define
 *
 * SYNOPSIS
 *      void opcode_label_define(opcode_label_ty *, size_t);
 *
 * DESCRIPTION
 *      The opcode_label_define function is used to specify the location
 *      of a label.  Outstanding references will be resolved, future
 *      references will be set accordingly.
 *
 * CAVEAT
 *      Should be called exactly once per label.
 */

void
opcode_label_define(opcode_label_ty *lp, size_t where)
{
    size_t          j;

    trace(("opcode_label_define(lp = %08lX)\n{\n", (long)lp));
    assert(lp->pc == (size_t)(-1));
    lp->pc = where;
    trace(("pc = %ld\n", (long)lp->pc));
    for (j = 0; j < lp->npending; ++j)
        *lp->pending[j] = lp->pc;
    if (lp->pending)
        mem_free(lp->pending);
    lp->npending = 0;
    lp->npending_max = 0;
    lp->pending = 0;
    trace(("}\n"));
}


/*
 * NAME
 *      opcode_label_refer
 *
 * SYNOPSIS
 *      void opcode_label_refer(void);
 *
 * DESCRIPTION
 *      The opcode_label_refer function is used to refer to a label.  If
 *      the label is not yet defined, references will eb accumulated and
 *      resolved after the definition.
 *
 * CAVEAT
 *      The opcodes in which the labels are used are expected to remain
 *      in existence until the label is defined.
 */

void
opcode_label_refer(opcode_label_ty *lp, size_t *where)
{
    trace(("opcode_label_refer(lp = %08lX)\n{\n", (long)lp));
    *where = (size_t) (-1);
    if (lp->pc == (size_t) (-1))
    {
        if (lp->npending >= lp->npending_max)
        {
            size_t          nbytes;

            lp->npending_max = lp->npending_max * 2 + 4;
            nbytes = lp->npending_max * sizeof(lp->pending[0]);
            lp->pending = mem_change_size(lp->pending, nbytes);
        }
        lp->pending[lp->npending++] = where;
    }
    else
        *where = lp->pc;
    trace(("}\n"));
}

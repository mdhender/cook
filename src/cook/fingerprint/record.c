/*
 *      cook - file construction tool
 *      Copyright (C) 1999, 2001, 2006, 2007 Peter Miller;
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

#include <cook/fingerprint/record.h>
#include <cook/fingerprint/subdir.h>
#include <cook/fingerprint/value.h>
#include <common/mem.h>
#include <common/trace.h>


/*
 * NAME
 *      fp_record_new
 *
 * SYNOPSIS
 *      fp_record_ty *fp_record_new(fp_subdir_ty *parent);
 *
 * DESCRIPTION
 *      The fp_record_new function is used to allocate and initialize
 *      a fp_record_ty structure in dynamic memory.  Use fp_record-delete
 *      when you are done with it.
 *
 *      Note: the parent's dirty flag is not altered.
 */

fp_record_ty *
fp_record_new(string_ty *filename, fp_subdir_ty *parent)
{
    fp_record_ty    *this;

    trace(("fp_record_new(parent = %08lX)\n{\n", (long)parent));
    this = mem_alloc(sizeof(fp_record_ty));
    this->filename = str_copy(filename);
    this->parent = parent;
    this->exists = 0;
    fp_value_constructor(&this->value);
    trace(("return %08lX;\n", (long)this));
    trace(("}\n"));
    return this;
}


/*
 * NAME
 *      fp_record_new2
 *
 * SYNOPSIS
 *      fp_record_ty *fp_record_new2(fp_subdir_ty *parent, fp_value_ty *value);
 *
 * DESCRIPTION
 *      The fp_record_new2 function is used to allocate and initialize a
 *      fp_record_ty structure in dynamic memory.  The parent and value
 *      fields are set as specified.  Use fp_record_delete when you are
 *      done with it.
 *
 *      Note: the parent's dirty flag is not altered.
 */

fp_record_ty *
fp_record_new2(string_ty *filename, fp_subdir_ty *parent, fp_value_ty *fp)
{
    fp_record_ty    *this;

    trace(("fp_record_new2(parent = %08lX, fp = %08lX)\n{\n", (long)parent,
        (long)fp));
    this = mem_alloc(sizeof(fp_record_ty));
    this->filename = str_copy(filename);
    this->parent = parent;
    this->exists = 1;
    fp_value_constructor_copy(&this->value, fp);
    trace(("return %08lX;\n", (long)this));
    trace(("}\n"));
    return this;
}


/*
 * NAME
 *      fp_record_delete
 *
 * SYNOPSIS
 *      void fp_record_delete(fp_record_ty *);
 *
 * DESCRIPTION
 *      The fp_record_delete function is used to release the resources
 *      held by a fp_record_ty structure in dynamic memory.
 *
 *      Note: the parent's dirty flag is not altered.
 */

void
fp_record_delete(fp_record_ty *this)
{
    trace(("fp_record_delete(this = %08lX)\n{\n", (long)this));
    str_free(this->filename);
    fp_value_destructor(&this->value);
    mem_free(this);
    trace(("}\n"));
}


/*
 * NAME
 *      fp_record_write
 *
 * SYNOPSIS
 *      void fp_record_write(fp_record_ty *, string_ty *, FILE *);
 *
 * DESCRIPTION
 *      The fp_record_write function is used to write a fp_record_ty
 *      structure to the fingerprint cache file on disk.  the value is
 *      only written if it exists.
 */

void
fp_record_write(fp_record_ty *this, string_ty *key, FILE *fp)
{
    trace(("fp_record_write(this = %08lX, key = \"%s\", fp = %08lX)\n{\n",
        (long)this, key->str_text, (long)fp));
    if (this->exists)
        fp_value_write(&this->value, (key ? key : this->filename), fp);
    trace(("}\n"));
}


/*
 * NAME
 *      fp_record_update
 *
 * SYNOPSIS
 *      void fp_record_update(fp_record _ty *this, fp_value_ty *value);
 *
 * DESCRIPTION
 *      The fp_record_update function is used to update the value of
 *      a fingerprint held in a fp_record_ty structure.  The existence
 *      attributes is updated if necessary.  The parent's dirty flag is
 *      set if necessary.
 */

void
fp_record_update(fp_record_ty *this, fp_value_ty *fp)
{
    trace(("fp_record_update(this = %08lX, fp = %08lX)\n{\n", (long)this,
        (long)fp));
    if (!this->exists || !fp_value_equal_all(&this->value, fp))
    {
        trace(("need to update\n"));
        this->exists = 1;
        fp_subdir_dirty_notify(this->parent, this->filename);
        fp_value_copy(&this->value, fp);
    }
    trace(("}\n"));
}


/*
 * NAME
 *      fp_record_clear
 *
 * SYNOPSIS
 *      void fp_record_clear(fp_record_ty *this);
 *
 * DESCRIPTION
 *      The fp_record_clear function is used to clear the value of
 *      a fingerprint held in a fp_record_ty structure.  The existence
 *      attribute is updated if necessary.  The parent's dirty flag is
 *      set if necessary.
 */

void
fp_record_clear(fp_record_ty *this)
{
    trace(("fp_record_clear(this = %08lX)\n{\n", (long)this));
    if (this->exists)
    {
        this->exists = 0;
        fp_subdir_dirty_notify(this->parent, this->filename);
    }
    trace(("}\n"));
}


/*
 * NAME
 *      fp_record_tweak
 *
 * SYNOPSIS
 *      void fp_record_tweak(fp_record_ty *this, time_t when,
 *              string_ty *crypto);
 *
 * DESCRIPTION
 *      The fp_record_tweak function is used to tweak the value
 *      of a fingerprint held in a fp_record_ty structure IF it is
 *      out-of-date with respect to the filesystem.  The existence
 *      attribute is updated if necessary.  The parent's dirty flag is
 *      set if necessary.
 */

void
fp_record_tweak(fp_record_ty *this, time_t when, string_ty *crypto)
{
    /*
     * If the file is not known to exist, or if the fingerprint is
     * different, replace the value with the information given.
     */
    if (!this->exists || !str_equal(this->value.contents_fingerprint, crypto))
    {
        fp_value_ty     value;

        fp_subdir_dirty_notify(this->parent, this->filename);
        fp_value_constructor3(&value, when, when, crypto);
        fp_value_copy(&this->value, &value);
        fp_value_destructor(&value);
        this->exists = 1;
    }
    else if (this->value.newest != when)
    {
        /*
         * If the newest time is the same (it is supposted to
         * be the same as the stat::st_mtime value) then leave
         * everything alone.  (The upper bound on oldest is in
         * case the file heads into the past.)
         */
        fp_subdir_dirty_notify(this->parent, this->filename);
        this->value.newest = when;
        if (this->value.oldest >= when)
            this->value.oldest = when;
    }
}

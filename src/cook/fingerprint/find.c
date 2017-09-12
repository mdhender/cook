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
 * MANIFEST: functions to manipulate finds
 */

#include <ac/string.h>

#include <fingerprint/find.h>
#include <fingerprint/record.h>
#include <fingerprint/subdir.h>
#include <option.h>
#include <os_interface.h>
#include <os/rel_if_poss.h>
#include <quit.h>
#include <symtab.h>
#include <trace.h>


/*
 * This is the table of all known files, from all sub-directories.
 */
static symtab_ty *main_stp;

/*
 * This is the table of known sub-directories.
 */
static symtab_ty *subdir_stp;


/*
 * NAME
 *	subdir_reap
 *
 * SYNOPSIS
 *	void subdir_reap(void *ptr);
 *
 * DESCRIPTION
 *	The subdir_reap function is used to release the resources held
 *	by a symbol table entry.  In this case, and entry which is a
 *	dynamically allocated fp_subdir_ty structure.
 */

static void subdir_reap _((void *));

static void
subdir_reap(p)
	void		*p;
{
	fp_subdir_ty	*sdp;

	trace(("subdir_reap\n"));
	sdp = p;
	fp_subdir_delete(sdp);
}


/*
 * NAME
 *	fp_find_subdir
 *
 * SYNOPSIS
 *	fp_subdir_ty *fp_find_subdir(string_ty *);
 *
 * DESCRIPTION
 *	The fp_find_subdir function is used to locate a fingerprint
 *	subdir, given the filename.  If the subdir has not been seen
 *	before, one will be created.
 */

fp_subdir_ty *
fp_find_subdir(dirname, ok_to_read)
	string_ty	*dirname;
	int		ok_to_read;
{
	fp_subdir_ty	*sdp;

	/*
	 * Most of the time, we have already polled this one before,
	 * so do a quick lookup and see.
	 */
	trace(("fp_find_subdir(dirname = \"%s\")\n{\n", dirname->str_text));
	if (!subdir_stp)
	{
		static string_ty *dot;

		subdir_stp = symtab_alloc(10);
		subdir_stp->reap = subdir_reap;

		/*
		 * Read dot (the current directory) immediately.
		 * This primes the cache with all the things that were
		 * found to be read-only on previous runs.
		 */
		if (!dot)
			dot = str_from_c(".");
		sdp = fp_subdir_new(dot);
		symtab_assign(subdir_stp, dot, sdp);
		fp_subdir_read(sdp);
	}
	sdp = symtab_query(subdir_stp, dirname);
	if (!sdp)
	{
		/*
		 * It's a new one, we must allocate it.
		 */
		sdp = fp_subdir_new(dirname);
		symtab_assign(subdir_stp, dirname, sdp);
	}

	if (ok_to_read && sdp->need_to_read)
	{
		/*
		 * Read in the cache file from the directory,
		 * if it exists.  This causes a number of
		 * fp_find_update calls (see below), one for
		 * each entry.
		 */
		fp_subdir_read(sdp);
	}

	trace(("return %08lX;\n", (long)sdp));
	trace(("}\n"));
	return sdp;
}


static symtab_ty *
get_main_stp(void)
{
	if (!main_stp)
	{
		main_stp = symtab_alloc(100);
		/*
		 * DO NOT set main_stp->reap
		 * as this will be done by fp_subdir_delete
		 */
		quit_handler_prio(fp_find_flush);
	}
	return main_stp;
}


/*
 * NAME
 *	fp_find_record
 *
 * SYNOPSIS
 *	fp_record_ty *fp_find_record(string_ty *);
 *
 * DESCRIPTION
 *	The fp_find_record function is used to locate a fingerprint
 *	record, given the filename.  If the record has not been seen
 *	before, one will be created.
 */

fp_record_ty *
fp_find_record(path_a)
	string_ty	*path_a;
{
	fp_record_ty	*p;
	string_ty	*path_r;

	/*
	 * Most of the time, we have already polled this one before,
	 * so do a quick lookup and see.
	 */
	trace(("fp_find_record(path = \"%s\")\n{\n", path_a->str_text));
	path_r = os_relative_if_possible(path_a);
	p = symtab_query(get_main_stp(), path_r);
	if (!p)
	{
		string_ty	*dirname;
		string_ty	*entryname;
		fp_subdir_ty	*sdp;

		/*
		 * It isn't there.  See if we know it's directory
		 * (or create a new subdir entry).
		 */
		dirname = os_dirname_relative(path_r);
		sdp = fp_find_subdir(dirname, 1);
		str_free(dirname);

		/*
		 * After all that, see if the path is now in the subdir table.
		 */
		entryname = os_entryname(path_r);
		p = symtab_query(sdp->stp, entryname);
		if (!p)
		{
			/*
			 * No luck.  Create an ``I'm not here'' entry.
			 */
			p = fp_record_new(entryname, sdp);
			symtab_assign(sdp->stp, entryname, p);
		}
		str_free(entryname);

		/*
		 * Create a path entry in the main symbol table.
		 */
		symtab_assign(main_stp, path_r, p);
	}

	str_free(path_r);
	trace(("return %08lX;\n", (long)p));
	trace(("}\n"));
	return p;
}


/*
 * NAME
 *	fp_find_update
 *
 * SYNOPSIS
 *	void fp_find_update(fp_subdir_ty *sdp, string_ty *file,
 *		fp_value_ty *value);
 *
 * DESCRIPTION
 *	The fp_find_update function is used to add entries to the symbol
 *	tables, used exclusively by the fp_gram parser.  The filename
 *	given is relative to the subdirectory given.
 */

void
fp_find_update(sdp, file, fp)
	fp_subdir_ty	*sdp;
	string_ty	*file;
	fp_value_ty	*fp;
{
	string_ty	*filename;
	fp_record_ty	*p;

	/*
	 * build the actual filename
	 */
	trace(("fp_find_update(sdp = %08lX, file = \"%s\", fp = %08lX)\n{\n",
		(long)sdp, file->str_text, (long)fp));
	filename = os_path_cat(sdp->path, file);

	/*
	 * If a cache file has recorded fingerprints from its
	 * sub-directories, it means that the subdirectory is (probably)
	 * unwritable.  When dot is written out, these will be, too.
	 */
	if (strchr(file->str_text, '/'))
	{
		string_ty	*dirname;

		dirname = os_dirname_relative(filename);
		sdp = fp_find_subdir(dirname, 0);
		str_free(dirname);

		/*
		 * There are two flags to choose from:
		 *
		 *	cache_in_dot
		 *		which is used for cache files which
		 *		can't be written into their directory,
		 *		causing them to be cached into dot.
		 *	dirty
		 *		which is used to indicate cache file
		 *		which needto be written out.
		 *
		 * If, when attempting to write the file out, permission
		 * is denied, cache_in_dot will be set.
		 *
		 * By setting the dirty flag, if the directory has been
		 * created in the mean time, or the mode changed, the
		 * cache file will write to the correct place.	If the
		 * directory is still unwritable, it will be written
		 * back into the dot cache.
		 */
		sdp->cache_in_dot = 0;
		sdp->dirty = 1;

		file = os_entryname(filename);
	}
	else
		file = str_copy(file);

	/*
	 * If the main symbol table has *ever* heard of this one, throw
	 * it away.  It could have been created recently, or delete
	 * could have over-ridden it, etc.
	 */
	p = symtab_query(get_main_stp(), filename);
	if (p)
	{
		str_free(filename);
		str_free(file);
		trace(("hmm...\n"));
		trace(("}\n"));
		return;
	}

	/*
	 * create a new record
	 *
	 * Note: the parent's dirty flag is not altered.
	 */
	assert(!symtab_query(sdp->stp, file));
	p = fp_record_new2(file, sdp, fp);

	/*
	 * Hook the record into the two symbol tables.
	 */
	symtab_assign(sdp->stp, file, p);
	str_free(file);
	symtab_assign(main_stp, filename, p);
	str_free(filename);
	trace(("}\n"));
}


/*
 * NAME
 *	subdir_walk
 *
 * SYNOPSIS
 *	void subdir_walk(symtab_ty *stp, string_ty *key, void *data, void *aux);
 *
 * DESCRIPTION
 *	The subdir_walk function is used to walk the symbol table of a
 *	sub-directory when writing the fingerprint cache to disk.
 */

static int need_to_write_dot;

static void subdir_walk _((symtab_ty *, string_ty *, void *, void *));

static void
subdir_walk(stp, key, data, aux)
	symtab_ty	*stp;
	string_ty	*key;
	void		*data;
	void		*aux;
{
	fp_subdir_ty	*sdp;
	static string_ty *dot;

	trace(("subdir_walk(key = \"%s\")\n{\n", key->str_text));
	if (!dot)
		dot = str_from_c(".");
	sdp = data;
	if (!str_equal(sdp->path, dot))
		fp_subdir_write(sdp, &need_to_write_dot);
	trace(("}\n"));
}


/*
 * NAME
 *	fp_find_flush
 *
 * SYNOPSIS
 *	void fp_find_flush(void);
 *
 * DESCRIPTION
 *	The fp_find_flush function is used to flush the fingerprint
 *	cache files to disk.  Only files which changed are written out.
 */

void
fp_find_flush()
{
	static string_ty *dot;
	fp_subdir_ty	*sdp;

	if (!option_test(OPTION_FINGERPRINT_WRITE))
	{
		trace(("no fp write\n"));
		return;
	}

	/*
	 * Write out all of the known subdirectories, except dot.
	 */
	trace(("fp_find_flush()\n{\n"));
	need_to_write_dot = 0;
	if (subdir_stp)
		symtab_walk(subdir_stp, subdir_walk, 0);

	/*
	 * Do dot last, so we catch all of the `cache_in_dot's.
	 */
	if (!dot)
		dot = str_from_c(".");
	sdp = fp_find_subdir(dot, 0);
	if (need_to_write_dot)
		sdp->dirty = 1;
	fp_subdir_write(sdp, &need_to_write_dot);
	trace(("}\n"));
}


static void fp_find_main_writer _((symtab_ty *, string_ty *, void *, void *));

static void
fp_find_main_writer(stp, key, data, aux)
	symtab_ty	*stp;
	string_ty	*key;
	void		*data;
	void		*aux;
{
	FILE		*fp;
	fp_record_ty	*rp;
	fp_subdir_ty	*sdp;

	trace(("fp_find_main_writer(key = \"%s\")\n{\n", key->str_text));
	fp = aux;
	rp = data;
	sdp = rp->parent;

	/*
	 * Write all cache_in_dot records, not just the dirty ones.
	 * This is because we are writing out dot, which has all of the
	 * unwritables, leaving out the dirty ones means the cache is
	 * incomplete, and cache misses mean extra CPU cycles later.
	 */
	if (sdp->cache_in_dot)
		fp_record_write(rp, key, fp);
	trace(("}\n"));
}


static void fp_find_main_write2 _((symtab_ty *, string_ty *, void *, void *));

static void
fp_find_main_write2(stp, key, data, aux)
	symtab_ty	*stp;
	string_ty	*key;
	void		*data;
	void		*aux;
{
	fp_subdir_ty	*sdp;

	trace(("fp_find_main_write2(key = \"%s\")\n{\n", key->str_text));
	sdp = data;
	if (sdp->cache_in_dot)
		sdp->dirty = 0;
	trace(("}\n"));
}


void
fp_find_main_write(fp)
	void		*fp;
{
	trace(("fp_find_main_write(fp = %ld)\n{\n", (long)fp));
	if (main_stp)
		symtab_walk(main_stp, fp_find_main_writer, fp);
	if (subdir_stp)
		symtab_walk(subdir_stp, fp_find_main_write2, 0);
	trace(("}\n"));
}

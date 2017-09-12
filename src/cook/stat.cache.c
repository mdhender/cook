/*
 *      cook - file construction tool
 *      Copyright (C) 1992-1994, 1997-1999, 2001, 2002, 2006-2009 Peter Miller
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

#include <common/ac/string.h>
#include <common/ac/errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <common/error_intl.h>
#include <common/mem.h>
#include <common/symtab.h>
#include <common/trace.h>
#include <cook/archive.h>
#include <cook/fingerprint.h>
#include <cook/fingerprint/value.h>
#include <cook/option.h>
#include <cook/option.h>
#include <cook/stat.cache.h>


typedef struct cache_ty cache_ty;
struct cache_ty
{
    time_t          oldest;
    time_t          newest;
    time_t          stat_mod_time;
};

static symtab_ty *symtab[2];


/*
 * NAME
 *      init
 *
 * SYNOPSIS
 *      void init(int follow_links);
 *
 * DESCRIPTION
 *      The init function is used to init one of the two databases.
 *      It creates the symbol table, and sets its reap function.
 */

static void
init(int n)
{
    trace(("init()\n{\n"));
    if (!symtab[n])
    {
        symtab[n] = symtab_alloc(100);
        symtab[n]->reap = mem_free;
    }
    trace(("}\n"));
}


/*
 * NAME
 *      mem_copy_cache
 *
 * SYNOPSIS
 *      cache_ty *mem_copy_cache(cache_ty *);
 *
 * DESCRIPTION
 *      The mem_copy_cache creates a copy of a cache in dynamic memory.
 *      (New with copy constructor.)
 */

static cache_ty *
mem_copy_cache(cache_ty *st)
{
    cache_ty         *result;

    result = mem_alloc(sizeof(cache_ty));
    *result = *st;
    return result;
}


/*
 * NAME
 *      stat_cache_fingerprint
 *
 *      stat_cache_fingerprint - mangle the cache_ty value
 *
 * SYNOPSIS
 *      void stat_cache_fingerprint(string_ty *path, struct cache_ty *);
 *
 * DESCRIPTION
 *      The stat_cache_fingerprint function is used to mangle a cache_ty
 *      based on the file fingerprint information.  It also updates the
 *      file fingerprint database if necessary.
 */

static void
stat_cache_fingerprint(string_ty *path, cache_ty *cp)
{
    fp_value_ty      *fp;

    assert(cp->newest==cp->oldest);
    fp = fp_search(path);
    if (fp)
    {
        /*
         * we have seen this file before
         */
        if (fp->stat_mod_time == cp->stat_mod_time)
        {
            /*
             * file not modified since last seen
             */
            trace(("file not mod\n"));
            cp->oldest = fp->oldest;
            cp->newest = fp->newest;
        }
        else
        {
            string_ty       *s;
            fp_value_ty     data;
            static time_t   now;

            /*
             * It is important to use the same concept of "now" for
             * all file time adjustments.  That way, when both a target
             * and an ingredient change at the some time (yes, I know,
             * only the ingredient was *supposed* to change) then the
             * recipe will still be out-of-date.
             */
            if (!now)
                time(&now);

            /*
             * The file's last-modified time has changed since we last saw it.
             */
            trace(("file's mod time changed\n"));
            s = fp_fingerprint(path);
            if (!s)
                goto fp_not_useful;
            if (str_equal(fp->contents_fingerprint, s))
            {
                /*
                 * If the file's fingerprint hasn't changed, we simply
                 * extend the time interval for which the file is valid.
                 *
                 * Cook decides to execute a recipe body
                 *      if (newest(target) <= oldest(ingredient))
                 * so if newest(ingredient) moves forwards in time,
                 * which is what usually happens, this will not trigger
                 * a rebuild.
                 */
                cp->newest = fp->newest;
                if (cp->newest < cp->stat_mod_time)
                    cp->newest = cp->stat_mod_time;
                if (cp->newest < now)
                    cp->newest = now;

                cp->oldest = fp->oldest;
                if (cp->oldest > cp->stat_mod_time)
                    cp->oldest = cp->stat_mod_time;
                if (cp->oldest > now)
                    cp->oldest = now;
            }
            else
            {
                /*
                 * The "normal" case is the the file has been edited, and
                 * moved forwards in time.  This will trigger a rebuild.
                 *
                 * However there are some weird cases (e.g. ClearCase
                 * or 3DFS or unionfs, etc) where the file can move
                 * backwards in time, or even forwards "a little bit",
                 * that also mean the file has changed, and Cook needs
                 * to think they have moved far enough forwards in time
                 * to trigger a rebuild.
                 *
                 * The way we set the time range is subtle: Cook
                 * decides to execute a recipe body
                 *      if (newest(target) <= oldest(ingredient))
                 * so it is necessary to update the *oldest* time of
                 * the file, even though you may expect to be updating
                 * the *newest* time.
                 *
                 * Note that we don't go out of our way to include the
                 * file's modify time in the oldest-newest range.
                 *
                 * This is because Cook has to notice that the
                 * file *changed* in order to trigger a recipe body.
                 * Systems like ClearCase or even union file systems can
                 * hide or reveal different versions of the file, and
                 * the users will expect their files to be recompiled,
                 * irrespective of the actual mod-time on the file.
                 *
                 * And sometimes those mod-times aren't useful, e.g. when
                 * they move backwards, or when they move forwards but
                 * not forwards enough to trigger a rebuild based on
                 * the mod-time alone.
                 */
                cp->oldest = now;
                if (cp->oldest <= fp->oldest)
                {
                    /*
                     * Make sure Cook will think it's newer.
                     *
                     * If they are using time-adjust-back this will
                     * be enough to trigger a rebuild, and if they aren't
                     * there is no way to guess what will.
                     */
                    cp->oldest = fp->oldest + 1;
                }
                if (cp->oldest < cp->stat_mod_time)
                {
                    /*
                     * A small concession to clock slew, that doesn't
                     * compromise our ability to trigger a rebuild.
                     */
                    cp->oldest = cp->stat_mod_time;
                }
                cp->newest = cp->oldest;

                if (option_test(OPTION_REASON))
                {
                    struct tm       *tm;
                    sub_context_ty  *scp;

                    tm = localtime(&cp->stat_mod_time);
                    scp = sub_context_new();
                    sub_var_set_string(scp, "File_Name", path);
                    sub_var_set
                    (
                        scp,
                        "Number",
                        "%4d/%02d/%02d.%02d:%02d:%02d",
                        1900 + tm->tm_year,
                        tm->tm_mon + 1,
                        tm->tm_mday,
                        tm->tm_hour,
                        tm->tm_min,
                        tm->tm_sec
                    );
                    error_intl
                    (
                        scp,
                        i18n("mtime(\"$filename\") was $number until "
                            "fingerprinting (reason)")
                    );
                }
            }
            fp_value_constructor5
            (
                &data,
                cp->oldest,
                cp->newest,
                cp->stat_mod_time,
                s,
                fp->ingredients_fingerprint
            );
            str_free(s);
            fp_assign(path, &data);
            fp_value_destructor(&data);
        }
    }
    else
    {
        string_ty       *s;

        /*
         * never fingerprinted this file before
         */
        s = fp_fingerprint(path);
        if (!s)
        {
            fp_not_useful:
            fp_delete(path);
        }
        else
        {
            fp_value_ty     data;

            fp_value_constructor3(&data, cp->newest, cp->newest, s);
            str_free(s);
            fp_assign(path, &data);
            fp_value_destructor(&data);
        }
    }
}


/*
 * NAME
 *      stat_cache - stat() with caching
 *
 * SYNOPSIS
 *      int stat_cache(string_ty *path, struct stat *result);
 *
 * DESCRIPTION
 *      The stat_cache function is used to perform the same as the stat()
 *      system function, but the results are cached to avoid too many probes
 *      into the file system.  Files which do not exist are indicated by
 *      filling the result structure with zeros.
 *
 * RETURNS
 *      int; -1 on error, 0 on success
 *
 * CAVEAT
 *      Errors, other than ENOENT, result in a fatal diagnostic.
 */

static int
stat_cache(string_ty *path, cache_ty *cp, int follow_links)
{
    cache_ty        *data_p;
    int             err;
    struct stat     st;

    /*
     * if we have previously stat()ed this file,
     * return old information
     */
    trace(("stat_cache(path = \"%s\", lnk = %d)\n{\n", path->str_text,
        follow_links));
    if (!symtab[follow_links])
        init(follow_links);
    data_p = symtab_query(symtab[follow_links], path);
    if (data_p)
    {
        *cp = *data_p;
        trace(("got it from the cache\n"));
        trace(("return 0;\n"));
        trace(("}\n"));
        return 0;
    }

    /*
     * new file, perform stat() for the first time
     */
    trace(("stat(\"%s\")\n", path->str_text));
#if defined(S_IFLNK) || defined(S_ISLNK)
    if (!follow_links)
        err = lstat(path->str_text, &st);
    else
#endif
        err = stat(path->str_text, &st);
    if (err && errno == ENOENT)
        err = archive_stat(path, &st);
    if (err)
    {
        switch (errno)
        {
        case ENOENT:
        case ENOTDIR:
            /*
             * ENOENT occurs when a path element does not exist
             * ENOTDIR occurs when a path element (except the last)
             *              is not a directory.
             * Either way, the file being "stat"ed does not exist.
             */
            break;

        default:
            error_intl_stat(path->str_text);
            trace(("return -1;\n"));
            trace(("}\n"));
            return -1;
        }

        fp_delete(path);

        cp->newest = 0;
        cp->oldest = 0;
        cp->stat_mod_time = 0;
    }
    else
    {
        /*
         * make sure the times of existing files
         * are always positive
         */
        if (st.st_mtime < 1)
            st.st_mtime = 1;

        /*
         * Make sure we aren't tricked by fancy footwork with the file's
         * mod time.  The st_ctime field records when the inode's
         * meta-data (e.g. the st_mtime member) was last changed.
         */
        if
        (
            option_test(OPTION_FINGERPRINT)
        &&
            option_test(OPTION_CTIME)
        &&
            st.st_mtime < st.st_ctime
        )
            st.st_mtime = st.st_ctime;

        cp->oldest = st.st_mtime;
        cp->newest = st.st_mtime;
        cp->stat_mod_time = st.st_mtime;

        /*
         * see if we have its fingerprint on file
         */
        if (option_test(OPTION_FINGERPRINT))
            stat_cache_fingerprint(path, cp);
    }

    /*
     * remember the stat information
     */
    symtab_assign(symtab[follow_links], path, mem_copy_cache(cp));
    trace(("return 0;\n"));
    trace(("}\n"));
    return 0;
}


/*
 * NAME
 *      stat_cache_newest
 *
 * SYNOPSIS
 *      time_t stat_cache_newest(string_ty *path, int follow_links);
 *
 * DESCRIPTION
 *      The stat_cahe_newest function is used to obtain the upper bound of
 *      the time range for which the file has had its current contents.
 *
 * RETURNS
 *      time_t; the time the file was last modified, (time_t)0 if the
 *      file does not exist.
 *
 * CAVEAT
 *      All errors except ENOENT result in a fatal error message.
 */

time_t
stat_cache_newest(string_ty *path, int follow_links)
{
    cache_ty        cache;
    sub_context_ty  *scp;

    trace(("stat_cache_newest(path = \"%s\", lnk = %d)\n{\n", path->str_text,
        follow_links));
    if (stat_cache(path, &cache, !!follow_links))
    {
        trace(("Bzzt!\n"));
        trace(("}\n"));
        return -1;
    }

    /*
     * trace the last-modified time
     */
    if (option_test(OPTION_REASON))
    {
        if (!cache.stat_mod_time)
        {
            scp = sub_context_new();
            sub_var_set_string(scp, "File_Name", path);
            error_intl(scp, i18n("mtime(\"$filename\") == ENOENT (reason)"));
            sub_context_delete(scp);
        }
        else
        {
            struct tm       *tm;

            tm = localtime(&cache.newest);
            scp = sub_context_new();
            sub_var_set_string(scp, "File_Name", path);
            sub_var_set
            (
                scp,
                "Number",
                "%4d/%02d/%02d.%02d:%02d:%02d",
                1900 + tm->tm_year,
                tm->tm_mon + 1,
                tm->tm_mday,
                tm->tm_hour,
                tm->tm_min,
                tm->tm_sec
            );
            if
            (
                option_test(OPTION_FINGERPRINT)
            &&
                cache.newest != cache.stat_mod_time
            )
            {
                error_intl
                (
                    scp,
                    i18n("newest mtime(\"$filename\") == $number (reason)")
                );
            }
            else
            {
                error_intl
                (
                    scp,
                    i18n("mtime(\"$filename\") == $number (reason)")
                );
            }
        }
    }
    trace(("return (%ld) %s", (long)cache.newest, ctime(&cache.newest)));
    trace(("}\n"));
    return cache.newest;
}


/*
 * NAME
 *      stat_cache_oldest
 *
 * SYNOPSIS
 *      time_t stat_cache_oldest(string_ty *path, int follow_links);
 *
 * DESCRIPTION
 *      The stat_cahe_oldest function is used to obtain the lower bound of
 *      the time range for which the file has had its current contents.
 *
 * RETURNS
 *      time_t; the time the file was last modified, (time_t)0 if the
 *      file does not exist.
 *
 * CAVEAT
 *      All errors except ENOENT result in a fatal error message.
 */

time_t
stat_cache_oldest(string_ty *path, int follow_links)
{
    cache_ty        cache;
    sub_context_ty  *scp;

    trace(("stat_cache_oldest(path = \"%s\", lnk = %d)\n{\n", path->str_text,
        follow_links));
    if (stat_cache(path, &cache, !!follow_links))
    {
        trace(("Bzzt!\n"));
        trace(("}\n"));
        return -1;
    }

    /*
     * trace the last-modified time
     */
    if (option_test(OPTION_REASON))
    {
        if (!cache.oldest)
        {
            scp = sub_context_new();
            sub_var_set_string(scp, "File_Name", path);
            error_intl(scp, i18n("mtime(\"$filename\") == ENOENT (reason)"));
            sub_context_delete(scp);
        }
        else
        {
            struct tm       *tm;

            tm = localtime(&cache.oldest);
            scp = sub_context_new();
            sub_var_set_string(scp, "File_Name", path);
            sub_var_set
            (
                scp,
                "Number",
                "%4d/%02d/%02d.%02d:%02d:%02d",
                1900 + tm->tm_year,
                tm->tm_mon + 1,
                tm->tm_mday,
                tm->tm_hour,
                tm->tm_min,
                tm->tm_sec
            );
            if
            (
                option_test(OPTION_FINGERPRINT)
            &&
                cache.oldest != cache.stat_mod_time
            )
            {
                error_intl
                (
                    scp,
                    i18n("oldest mtime(\"$filename\") == $number (reason)")
                );
            }
            else
            {
                error_intl
                (
                    scp,
                    i18n("mtime(\"$filename\") == $number (reason)")
                );
            }
        }
    }
    trace(("return (%ld) %s", (long)cache.oldest, ctime(&cache.oldest)));
    trace(("}\n"));
    return cache.oldest;
}


void
stat_cache_set(string_ty *path, time_t when, int fp2)
{
    cache_ty        *data_p;
    cache_ty        cache;
    sub_context_ty  *scp;

    /*
     * clear the don't-follow-links cache
     */
    trace(("stat_cache_set(path = \"%s\")\n{\n", path->str_text));
    if (symtab[0])
        symtab_delete(symtab[0], path);

    if (!symtab[1])
        init(1);
    data_p = symtab_query(symtab[1], path);
    if (data_p)
    {
        if
        (
            !data_p->oldest
        ||
            !option_test(OPTION_FINGERPRINT)
        ||
            when < data_p->oldest
        )
            data_p->oldest = when;
        data_p->newest = when;
    }
    else
    {
        cache.oldest = when;
        cache.newest = when;
        cache.stat_mod_time = when;
        symtab_assign(symtab[1], path, mem_copy_cache(&cache));
        data_p = &cache;
    }

    /*
     * Update the fingerprint.
     * (Important not to lie here, the fp2 flags
     * says is immediately following a utime call.)
     */
    if (fp2 && option_test(OPTION_FINGERPRINT))
    {
        fp_value_ty     *fp;

        fp = fp_search(path);
        if (fp)
        {
            fp_value_ty     data;

            fp_value_constructor5
            (
                &data,
                fp->oldest,
                when,
                when,
                fp->contents_fingerprint,
                fp->ingredients_fingerprint
            );
            fp_assign(path, &data);
            fp_value_destructor(&data);
        }
    }

    /*
     * emit a trace
     */
    if (option_test(OPTION_REASON))
    {
        struct tm       *tm;

        tm = localtime(&when);
        scp = sub_context_new();
        sub_var_set_string(scp, "File_Name", path);
        sub_var_set
        (
            scp,
            "Number",
            "%4d/%02d/%02d.%02d:%02d:%02d",
            1900 + tm->tm_year,
            tm->tm_mon + 1,
            tm->tm_mday,
            tm->tm_hour,
            tm->tm_min,
            tm->tm_sec
        );
        if (option_test(OPTION_FINGERPRINT) && data_p->oldest != data_p->newest)
        {
            error_intl
            (
                scp,
                i18n("newest mtime(\"$filename\") = $number (reason)")
            );
        }
        else
        {
            error_intl(scp, i18n("mtime(\"$filename\") = $number (reason)"));
        }
    }
    trace(("}\n"));
}


void
stat_cache_clear(string_ty *path)
{
    trace(("stat_cache_clear(path =\"%s\")\n{\n", path->str_text));
    if (symtab[0])
        symtab_delete(symtab[0], path);
    if (symtab[1])
        symtab_delete(symtab[1], path);
    trace(("}\n"));
}


static void
dumper(symtab_ty *stp, string_ty *key, void *data, void *arg)
{
    struct stat     st;

    (void)stp;
    (void)data;
    if (stat(key->str_text, &st) >= 0 && S_ISREG(st.st_mode))
    {
        FILE            *fp;

        fp = arg;
        fprintf(fp, "%lld %s\n", (long long)st.st_size, key->str_text);
    }
}


void
stat_cache_dump(void)
{
    if (!option_test(OPTION_FILE_SIZE_STATISTICS))
        return;
    if (!symtab[1])
        return;
    FILE *fp = fopen("file-size-statistics.txt", "w");
    if (!fp)
        return;
    symtab_walk(symtab[1], dumper, fp);
    fclose(fp);
}

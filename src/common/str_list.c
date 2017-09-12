/*
 *      cook - file construction tool
 *      Copyright (C) 1992-2007 Peter Miller
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
 *
 * This file contains routines for mainpulating words and word lists.
 * Much of the functionality of cook uses these routines.
 */

#include <common/ac/ctype.h>
#include <common/ac/stddef.h>
#include <common/ac/string.h>
#include <common/ac/stdlib.h>
#include <common/ac/time.h>

#include <common/mem.h>
#include <common/str.h>
#include <common/str_list.h>
#include <common/trace.h>       /* for assert */


/*
 * NAME
 *      string_list_append - append to a word list
 *
 * SYNOPSIS
 *      void string_list_append(string_list_ty *wlp, string_ty *wp);
 *
 * DESCRIPTION
 *      Wl_append is used to append to a word list.
 *
 * CAVEAT
 *      The word being appended IS copied.
 */

void
string_list_append(string_list_ty *wlp, string_ty *w)
{
    assert(wlp);
    assert(w);
    if (wlp->nstrings >= wlp->nstrings_max)
    {
        size_t          nbytes;

        wlp->nstrings_max = wlp->nstrings_max * 2 + 16;
        nbytes = wlp->nstrings_max * sizeof(string_ty *);
        wlp->string = mem_change_size(wlp->string, nbytes);
    }
    wlp->string[wlp->nstrings++] = str_copy(w);
}


/*
 * NAME
 *      string_list_append_list
 *
 * SYNOPSIS
 *      void string_list_append_list(string_list_ty *to, string_list_ty *from);
 *
 * DESCRIPTION
 *      The string_list_append_list function is used to append one
 *      string list (from) onto the end of another (to).
 */

void
string_list_append_list(string_list_ty *to, const string_list_ty *from)
{
    size_t          j;

    for (j = 0; j < from->nstrings; ++j)
        string_list_append(to, from->string[j]);
}


/*
 * NAME
 *      string_list_prepend
 *
 * SYNOPSIS
 *      void string_list_prepend(string_list_ty *, string_ty *);
 *
 * DESCRIPTION
 *      The string_list_prepend function is used to insert a string at
 *      the beginning of a string list.
 */

void
string_list_prepend(string_list_ty *wlp, string_ty *w)
{
    ptrdiff_t       j;

    assert(wlp);
    assert(w);
    if (wlp->nstrings >= wlp->nstrings_max)
    {
        size_t          nbytes;

        wlp->nstrings_max = wlp->nstrings_max * 2 + 16;
        nbytes = wlp->nstrings_max * sizeof(string_ty *);
        wlp->string = mem_change_size(wlp->string, nbytes);
    }
    wlp->nstrings++;
    for (j = wlp->nstrings - 1; j > 0; --j)
        wlp->string[j] = wlp->string[j - 1];
    wlp->string[0] = str_copy(w);
}


/*
 * NAME
 *      string_list_destructor - free a word list
 *
 * SYNOPSIS
 *      void string_list_destructor(string_list_ty *wlp);
 *
 * DESCRIPTION
 *      Wl_free is used to free the contents of a word list
 *      when it is finished with.
 *
 * CAVEAT
 *      It is assumed that the contents of the word list were all
 *      created using strdup() or similar, and grown using string_list_append().
 */

void
string_list_destructor(string_list_ty *wlp)
{
    size_t          j;

    for (j = 0; j < wlp->nstrings; j++)
        str_free(wlp->string[j]);
    if (wlp->string)
        mem_free(wlp->string);
    wlp->nstrings = 0;
    wlp->nstrings_max = 0;
    wlp->string = 0;
}


/*
 * NAME
 *      string_list_member - word list membership
 *
 * SYNOPSIS
 *      int string_list_member(string_list_ty *wlp, string_ty *wp);
 *
 * DESCRIPTION
 *      Wl_member is used to determine if the given word is
 *      contained in the given word list.
 *
 * RETURNS
 *      A zero if the word is not in the list,
 *      and a non-zero if it is.
 */

int
string_list_member(const string_list_ty *wlp, string_ty *w)
{
    size_t          j;

    for (j = 0; j < wlp->nstrings; j++)
        if (str_equal(wlp->string[j], w))
            return 1;
    return 0;
}


/*
 * NAME
 *      string_list_intersect - word list intersection test
 *
 * SYNOPSIS
 *      int string_list_intersect(string_list_ty *wlp, string_list_ty *wp);
 *
 * DESCRIPTION
 *      Wl_intersect is used to determine if the given word is
 *      contained in the given word list.
 *
 * RETURNS
 *      A zero if the word is not in the list,
 *      and a non-zero if it is.
 */

int
string_list_intersect(const string_list_ty *wl1, const string_list_ty *wl2)
{
    size_t          j;

    if (wl1->nstrings > wl2->nstrings)
    {
        for (j = 0; j < wl2->nstrings; j++)
            if (string_list_member(wl1, wl2->string[j]))
                return 1;
    }
    else
    {
        for (j = 0; j < wl1->nstrings; j++)
            if (string_list_member(wl2, wl1->string[j]))
                return 1;
    }
    return 0;
}


/*
 * NAME
 *      string_list_copy_constructor - copy a word list
 *
 * SYNOPSIS
 *      void string_list_copy_constructor(string_list_ty *to,
 *              string_list_ty *from);
 *
 * DESCRIPTION
 *      Wl_copy is used to copy word lists.
 *
 * RETURNS
 *      A copy of the 'to' word list is placed in 'from'.
 *
 * CAVEAT
 *      It is the responsibility of the caller to ensure that the
 *      new word list is freed when finished with, by a call to
 *      string_list_destructor().
 */

void
string_list_copy_constructor(string_list_ty *to, const string_list_ty *from)
{
    size_t          j;

    string_list_constructor(to);
    for (j = 0; j < from->nstrings; j++)
        string_list_append(to, str_copy(from->string[j]));
}


/*
 * NAME
 *      wl2str_respect_empty - form a string from a word list
 *
 * SYNOPSIS
 *      string_ty *wl2str_respect_empty(string_list_ty *wlp, int start,
 *              int stop, char *sep, int empty);
 *
 * DESCRIPTION
 *      Wl2str is used to form a string from a word list.  The ``empty''
 *      argument is true if empty strings should be honoured; if false
 *      empty strings will be ignored.
 *
 * RETURNS
 *      A pointer to the newly formed string in dynamic memory.
 *
 * CAVEAT
 *      It is the responsibility of the caller to ensure that the
 *      new string is freed when finished with, by a call to str_free().
 */

string_ty *
wl2str_respect_empty(const string_list_ty *wl, int start, int stop,
    const char *sep, int empty)
{
    int             j;
    static char     *tmp;
    static size_t   tmplen;
    size_t          length;
    size_t          seplen;
    char            *pos;
    string_ty       *s;

    if (!sep)
        sep = " ";
    seplen = strlen(sep);
    length = 0;
    for (j = start; j <= stop && j < (int)(wl->nstrings); j++)
    {
        s = wl->string[j];
        if (s->str_length || empty)
        {
            if (length > 0 || (empty && j > start))
                length += seplen;
            length += s->str_length;
        }
    }

    if (tmplen < length)
    {
        tmplen = length;
        tmp = mem_change_size(tmp, tmplen);
    }

    pos = tmp;
    for (j = start; j <= stop && j < (int)(wl->nstrings); j++)
    {
        s = wl->string[j];
        if (s->str_length || empty)
        {
            if (seplen && (pos != tmp || (empty && j > start)))
            {
                memcpy(pos, sep, seplen);
                pos += seplen;
            }
            if (s->str_length > 0)
            {
                memcpy(pos, s->str_text, s->str_length);
                pos += s->str_length;
            }
        }
    }

    s = str_n_from_c(tmp, length);
    return s;
}


/*
 * NAME
 *      wl2str - form a string from a word list
 *
 * SYNOPSIS
 *      string_ty *wl2str(string_list_ty *wlp, int start, int stop, char *sep);
 *
 * DESCRIPTION
 *      Wl2str is used to form a string from a word list.
 *
 * RETURNS
 *      A pointer to the newly formed string in dynamic memory.
 *
 * CAVEAT
 *      It is the responsibility of the caller to ensure that the
 *      new string is freed when finished with, by a call to str_free().
 */

string_ty *
wl2str(const string_list_ty *wl, int start, int stop, const char *sep)
{
    return wl2str_respect_empty(wl, start, stop, sep, 0);
}


/*
 * NAME
 *      str2wl - string to word list
 *
 * SYNOPSIS
 *      void str2wl(string_list_ty *wlp, string_ty *s, char *sep, int ewhite);
 *
 * DESCRIPTION
 *      Str2wl is used to form a word list from a string.
 *      wlp     - where to put the word list
 *      s       - string to break
 *      sep     - separators, default to " " if 0 given
 *      ewhite  - supress extra white space around separators
 *
 * RETURNS
 *      The string is broken on spaces into words,
 *      using strndup() and string_list_append().
 *
 * CAVEAT
 *      Quoting is not understood.
 */

void
str2wl(string_list_ty *slp, string_ty *s, char *sep, int ewhite)
{
    char            *cp;
    int             more;

    if (!sep)
    {
        sep = " \t\n\f\r";
        ewhite = 1;
    }
    string_list_constructor(slp);
    cp = s->str_text;
    more = 0;
    while (*cp || more)
    {
        string_ty       *w;
        char            *cp1;
        char            *cp2;

        if (ewhite)
            while (isspace(*cp))
                cp++;
        if (!*cp && !more)
            break;
        more = 0;
        cp1 = cp;
        while (*cp && !strchr(sep, *cp))
            cp++;
        if (*cp)
        {
            cp2 = cp + 1;
            more = 1;
        }
        else
            cp2 = cp;
        if (ewhite)
            while (cp > cp1 && isspace(cp[-1]))
                cp--;
        w = str_n_from_c(cp1, cp - cp1);
        string_list_append(slp, w);
        str_free(w);
        cp = cp2;
    }
}


/*
 * NAME
 *      wl_insert - a insert a word into a list
 *
 * SYNOPSIS
 *      void wl_insert(string_list_ty *wlp, string_ty *wp);
 *
 * DESCRIPTION
 *      Wl_insert is similar to string_list_append, however it does not
 *      append the word unless it is not already in the list.
 *
 * CAVEAT
 *      If the word is inserted it is copied.
 */

void
string_list_append_unique(string_list_ty *wlp, string_ty *wp)
{
    size_t          j;

    for (j = 0; j < wlp->nstrings; j++)
        if (str_equal(wlp->string[j], wp))
            return;
    string_list_append(wlp, wp);
}


/*
 * NAME
 *      string_list_append_list_unique
 *
 * SYNOPSIS
 *      void string_list_append_list_unique(string_list_ty *to,
 *              string_list_ty *from);
 *
 * DESCRIPTION
 *      The string_list_append_list_unique function is used to append
 *      the contents of one string list (from) to the end of another
 *      tring list (to).  Entries which duplicate items already present
 *      will be ignored.
 */

void
string_list_append_list_unique(string_list_ty *to, const string_list_ty *from)
{
    size_t          j;

    for (j = 0; j < from->nstrings; ++j)
        string_list_append_unique(to, from->string[j]);
}


/*
 * NAME
 *      string_list_remove - remove list member
 *
 * SYNOPSIS
 *      void string_list_remove(string_list_ty *wlp, string_ty *wp);
 *
 * DESCRIPTION
 *      The string_list_remove function is used to delete a member of
 *      a word list.
 *
 * RETURNS
 *      void
 */

void
string_list_remove(string_list_ty *wlp, string_ty *wp)
{
    size_t          j;
    size_t          k;

    for (j = 0; j < wlp->nstrings; ++j)
    {
        if (str_equal(wlp->string[j], wp))
        {
            wlp->nstrings--;
            for (k = j; k < wlp->nstrings; ++k)
                wlp->string[k] = wlp->string[k + 1];
            str_free(wp);
            break;
        }
    }
}


/*
 * NAME
 *      string_list_remove_list - remove list members
 *
 * SYNOPSIS
 *      void string_list_remove_list(string_list_ty *wlp, string_list_ty *wp);
 *
 * DESCRIPTION
 *      The string_list_remove_list function is used to delete all the
 *      members of a word list from another word list.
 *
 * RETURNS
 *      void
 */

void
string_list_remove_list(string_list_ty *wlp, const string_list_ty *nuke)
{
    size_t          j;

    for (j = 0; j < nuke->nstrings; ++j)
        string_list_remove(wlp, nuke->string[j]);
}


/*
 * NAME
 *      string_list_constructor
 *
 * SYNOPSIS
 *      void string_list_constructor(string_list_ty *);
 *
 * DESCRIPTION
 *      The string_list_constructor function is used to prepare a string
 *      list for use.  It will be empty.
 *
 * CAVEAT
 *      This must be called on the string list before any other action
 *      is taken.  Use string_list_destructor when you are done.
 */

void
string_list_constructor(string_list_ty *wlp)
{
    wlp->nstrings = 0;
    wlp->nstrings_max = 0;
    wlp->string = 0;
}


/*
 * NAME
 *      string_list_new
 *
 * DESCRIPTION
 *      string_list_ty *string_list_new(void);
 *
 * DESCRIPTION
 *      The string_list_new function is used to allocate a new  string
 *      list in dynamic memory.  It will be empty.
 *
 * RETURNS
 *      string_list_ty *
 *
 * CAVEAT
 *      Use string_list_delete when you are done.
 */

string_list_ty *
string_list_new(void)
{
    string_list_ty  *slp;

    slp = mem_alloc(sizeof(string_list_ty));
    string_list_constructor(slp);
    return slp;
}


/*
 * NAME
 *      string_list_new_copy
 *
 * SYNOPSIS
 *      string_list_ty *string_list_new_copy(string_list_ty *);
 *
 * DESCRIPTION
 *      The string_list_new_copy function is used to allocate a new copy
 *      of a string list in dynamic memory.
 *
 * RETURNS
 *      string_list_ty *
 *
 * CAVEAT
 *      Use string_list_delete when you are done.
 */

string_list_ty *
string_list_new_copy(const string_list_ty *from)
{
    string_list_ty  *slp;

    slp = mem_alloc(sizeof(string_list_ty));
    string_list_copy_constructor(slp, from);
    return slp;
    trace(("to silence warnings\n"));
}


/*
 * NAME
 *      string_list_delete
 *
 * SYNOPSIS
 *      void string_list_delete(string_list_ty *);
 *
 * DESCRIPTION
 *      The string_list_delete function is used to release the resources
 *      held by a string list in dynamic memory.
 */

void
string_list_delete(string_list_ty *slp)
{
    string_list_destructor(slp);
    mem_free(slp);
}


/*
 * NAME
 *      string_list_bool
 *
 * SYNOPSIS
 *      int string_list_bool(string_list_ty *);
 *
 * DESCRIPTION
 *      The string_list_bool function is used to perform a boolean
 *      evaluation on a list of strings.  If any return str_bool of
 *      true, the list is true.
 */

int
string_list_bool(const string_list_ty *slp)
{
    size_t          j;

    for (j = 0; j < slp->nstrings; ++j)
        if (str_bool(slp->string[j]))
            return 1;
    return 0;
}


static int
cmp(const void *va, const void *vb)
{
    string_ty       *a;
    string_ty       *b;

    a = *(string_ty **)va;
    b = *(string_ty **)vb;
    return strcmp(a->str_text, b->str_text);
}


void
string_list_sort(string_list_ty *slp)
{
    qsort(slp->string, slp->nstrings, sizeof(slp->string[0]), cmp);
}

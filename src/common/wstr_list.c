/*
 *      cook - file construction tool
 *      Copyright (C) 1997, 2001, 2006, 2007 Peter Miller;
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

#include <common/ac/string.h>

#include <common/wstr_list.h>
#include <common/mem.h>


/*
 * NAME
 *      wstring_list_append - append to a wide string list
 *
 * SYNOPSIS
 *      void wstring_list_append(wstring_list_ty *wlp, wstring_ty *wp);
 *
 * DESCRIPTION
 *      The wstring_list_append function is used to append to a wide
 *      string list.
 *
 * CAVEAT
 *      The wide string being appended IS copied.
 */

void
wstring_list_append(wstring_list_ty *wlp, wstring_ty *w)
{
    size_t          nbytes;

    if (wlp->nitems >= wlp->nitems_max)
    {
        /*
         * always 8 less than a power of 2, which is
         * most efficient for many memory allocators
         */
        wlp->nitems_max = wlp->nitems_max * 2 + 8;
        nbytes = wlp->nitems_max * sizeof(wstring_ty *);
        wlp->item = mem_change_size(wlp->item, nbytes);
    }
    wlp->item[wlp->nitems++] = wstr_copy(w);
}


void
wstring_list_prepend(wstring_list_ty *wlp, wstring_ty *w)
{
    size_t          nbytes;
    size_t          j;

    if (wlp->nitems >= wlp->nitems_max)
    {
        /*
         * always 8 less than a power of 2, which is
         * most efficient for many memory allocators
         */
        wlp->nitems_max = wlp->nitems_max * 2 + 8;
        nbytes = wlp->nitems_max * sizeof(wstring_ty *);
        wlp->item = mem_change_size(wlp->item, nbytes);
    }
    for (j = wlp->nitems; j > 0; --j)
        wlp->item[j] = wlp->item[j - 1];
    wlp->nitems++;
    wlp->item[0] = wstr_copy(w);
}


/*
 * NAME
 *      wstring_list_free - free a wide string list
 *
 * SYNOPSIS
 *      void wstring_list_free(wstring_list_ty *wlp);
 *
 * DESCRIPTION
 *      The wstring_list_free function is used to free the contents of a
 *      wide string list when it is finished with.
 *
 * CAVEAT
 *      It is assumed that the contents of the wide string list were all
 *      created using strdup() or similar, and grown using
 *      wstring_list_append().
 */

void
wstring_list_free(wstring_list_ty *wlp)
{
    size_t          j;

    for (j = 0; j < wlp->nitems; j++)
        wstr_free(wlp->item[j]);
    if (wlp->item)
        mem_free(wlp->item);
    wlp->nitems = 0;
    wlp->nitems_max = 0;
    wlp->item = 0;
}


/*
 * NAME
 *      wstring_list_member - wide string list membership
 *
 * SYNOPSIS
 *      int wstring_list_member(wstring_list_ty *wlp, wstring_ty *wp);
 *
 * DESCRIPTION
 *      The wstring_list_member function is used to determine if the
 *      given wide string is contained in the given wide string list.
 *
 * RETURNS
 *      A zero if the wide string is not in the list, and a non-zero if it
 *      is.
 */

int
wstring_list_member(wstring_list_ty *wlp, wstring_ty *w)
{
    size_t          j;

    for (j = 0; j < wlp->nitems; j++)
        if (wstr_equal(wlp->item[j], w))
            return 1;
    return 0;
}


/*
 * NAME
 *      wstring_list_copy - copy a wide string list
 *
 * SYNOPSIS
 *      void wstring_list_copy(wstring_list_ty *to, wstring_list_ty *from);
 *
 * DESCRIPTION
 *      The wstring_listl_copy function is used to copy wide string lists.
 *
 * RETURNS
 *      A copy of the 'to' wide string list is placed in 'from'.
 *
 * CAVEAT
 *      It is the responsibility of the caller to ensure that the new
 *      wide string list is freed when finished with, by a call to
 *      wstring_list_free().
 */

void
wstring_list_copy(wstring_list_ty *to, wstring_list_ty *from)
{
    size_t          j;

    wstring_list_zero(to);
    for (j = 0; j < from->nitems; j++)
        wstring_list_append(to, wstr_copy(from->item[j]));
}


/*
 * NAME
 *      wstring_list_to_wstring - form a string from a wide string list
 *
 * SYNOPSIS
 *      wstring_ty *wstring_list_to_wstring(wstring_list_ty *wlp, int start,
 *              int stop, char *sep);
 *
 * DESCRIPTION
 *      The wstring_list_to_wstring function is used to form a string
 *      from a wide string list.
 *
 * RETURNS
 *      A pointer to the newly formed string in dynamic memory.
 *
 * CAVEAT
 *      It is the responsibility of the caller to ensure that the
 *      new string is freed when finished with, by a call to free().
 */

wstring_ty *
wstring_list_to_wstring(wstring_list_ty *wl, int start, int stop, char *sep)
{
    size_t          j;
    size_t          k;
    static wchar_t  *tmp;
    static size_t   tmplen;
    size_t          length;
    size_t          seplen;
    wchar_t         *pos;
    wstring_ty      *s;

    if (!sep)
        sep = " ";
    seplen = strlen(sep);
    length = 0;
    for (j = start; (int)j <= stop && j < wl->nitems; ++j)
    {
        s = wl->item[j];
        if (s->wstr_length)
        {
            if (length)
                length += seplen;
            length += s->wstr_length;
        }
    }

    if (tmplen < length)
    {
        tmplen = length;
        tmp = mem_change_size(tmp, tmplen * sizeof(wchar_t));
    }

    pos = tmp;
    for (j = start; (int)j <= stop && j < wl->nitems; j++)
    {
        s = wl->item[j];
        if (s->wstr_length)
        {
            if (pos != tmp)
            {
                for (k = 0; k < seplen; ++k)
                    *pos++ = sep[k];
            }
            memcpy(pos, s->wstr_text, s->wstr_length * sizeof(wchar_t));
            pos += s->wstr_length;
        }
    }

    s = wstr_n_from_wc(tmp, length);
    return s;
}


static int
wc_find(char *s, wchar_t c)
{
    while (*s)
    {
        if (*s == c)
            return 1;
        ++s;
    }
    return 0;
}


/*
 * NAME
 *      wstring_to_wstring_list - string to wide string list
 *
 * SYNOPSIS
 *      void wstring_to_wstring_list(wstring_list_ty *wlp, wstring_ty *s,
 *              char *sep, int ewhite);
 *
 * DESCRIPTION
 *      The wstring_to_wstring_list function is used to form a wide string
 *      list from a string.
 *
 * ARGUMENTS
 *      wlp     - where to put the wide string list
 *      s       - string to break
 *      sep     - separators, default to " " if 0 given
 *      ewhite  - supress extra white space around separators
 *
 * RETURNS
 *      The string is broken on spaces into words,
 *      using strndup() and wstring_list_append().
 *
 * CAVEAT
 *      Quoting is not understood.
 */

void
wstring_to_wstring_list(wstring_list_ty *slp, wstring_ty *s, char *sep,
    int ewhite)
{
    static char     white[] = " \t\n\f\r";
    wchar_t         *cp;
    int             more;

    if (!sep)
    {
        sep = white;
        ewhite = 1;
    }
    wstring_list_zero(slp);
    cp = s->wstr_text;
    more = 0;
    while (*cp || more)
    {
        wstring_ty      *w;
        wchar_t         *cp1;
        wchar_t         *cp2;

        if (ewhite)
            while (wc_find(white, *cp))
                cp++;
        if (!*cp && !more)
            break;
        more = 0;
        cp1 = cp;
        while (*cp && !wc_find(sep, *cp))
            cp++;
        if (*cp)
        {
            cp2 = cp + 1;
            more = 1;
        }
        else
            cp2 = cp;
        if (ewhite)
            while (cp > cp1 && wc_find(white, cp[-1]))
                cp--;
        w = wstr_n_from_wc(cp1, cp - cp1);
        wstring_list_append(slp, w);
        wstr_free(w);
        cp = cp2;
    }
}


/*
 * NAME
 *      wstring_list_insert - a insert a wide string into a list
 *
 * SYNOPSIS
 *      void wstring_list_insert(wstring_list_ty *wlp, wstring_ty *wp);
 *
 * DESCRIPTION
 *      The wstring_list_insert function is similar to the
 *      wstring_list_append function, however it does not append the
 *      wide string unless it is not already in the list.
 *
 * CAVEAT
 *      If the wide string is inserted it is copied.
 */

void
wstring_list_append_unique(wstring_list_ty *wlp, wstring_ty *wp)
{
    size_t          j;

    for (j = 0; j < wlp->nitems; j++)
        if (wstr_equal(wlp->item[j], wp))
            return;
    wstring_list_append(wlp, wp);
}


/*
 * NAME
 *      wstring_list_delete - remove list member
 *
 * SYNOPSIS
 *      void wstring_list_delete(wstring_list_ty *wlp, wstring_ty *wp);
 *
 * DESCRIPTION
 *      The wstring_list_delete function is used to delete a member of a
 *      wide string list.
 *
 * RETURNS
 *      void
 */

void
wstring_list_delete(wstring_list_ty *wlp, wstring_ty *wp)
{
    size_t          j;
    size_t          k;

    for (j = 0; j < wlp->nitems; ++j)
    {
        if (wstr_equal(wlp->item[j], wp))
        {
            wlp->nitems--;
            for (k = j; k < wlp->nitems; ++k)
                wlp->item[k] = wlp->item[k + 1];
            wstr_free(wp);
            break;
        }
    }
}


void
wstring_list_zero(wstring_list_ty *wlp)
{
    wlp->nitems = 0;
    wlp->nitems_max = 0;
    wlp->item = 0;
}


int
wstring_list_equal(wstring_list_ty *a, wstring_list_ty *b)
{
    size_t          j,
                    k;

    for (j = 0; j < a->nitems; ++j)
    {
        for (k = 0; k < b->nitems; ++k)
            if (wstr_equal(a->item[j], b->item[k]))
                break;
        if (k >= b->nitems)
            return 0;
    }
    for (j = 0; j < b->nitems; ++j)
    {
        for (k = 0; k < a->nitems; ++k)
            if (wstr_equal(b->item[j], a->item[k]))
                break;
        if (k >= a->nitems)
            return 0;
    }
    return 1;
}


int
wstring_list_subset(wstring_list_ty *a, wstring_list_ty *b)
{
    size_t          j,
                    k;

    /*
     * test if "a is a subset of b"
     */
    if (a->nitems > b->nitems)
        return 0;
    for (j = 0; j < a->nitems; ++j)
    {
        for (k = 0; k < b->nitems; ++k)
            if (wstr_equal(a->item[j], b->item[k]))
                break;
        if (k >= b->nitems)
            return 0;
    }
    return 1;
}

/*
 *      cook - file construction tool
 *      Copyright (C) 1997, 1999, 2001, 2006-2009 Peter Miller
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
 *
 * A literal pool is maintained.  Each string has a reference count.  The
 * string stays in the literal pool for as long as it has a positive
 * reference count.  To determine if a string is already in the literal pool,
 * linear dynamic hashing is used to guarantee an O(1) search.  Making all equal
 * strings the same item in the literal pool means that string equality is
 * a pointer test, and thus very fast.
 */

#include <common/ac/limits.h>
#include <common/ac/stdarg.h>
#include <common/ac/stddef.h>
#include <common/ac/stdio.h>
#include <common/ac/stdlib.h>
#include <common/ac/string.h>
#include <common/ac/wchar.h>
#include <common/ac/wctype.h>

#include <common/error.h>
#include <common/language.h>
#include <common/mem.h>
#include <common/str.h>
#include <common/trace.h>       /* for assert */
#include <common/wstr.h>


static wstring_ty **hash_table;
static wstr_hash_ty hash_modulus;
static wstr_hash_ty hash_cutover;
static wstr_hash_ty hash_cutover_mask;
static wstr_hash_ty hash_cutover_split_mask;
static wstr_hash_ty hash_split;
static wstr_hash_ty hash_load;
static int      changed;

#define MAX_HASH_LEN 20


/*
 * NAME
 *      hash_generate - hash string to number
 *
 * SYNOPSIS
 *      wstr_hash_ty hash_generate(wchar_t *s, size_t n);
 *
 * DESCRIPTION
 *      The hash_generate function is used to make a number from a string.
 *
 * RETURNS
 *      wstr_hash_ty - the magic number
 *
 * CAVEAT
 *      Only the last MAX_HASH_LEN characters are used.
 *      It is important that wstr_hash_ty be unsigned (int or long).
 */

static wstr_hash_ty
hash_generate(const wchar_t *s, size_t n)
{
    wstr_hash_ty    retval;

    if (n > MAX_HASH_LEN)
    {
        s += n - MAX_HASH_LEN;
        n = MAX_HASH_LEN;
    }

    retval = 0;
    while (n > 0)
    {
        retval = (retval + (retval << 1)) ^ *s++;
        --n;
    }
    return retval;
}


/*
 * NAME
 *      wstr_initialize - start up string table
 *
 * SYNOPSIS
 *      void wstr_initialize(void);
 *
 * DESCRIPTION
 *      The wstr_initialize function is used to create the hash table and
 *      initialize it to empty.
 *
 * RETURNS
 *      void
 *
 * CAVEAT
 *      This function must be called before any other defined in this file.
 */

static void
wstr_initialize(void)
{
    wstr_hash_ty    j;

    if (hash_modulus)
        return;
    hash_modulus = 1 << 8;      /* MUST be a power of 2 */
    hash_cutover = hash_modulus;
    hash_split = hash_modulus - hash_cutover;
    hash_cutover_mask = hash_cutover - 1;
    hash_cutover_split_mask = (hash_cutover * 2) - 1;
    hash_load = 0;
    hash_table = (wstring_ty **)mem_alloc(hash_modulus * sizeof(wstring_ty *));
    for (j = 0; j < hash_modulus; ++j)
        hash_table[j] = 0;
}


/*
 * NAME
 *      split - reduce table loading
 *
 * SYNOPSIS
 *      void split(void);
 *
 * DESCRIPTION
 *      The split function is used to reduce the load factor on the hash table.
 *
 * RETURNS
 *      void
 *
 * CAVEAT
 *      A load factor of about 80% is suggested.
 */

static void
split(void)
{
    wstring_ty      *p;
    wstring_ty      *p2;
    wstr_hash_ty    idx;

    /*
     * get the list to be split across buckets
     */
    p = hash_table[hash_split];
    hash_table[hash_split] = 0;

    /*
     * increase the modulus by one
     */
    hash_modulus++;
    hash_table =
        mem_change_size(hash_table, hash_modulus * sizeof(wstring_ty *));
    hash_table[hash_modulus - 1] = 0;
    hash_split = hash_modulus - hash_cutover;
    if (hash_split >= hash_cutover)
    {
        hash_cutover = hash_modulus;
        hash_split = 0;
        hash_cutover_mask = hash_cutover - 1;
        hash_cutover_split_mask = (hash_cutover * 2) - 1;
    }

    /*
     * now redistribute the list elements
     */
    while (p)
    {
        p2 = p;
        p = p->wstr_next;

        idx = p2->wstr_hash & hash_cutover_mask;
        if (idx < hash_split)
            idx = p2->wstr_hash & hash_cutover_split_mask;
        p2->wstr_next = hash_table[idx];
        hash_table[idx] = p2;
    }
}


/*
 * NAME
 *      wstr_from_c - make string from C string
 *
 * SYNOPSIS
 *      wstring_ty *wstr_from_c(char *);
 *
 * DESCRIPTION
 *      The wstr_from_c function is used to make a string from a NUL
 *      terminated C string.  The conversion from multi-byte to wide
 *      characters is done in the current locale.
 *
 * RETURNS
 *      wstring_ty* - a pointer to a string in dynamic memory.  Use
 *      wstr_free when finished with.
 *
 * CAVEAT
 *      The contents of the structure pointed to MUST NOT be altered.
 */

wstring_ty *
wstr_from_c(const char *s)
{
    return wstr_n_from_c(s, strlen(s));
}


/*
 * NAME
 *      wstr_from_wc - make string from a wide C string
 *
 * SYNOPSIS
 *      wstring_ty *wstr_from_wc(wchar_t *);
 *
 * DESCRIPTION
 *      The wstr_from_c function is used to make a string from a NUL
 *      terminated wide C string.
 *
 * RETURNS
 *      wstring_ty* - a pointer to a string in dynamic memory.  Use
 *      wstr_free when finished with.
 *
 * CAVEAT
 *      The contents of the structure pointed to MUST NOT be altered.
 */

wstring_ty *
wstr_from_wc(const wchar_t *ws)
{
    return wstr_n_from_wc(ws, wcslen(ws));
}


/*
 * NAME
 *      wstr_n_from_c - make string
 *
 * SYNOPSIS
 *      wstring_ty *wstr_n_from_c(char *s, size_t n);
 *
 * DESCRIPTION
 *      The wstr_n_from_c function is used to make a string from an
 *      array of characters.  No NUL terminator is assumed.  The
 *      conversion from muti-byte to wide characters is done in the
 *      current locale.
 *
 * RETURNS
 *      wstring_ty* - a pointer to a string in dynamic memory.  Use
 *      wstr_free when finished with.
 *
 * CAVEAT
 *      The contents of the structure pointed to MUST NOT be altered.
 */

wstring_ty *
wstr_n_from_c(const char *s, size_t length)
{
    static char     escapes[] = "\aa\bb\ff\nn\rr\tt\vv";
    static wchar_t  *buf;
    static size_t   bufmax;
    size_t          remainder;
    const char      *ip;
    wchar_t         *op;

    /*
     * Do the conversion "long hand".  This is because some
     * implementations of the mbstowcs function barf when they see
     * invalid multi byte character sequences.  This function
     * renders them as C escape sequences and keeps going.
     */
    if (bufmax < length)
    {
        bufmax = length;
        /* the 4 is the longest escape sequence */
        buf = mem_change_size(buf, bufmax * sizeof(wchar_t) * 4);
    }

    /*
     * change the locale to the native language default
     */
    language_human();

    /*
     * Reset the mbtowc internal state.
     */
    if (mbtowc((wchar_t *) 0, (char *)0, 0) == -1)
    {
        assert(!"assume success");
    }

    /*
     * scan the string and extract the wide characters
     */
    ip = s;
    op = buf;
    remainder = length;
    while (remainder > 0)
    {
        int             n;

        n = mbtowc(op, ip, remainder);
        if (n == 0)
            break;
        if (n < 0)
        {
            char            *esc;

            /*
             * Invalid multi byte sequence, replace the
             * first character with a C escape sequence.
             */
            esc = strchr(escapes, *ip);
            if (esc)
            {
                *op++ = '\\';
                *op++ = esc[1];
            }
            else
            {
                *op++ = '\\';
                *op++ = '0' + ((*ip >> 6) & 7);
                *op++ = '0' + ((*ip >> 3) & 7);
                *op++ = '0' + (*ip & 7);
            }
            ++ip;
            --remainder;

            /*
             * The mbtowc function's internal state will now
             * be "error" or broken, or otherwise useless.
             * Reset it so that we can keep going.
             */
            if (mbtowc((wchar_t *) 0, (char *)0, 0) == -1)
            {
                assert(!"assume success");
            }
        }
        else
        {
            /*
             * the one wchar_t used n chars
             */
            ip += n;
            remainder -= n;
            ++op;
        }
    }

    /*
     * change the locale back to the C locale
     */
    language_C();

    /*
     * build the result from the image in ``buf''
     */
    return wstr_n_from_wc(buf, op - buf);
}


/*
 * NAME
 *      wstr_to_mbs - wide string to multi-byte C string
 *
 * SYNOPSIS
 *      void wstr_to_mbs(wstring_ty *s, char **rslt, size_t *rslt_len);
 *
 * DESCRIPTION
 *      The wstr_to_mbs function convers a wide character string into a
 *      multi-byte C string.  The conversion is done in the current
 *      locale.  The result is NUL terminated, however the result length
 *      does not include the NUL.
 *
 * CAVEAT
 *      DO NOT free the result.  The result will change between calls,
 *      so copy it if you need to keep it.
 */

void
wstr_to_mbs(wstring_ty *s, char **result_p, size_t *result_length_p)
{
    static char     *buf;
    static size_t   bufmax;
    int             n;
    wchar_t         *ip;
    size_t          remainder;
    char            *op;
    size_t          buflen;

    /*
     * Do the conversion "long hand".  This is because the wcstombs
     * function barfs when it sees an invalid wchar_t.  This
     * function treats them literally and keeps going.
     *
     * MB_LEN_MAX is used because the standard guarantees that it is
     * a non-zero constant.  MB_CUR_MAX is an expression, and
     * experience has shown (e.g. SunOS 4.1.3) this it may be zero!
     */
    buflen = (s->wstr_length + 1) * MB_LEN_MAX;
    assert(buflen > 0);
    if (buflen > bufmax)
    {
        bufmax = buflen;
        buf = mem_change_size(buf, bufmax);
    }

    /*
     * perform the conversion in the native language default
     */
    language_human();

    /*
     * The wctomb function has internal state.  It needs to be reset.
     */
    if (wctomb((char *)0, (wchar_t) 0) == -1)
    {
        assert(!"assume success");
    }

    ip = s->wstr_text;
    remainder = s->wstr_length;
    op = buf;
    while (remainder > 0)
    {
        n = wctomb(op, *ip);
        if (n <= 0)
        {
            /*
             * Copy the character literally.
             * Throw away anything that will not fit.
             */
            *op++ = *ip++;
            if (!op[-1])
                op[-1] = '?';
            --remainder;

            /*
             * The wctomb function's internal state will now
             * be "error" or broken, or otherwise useless.
             * Reset it so that we can keep going.
             */
            if (wctomb((char *)0, (wchar_t) 0) == -1)
            {
                assert(!"assume success");
            }
        }
        else
        {
            op += n;
            ++ip;
            --remainder;
        }
    }

    /*
     * The final NUL could require shift state end characters,
     * meaning that n could be more than 1.
     */
    n = wctomb(op, (wchar_t) 0);
    if (n <= 0)
        *op = 0;
    else
    {
        op += n - 1;
        assert(*op == 0);
    }

    /*
     * restore the locale to the C locale
     */
    language_C();

    /*
     * set the output side effects
     */
    *result_p = buf;
    *result_length_p = op - buf;
}


/*
 * NAME
 *      wstr_n_from_wc - make string
 *
 * SYNOPSIS
 *      wstring_ty *wstr_n_from_wc(wchar_t *s, size_t n);
 *
 * DESCRIPTION
 *      The wstr_n_from_c function is used to make a string from an
 *      array of wide characters.  No NUL terminator is assumed.
 *
 * RETURNS
 *      wstring_ty* - a pointer to a string in dynamic memory.  Use
 *      wstr_free when finished with.
 *
 * CAVEAT
 *      The contents of the structure pointed to MUST NOT be altered.
 */

wstring_ty *
wstr_n_from_wc(const wchar_t *s, size_t length)
{
    wstr_hash_ty    hash;
    wstr_hash_ty    idx;
    wstring_ty      *p;

    if (!hash_modulus)
        wstr_initialize();
    hash = hash_generate(s, length);

    idx = hash & hash_cutover_mask;
    if (idx < hash_split)
        idx = hash & hash_cutover_split_mask;

    for (p = hash_table[idx]; p; p = p->wstr_next)
    {
        if
        (
            p->wstr_hash == hash
        &&
            p->wstr_length == length
        &&
            !memcmp(p->wstr_text, s, length * sizeof(wchar_t))
        )
        {
            p->wstr_references++;
            return p;
        }
    }

    p = (wstring_ty *)mem_alloc(sizeof(wstring_ty) + length * sizeof(wchar_t));
    p->wstr_hash = hash;
    p->wstr_length = length;
    p->wstr_references = 1;
    p->wstr_next = hash_table[idx];
    hash_table[idx] = p;
    memcpy(p->wstr_text, s, length * sizeof(wchar_t));
    p->wstr_text[length] = 0;

    hash_load++;
    while (hash_load * 10 > hash_modulus * 8)
        split();
    ++changed;
    return p;
}


/*
 * NAME
 *      wstr_copy - make a copy of a string
 *
 * SYNOPSIS
 *      wstring_ty *wstr_copy(wstring_ty *s);
 *
 * DESCRIPTION
 *      The wstr_copy function is used to make a copy of a string.
 *
 * RETURNS
 *      wstring_ty* - a pointer to a string in dynamic memory.
 *      Use wstr_free when finished with.
 *
 * CAVEAT
 *      The contents of the structure pointed to MUST NOT be altered.
 */

wstring_ty *
wstr_copy(wstring_ty *s)
{
    s->wstr_references++;
    return s;
}


/*
 * NAME
 *      wstr_free - release a string
 *
 * SYNOPSIS
 *      void wstr_free(wstring_ty *s);
 *
 * DESCRIPTION
 *      The wstr_free function is used to indicate that a string hash been
 *      finished with.
 *
 * RETURNS
 *      void
 *
 * CAVEAT
 *      This is the only way to release strings DO NOT use the free function.
 */

void
wstr_free(wstring_ty *s)
{
    wstr_hash_ty    idx;
    wstring_ty      **spp;

    if (!s)
        return;
    if (s->wstr_references > 1)
    {
        s->wstr_references--;
        return;
    }
    ++changed;

    /*
     * find the hash bucket it was in,
     * and remove it
     */
    idx = s->wstr_hash & hash_cutover_mask;
    if (idx < hash_split)
        idx = s->wstr_hash & hash_cutover_split_mask;
    for (spp = &hash_table[idx]; *spp; spp = &(*spp)->wstr_next)
    {
        if (*spp == s)
        {
            *spp = s->wstr_next;
            free(s);
            --hash_load;
            return;
        }
    }

    /*
     * should never reach here!
     */
    fatal_raw("attempted to free non-existent wstring (bug)");
}


/*
 * NAME
 *      wstr_catenate - join two strings
 *
 * SYNOPSIS
 *      wstring_ty *wstr_catenate(wstring_ty *, wstring_ty *);
 *
 * DESCRIPTION
 *      The wstr_catenate function is used to concatenate two strings to form a
 *      new string.
 *
 * RETURNS
 *      wstring_ty* - a pointer to a string in dynamic memory.
 *      Use wstr_free when finished with.
 *
 * CAVEAT
 *      The contents of the structure pointed to MUST NOT be altered.
 */

wstring_ty *
wstr_catenate(wstring_ty *s1, wstring_ty *s2)
{
    static wchar_t  *tmp;
    static size_t   tmplen;
    wstring_ty      *s;
    size_t          length;

    length = s1->wstr_length + s2->wstr_length;
    if (length > tmplen)
    {
        tmplen = length;
        tmp = mem_change_size(tmp, tmplen * sizeof(wchar_t));
    }
    memcpy(tmp, s1->wstr_text, s1->wstr_length * sizeof(wchar_t));
    memcpy
    (
        tmp + s1->wstr_length,
        s2->wstr_text,
        s2->wstr_length * sizeof(wchar_t)
    );
    s = wstr_n_from_wc(tmp, length);
    return s;
}


/*
 * NAME
 *      wstr_cat_three - join three strings
 *
 * SYNOPSIS
 *      wstring_ty *wstr_cat_three(wstring_ty *, wstring_ty *, wstring_ty *);
 *
 * DESCRIPTION
 *      The wstr_cat_three function is used to concatenate three strings to form
 *      a new string.
 *
 * RETURNS
 *      wstring_ty* - a pointer to a string in dynamic memory.
 *      Use wstr_free when finished with.
 *
 * CAVEAT
 *      The contents of the structure pointed to MUST NOT be altered.
 */

wstring_ty *
wstr_cat_three(wstring_ty *s1, wstring_ty *s2, wstring_ty *s3)
{
    static wchar_t  *tmp;
    static size_t   tmplen;
    wstring_ty      *s;
    size_t          length;

    length = s1->wstr_length + s2->wstr_length + s3->wstr_length;
    if (tmplen < length)
    {
        tmplen = length;
        tmp = mem_change_size(tmp, tmplen * sizeof(wchar_t));
    }
    memcpy(tmp, s1->wstr_text, s1->wstr_length * sizeof(wchar_t));
    memcpy
    (
        tmp + s1->wstr_length,
        s2->wstr_text,
        s2->wstr_length * sizeof(wchar_t)
    );
    memcpy
    (
        tmp + s1->wstr_length + s2->wstr_length,
        s3->wstr_text,
        s3->wstr_length * sizeof(wchar_t)
    );
    s = wstr_n_from_wc(tmp, length);
    return s;
}


wstring_ty *
wstr_to_upper(const wstring_ty *ws)
{
    static wchar_t  *buffer;
    static size_t   buflen;
    size_t          j;

    if (ws->wstr_length > buflen)
    {
        buflen = ws->wstr_length;
        buffer = mem_change_size(buffer, buflen * sizeof(wchar_t));
    }
    language_human();
    for (j = 0; j < ws->wstr_length; ++j)
    {
        wchar_t         c;

        c = ws->wstr_text[j];
        if (iswlower(c))
            c = towupper(c);
        buffer[j] = c;
    }
    language_C();
    return wstr_n_from_wc(buffer, ws->wstr_length);
}


wstring_ty *
wstr_to_lower(const wstring_ty *ws)
{
    static wchar_t  *buffer;
    static size_t   buflen;
    size_t          j;

    if (ws->wstr_length > buflen)
    {
        buflen = ws->wstr_length;
        buffer = mem_change_size(buffer, buflen * sizeof(wchar_t));
    }
    language_human();
    for (j = 0; j < ws->wstr_length; ++j)
    {
        wchar_t         c;

        c = ws->wstr_text[j];
        if (iswupper(c))
            c = towlower(c);
        buffer[j] = c;
    }
    language_C();
    return wstr_n_from_wc(buffer, ws->wstr_length);
}


wstring_ty *
wstr_to_ident(const wstring_ty *ws)
{
    static wchar_t  *buffer;
    static size_t   buflen;
    size_t          j;

    if (ws->wstr_length == 0)
        return wstr_from_c("_");
    if (ws->wstr_length > buflen)
    {
        buflen = ws->wstr_length;
        buffer = mem_change_size(buffer, buflen * sizeof(wchar_t));
    }
    language_human();
    for (j = 0; j < ws->wstr_length; ++j)
    {
        wchar_t         c;

        c = ws->wstr_text[j];
        if (!iswalnum(c))
            c = '_';
        buffer[j] = c;
    }
    if (iswdigit(buffer[0]))
        buffer[0] = '_';
    language_C();
    return wstr_n_from_wc(buffer, ws->wstr_length);
}


/*
 * NAME
 *      wstr_equal - test equality of strings
 *
 * SYNOPSIS
 *      int wstr_equal(wstring_ty *, wstring_ty *);
 *
 * DESCRIPTION
 *      The wstr_equal function is used to test if two strings are equal.
 *
 * RETURNS
 *      int; zero if the strings are not equal, nonzero if the strings are
 *      equal.
 *
 * CAVEAT
 *      This function is implemented as a macro in strings.h
 */

#ifndef wstr_equal

int
wstr_equal(wstring_ty *s1, wstring_ty *s2)
{
    return (s1 == s2);
}

#endif


wstring_ty *
str_to_wstr(string_ty *s)
{
    return wstr_n_from_c(s->str_text, s->str_length);
}


string_ty *
wstr_to_str(wstring_ty *ws)
{
    char            *text;
    size_t          length;

    wstr_to_mbs(ws, &text, &length);
    return str_n_from_c(text, length);
    trace(("to silence warnings\n"));
}

/*
 *	cook - file construction tool
 *	Copyright (C) 1993-1995, 1997-1999, 2001, 2003 Peter Miller;
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
 * MANIFEST: functions to manipulate shared strings
 *
 * Strings are the most heavily used resource in cook.  They are manipulated
 * inside the match functions, and hence are in the inside loop.  For this
 * reason they must be fast.
 *
 * A literal pool is maintained.  Each string has a reference count.  The
 * string stays in the literal pool for as long as it hash a positive
 * reference count.  To determine if a string is already in the literal pool,
 * linear dynamic hashing is used to guarantee an O(1) search.  That all equal
 * strings are the same item in the literal pool means that string equality is
 * a pointer test, and thus very fast.
 */

#include <ac/ctype.h>
#include <ac/stddef.h>
#include <ac/stdio.h>
#include <ac/stdlib.h>
#include <ac/string.h>

#include <error.h>
#include <mem.h>
#include <mprintf.h>
#include <str.h>
#include <trace.h> /* for assert */


/*
 * maximum conversion width for numbers
 */
#define MAX_WIDTH 509

string_ty *str_true;
string_ty *str_false;
static string_ty **hash_table;
static str_hash_ty hash_modulus;
static str_hash_ty hash_mask;
static str_hash_ty hash_load;

#define MAX_HASH_LEN 20


/*
 * NAME
 *	hash_generate - hash string to number
 *
 * SYNOPSIS
 *	str_hash_ty hash_generate(char *s, size_t n);
 *
 * DESCRIPTION
 *	The hash_generate function is used to make a number from a string.
 *
 * RETURNS
 *	str_hash_ty - the magic number
 *
 * CAVEAT
 *	Only the last MAX_HASH_LEN characters are used.
 *	It is important that str_hash_ty be unsigned (int or long).
 */

static str_hash_ty
hash_generate(const char *s, size_t n)
{
    str_hash_ty     retval;

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
 *	str_valid - test a string
 *
 * SYNOPSIS
 *	int str_valid(string_ty *s);
 *
 * DESCRIPTION
 *	The str_valid function is used to test if a pointer points to a valid
 *	string.
 *
 * RETURNS
 *	int: zero if the string is not valid, nonzero if the string is valid.
 *
 * CAVEAT
 *	This function is only available then the DEBUG symbol is #define'd.
 */

#ifdef DEBUG

int
str_valid(string_ty *s)
{
return
(
    s->str_references > 0
&&
    strlen(s->str_text) == s->str_length
&&
    s->str_hash == hash_generate(s->str_text, s->str_length)
);
}

#endif


/*
* NAME
*	str_initialize - start up string table
*
* SYNOPSIS
*	void str_initialize(void);
*
* DESCRIPTION
*	The str_initialize function is used to create the hash table and
*	initialize it to empty.
*
* RETURNS
*	void
*
* CAVEAT
*	This function must be called before any other defined in this file.
*/

void
str_initialize(void)
{
    str_hash_ty     j;

    hash_modulus = 1 << 8; /* MUST be a power of 2 */
    hash_mask = hash_modulus - 1;
    hash_load = 0;
    hash_table = mem_alloc(hash_modulus * sizeof(string_ty *));
    for (j = 0; j < hash_modulus; ++j)
	    hash_table[j] = 0;

    str_true = str_from_c("1");
    str_false = str_from_c("");
}


/*
 * NAME
 *	split - reduce table loading
 *
 * SYNOPSIS
 *	void split(void);
 *
 * DESCRIPTION
 *	The split function is used to reduce the load factor on the hash table.
 *
 * RETURNS
 *	void
 *
 * CAVEAT
 *	A load factor of about 80% is suggested.
 */

static void
split(void)
{
    string_ty       **new_hash_table;
    str_hash_ty     new_hash_modulus;
    str_hash_ty     new_hash_mask;
    str_hash_ty     j;

    /*
     * double the modulus
     *
     * This is subtle.  If we only increase the modulus by one, the
     * load always hovers around 80%, so we have to do a split for
     * every insert.  I.e. the malloc burden is O(n) for the lifetime of
     * the program.  BUT if we double the modulus, the length of time
     * until the next split also doubles, making the probablity of a
     * split halve, and sigma(2**-n)=1, so the malloc burden becomes O(1)
     * for the lifetime of the program.
     */
    new_hash_modulus = hash_modulus << 1;
    new_hash_mask = new_hash_modulus - 1;
    new_hash_table = mem_alloc(new_hash_modulus * sizeof(string_ty *));

    /*
     * now redistribute the list elements
     */
    for (j = 0; j < hash_modulus; ++j)
    {
	string_ty       *p;

	p = hash_table[j];
	new_hash_table[j] = 0;
	new_hash_table[hash_modulus + j] = 0;
	while (p)
	{
	    string_ty       *p2;
	    str_hash_ty     idx;

	    p2 = p;
	    p = p->str_next;

	    idx = p2->str_hash & new_hash_mask;
	    p2->str_next = new_hash_table[idx];
	    new_hash_table[idx] = p2;
	}
    }

    /*
     * swap it over
     */
    mem_free(hash_table);
    hash_table = new_hash_table;
    hash_modulus = new_hash_modulus;
    hash_mask = new_hash_mask;
}


/*
 * NAME
 *	str_from_c - make string from C string
 *
 * SYNOPSIS
 *	string_ty *str_from_c(char*);
 *
 * DESCRIPTION
 *	The str_from_c function is used to make a string from a null terminated
 *	C string.
 *
 * RETURNS
 *	string_ty * - a pointer to a string in dynamic memory.
 *	Use str_free when finished with.
 *
 * CAVEAT
 *	The contents of the structure pointed to MUST NOT be altered.
 */

string_ty *
str_from_c(const char *s)
{
    return str_n_from_c(s, strlen(s));
}


/*
 * NAME
 *	str_n_from_c - make string
 *
 * SYNOPSIS
 *	string_ty *str_n_from_c(char *s, size_t n);
 *
 * DESCRIPTION
 *	The str_n_from_c function is used to make a string from an array of
 *	characters.  No null terminator is assumed.
 *
 * RETURNS
 *	string_ty * - a pointer to a string in dynamic memory.
 *	Use str_free when finished with.
 *
 * CAVEAT
 *	The contents of the structure pointed to MUST NOT be altered.
 */

string_ty *
str_n_from_c(const char *s, size_t length)
{
    str_hash_ty     hash;
    str_hash_ty     idx;
    string_ty       *p;

    hash = hash_generate(s, length);

#ifdef DEBUG
    if (!hash_table)
	fatal_raw("you must call str_initialize early in main()");
#endif
    idx = hash & hash_mask;
    assert(idx < hash_modulus);

    for (p = hash_table[idx]; p; p = p->str_next)
    {
	if
	(
	    p->str_hash == hash
	&&
	    p->str_length == length
	&&
	    0 == memcmp(p->str_text, s, length)
	)
	{
	    p->str_references++;
	    return p;
	}
    }

    p = mem_alloc(sizeof(string_ty) + length);
    p->str_hash = hash;
    p->str_length = length;
    p->str_references = 1;
    p->str_next = hash_table[idx];
    hash_table[idx] = p;
#if 0
    /* silence purify */
    {
	    /*
	     * probably sizeof(int) bytes,
	     * but is compiler dependent
	     */
	    size_t n = sizeof(string_ty) - offsetof(string_ty, str_text);
	    memset(p->str_text, 0, n);
    }
#endif
    memcpy(p->str_text, s, length);
    p->str_text[length] = 0;

    hash_load++;
    if (hash_load * 10 > hash_modulus * 8)
	split();
    return p;
}


/*
 * NAME
 *	str_copy - make a copy of a string
 *
 * SYNOPSIS
 *	string_ty *str_copy(string_ty *s);
 *
 * DESCRIPTION
 *	The str_copy function is used to make a copy of a string.
 *
 * RETURNS
 *	string_ty * - a pointer to a string in dynamic memory.
 *	Use str_free when finished with.
 *
 * CAVEAT
 *	The contents of the structure pointed to MUST NOT be altered.
 */

string_ty *
str_copy(string_ty *s)
{
    s->str_references++;
    return s;
}


/*
 * NAME
 *	str_free - release a string
 *
 * SYNOPSIS
 *	void str_free(string_ty *s);
 *
 * DESCRIPTION
 *	The str_free function is used to indicate that a string hash been
 *	finished with.
 *
 * RETURNS
 *	void
 *
 * CAVEAT
 *	This is the only way to release strings DO NOT use the free function.
 */

void
str_free(string_ty *s)
{
    str_hash_ty     idx;
    string_ty       **spp;

    assert(str_valid(s));
    if (s->str_references > 1)
    {
	s->str_references--;
	return;
    }
    assert(s->str_references == 1);

    /*
     * find the hash bucket it was in,
     * and remove it
     */
    idx = s->str_hash & hash_mask;
    assert(idx < hash_modulus);
    for (spp = &hash_table[idx]; *spp; spp = &(*spp)->str_next)
    {
	if (*spp == s)
	{
	    *spp = s->str_next;
	    free(s);
	    --hash_load;
	    return;
	}
    }
    /* should never reach here! */
    fatal_raw("attempted to free non-existent string (bug)");
}


/*
 * NAME
 *	str_equal - test equality of strings
 *
 * SYNOPSIS
 *	int str_equal(string_ty *, string_ty *);
 *
 * DESCRIPTION
 *	The str_equal function is used to test if two strings are equal.
 *
 * RETURNS
 *	int; zero if the strings are not equal, nonzero if the strings are
 *	equal.
 *
 * CAVEAT
 *	This function is implemented as a macro in strings.h
 */

#ifndef str_equal

int
str_equal(string_ty *s1, string_ty *s2)
{
    return (s1 == s2);
}

#endif


/*
 * NAME
 *	str_bool - get boolean value
 *
 * SYNOPSIS
 *	int str_bool(string_ty *s);
 *
 * DESCRIPTION
 *	The str_bool function is used to determine the boolean value of the
 *	given string.  A "false" result is if the string is empty or
 *	0 or blank, and "true" otherwise.
 *
 * RETURNS
 *	int: zero to indicate a "false" result, nonzero to indicate a "true"
 *	result.
 */

int
str_bool(string_ty *s)
{
    char        *cp;

    cp = s->str_text;
    while (*cp)
    {
	if (*cp != ' ' && *cp != '0')
    	    return 1;
	++cp;
    }
    return 0;
}


/*
 * NAME
 *	str_field - extract a field from a string
 *
 * SYNOPSIS
 *	string_ty *str_field(string_ty *, char separator, int field_number);
 *
 * DESCRIPTION
 *	The str_field functipon is used to erxtract a field from a string.
 *	Fields of the string are separated by ``separator'' characters.
 *	Fields are numbered from 0.
 *
 * RETURNS
 *	Asking for a field off the end of the string will result in a null
 *	pointer return.  The null string is considered to have one empty field.
 */

string_ty *
str_field(string_ty *s, int sep, int fldnum)
{
    char        *cp;
    char        *ep;

    cp = s->str_text;
    while (fldnum > 0)
    {
	ep = strchr(cp, sep);
	if (!ep)
    	    return 0;
	cp = ep + 1;
	--fldnum;
    }
    ep = strchr(cp, sep);
    if (ep)
	return str_n_from_c(cp, ep - cp);
    return str_from_c(cp);
}


/*
 * NAME
 *	str_format - analog of sprintf
 *
 * SYNOPSIS
 *	string_ty *str_format(char *, ...);
 *
 * DESCRIPTION
 *	The str_format function is used to create new strings
 *	using a format specification similar to printf(3).
 *	The "%S" specifier is used to mean a ``string_ty *'' string.
 *
 * RETURNS
 *	string_ty * - a pointer to a string in dynamic memory.
 *	Use str_free when finished with.
 */


string_ty *
str_format(const char *fmt, ...)
{
    va_list         ap;
    string_ty       *result;

    sva_init(ap, fmt);
    result = vmprintfes(fmt, ap);
    va_end(ap);
    return result;
    /* to silence debug warning */
    trace(("bogus\n"));
}


string_ty *
str_vformat(const char *fmt, va_list ap)
{
    return vmprintfes(fmt, ap);
}

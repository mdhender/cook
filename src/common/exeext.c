/*
 *      cook - file construction tool
 *      Copyright (C) 1999, 2006-2008 Peter Miller
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

#include <common/exeext.h>

#include <common/ac/ctype.h>
#include <common/ac/string.h>

#include <common/exeext.h>
#include <common/libdir.h>


static int
memcasecmp(const char *s1, const char *s2, size_t n)
{
    int             c1;
    int             c2;

    while (n > 0)
    {
        c1 = *s1++;
        if (isupper(c1))
            c1 = tolower(c1);
        c2 = *s2++;
        if (isupper(c2))
            c1 = tolower(c2);
        if (c1 != c2)
            return ((unsigned char)c1 - (unsigned char)c2);
        --n;
    }
    return 0;
}


static int
look_for_suffix(const char *stem, const char *suffix)
{
    size_t          main_len;
    size_t          suffix_len;
    size_t          idx;

    main_len = strlen(stem);
    suffix_len = strlen(suffix);
    if (main_len < suffix_len)
        return -1;
    idx = main_len - suffix_len;
    if (0 != memcasecmp(stem + idx, suffix, suffix_len))
        return -1;
    return idx;
}


const char *
exeext_nth(int n)
{
    switch (n)
    {
    case 0:
        return executable_extension_get();

#if defined(__CYGWIN__) || defined(__CYGWIN32__) || defined(__NUTC__)
    case 1:
        return ".bat";

    case 2:
        return ".cmd";

    case 3:
        return ".com";

    case 4:
        return ".exe";
#endif
    }
    return 0;
}


int
exeext(const char *s)
{
    int             n;
    const char      *suffix;
    int             result;

    for (n = 0;; ++n)
    {
        suffix = exeext_nth(n);
        if (!suffix || !*suffix)
            return -1;
        result = look_for_suffix(s, suffix);
        if (result >= 0)
            return result;
    }
}

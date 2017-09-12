/*
 *      cook - file construction tool
 *      Copyright (C) 1992-1994, 1997-1999, 2001, 2006, 2007 Peter Miller;
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

#include <common/ac/stddef.h>
#include <common/ac/stdlib.h>
#include <common/ac/string.h>

#include <common/env.h>
#include <common/mem.h>


extern char   **environ;
static size_t   nenvirons;
static int      initialized;


/*
 * NAME
 *      env_initialize - start up environment
 *
 * SYNOPSIS
 *      void env_initialize(void);
 *
 * DESCRIPTION
 *      The env_initialize function is used to copy all of the environment
 *      variables into dynamic memory, so that they may be altered by the
 *      setenv and unsetenv commands.
 */

static void
env_initialize(void)
{
    size_t          j;
    char            **old;

    if (initialized)
        return;
    initialized = 1;
    nenvirons = 0;
    for (j = 0; environ[j]; ++j)
        ++nenvirons;
    old = environ;
    environ = mem_alloc((nenvirons + 1) * sizeof(char *));
    for (j = 0; j < nenvirons; ++j)
    {
        char            *cp;
        char            *was;
        size_t          len;

        was = old[j];
        len = strlen(was) + 1;
        cp = mem_alloc(len);
        memcpy(cp, was, len);
        environ[j] = cp;
    }
    environ[nenvirons] = 0;
    env_set("SHELL", "/bin/sh");
}


/*
 * NAME
 *      setenv - set environment variable
 *
 * SYNOPSIS
 *      void setenv(char *name, char *value);
 *
 * DESCRIPTION
 *      The setenv function is used to set the given environment variable to
 *      the given value.
 *
 * CAVEAT
 *      Assumes that the env_initialize function has already been called.
 */

void
env_set(const char *name, const char *value)
{
    size_t          name_len;
    size_t          j;
    char            *cp;
    size_t          len2;

    if (!initialized)
        env_initialize();
    cp = 0;
    name_len = strlen(name);
    for (j = 0; j < nenvirons; ++j)
    {
        cp = environ[j];
        /* assert(cp); */
        if
        (
            name_len <= strlen(cp)
        &&
            (cp[name_len] == '=' || !cp[name_len])
        &&
            !memcmp(cp, name, name_len)
        )
            break;
    }
    if (environ[j])
    {
        environ[j] = 0;
        if (cp)
            mem_free(cp);
    }
    else
    {
        size_t          nbytes;

        nbytes = (nenvirons + 2) * sizeof(char *);
        environ = mem_change_size(environ, nbytes);
        environ[++nenvirons] = 0;
    }
    len2 = strlen(value);
    cp = mem_alloc(name_len + len2 + 2);
    memcpy(cp, name, name_len);
    cp[name_len] = '=';
    memcpy(cp + name_len + 1, value, len2 + 1);
    environ[j] = cp;
}


/*
 * NAME
 *      unsetenv - remove environment variable
 *
 * SYNOPSIS
 *      void unsetenv(char *name);
 *
 * DESCRIPTION
 *      The unsetenv function is used to remove the named variable from the
 *      environment.
 *
 * RETURNS
 *      void
 *
 * CAVEAT
 *      Assumes that the env_initialize function has been called already.
 */

void
env_unset(const char *name)
{
    size_t          name_len;
    size_t          j;
    char            *cp;

    if (!initialized)
        env_initialize();
    name_len = strlen(name);
    cp = 0;
    for (j = 0; j < nenvirons; ++j)
    {
        cp = environ[j];
        /* assert(cp); */
        if
        (
            (cp[name_len] == '=' || !cp[name_len])
        &&
            !strncmp(cp, name, name_len)
        )
        {
            break;
        }
    }
    if (!environ[j])
        return;
    environ[j] = 0;
    if (cp)
        mem_free(cp);
    --nenvirons;
    for (; j < nenvirons; ++j)
        environ[j] = environ[j + 1];
}

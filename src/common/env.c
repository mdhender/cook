/*
 *	cook - file construction tool
 *	Copyright (C) 1992, 1993, 1994, 1997, 1998, 1999, 2001 Peter Miller;
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
 * MANIFEST: functions to manipulate environment variables
 */

#include <ac/stddef.h>
#include <ac/stdlib.h>
#include <ac/string.h>

#include <env.h>
#include <mem.h>


extern	char	**environ;
static	size_t	nenvirons;
static	int	initialized;


/*
 * NAME
 *	env_initialize - start up environment
 *
 * SYNOPSIS
 *	void env_initialize(void);
 *
 * DESCRIPTION
 *	The env_initialize function is used to copy all of the environment
 *	variables into dynamic memory, so that they may be altered by the
 *	setenv and unsetenv commands.
 */

static void env_initialize _((void));

static void
env_initialize()
{
	int		j;
	char	**old;

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
		char	*cp;
		char	*was;

		was = old[j];
		cp = mem_alloc(strlen(was) + 1);
		strcpy(cp, was);
		environ[j] = cp;
	}
	environ[nenvirons] = 0;
	env_set("SHELL", "/bin/sh");
}


/*
 * NAME
 *	setenv - set environment variable
 *
 * SYNOPSIS
 *	void setenv(char *name, char *value);
 *
 * DESCRIPTION
 *	The setenv function is used to set the given environment variable to
 *	the given value.
 *
 * CAVEAT
 *	Assumes that the env_initialize function has already been called.
 */

void
env_set(name, value)
	const char	*name;
	const char	*value;
{
	size_t		name_len;
	int		j;
	char		*cp;

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
		size_t	nbytes;

		nbytes = (nenvirons + 2) * sizeof(char *);
		environ = mem_change_size(environ, nbytes);
		environ[++nenvirons] = 0;
	}
	cp = mem_alloc(name_len + strlen(value) + 2);
	strcpy(cp, name);
	cp[name_len] = '=';
	strcpy(cp + name_len + 1, value);
	environ[j] = cp;
}


/*
 * NAME
 *	unsetenv - remove environment variable
 *
 * SYNOPSIS
 *	void unsetenv(char *name);
 *
 * DESCRIPTION
 *	The unsetenv function is used to remove the named variable from the
 *	environment.
 *
 * RETURNS
 *	void
 *
 * CAVEAT
 *	Assumes that the env_initialize function has been called already.
 */

void
env_unset(name)
	const char	*name;
{
	size_t		name_len;
	int		j;
	char		*cp;

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
			break;
	}
	if (!environ[j])
		return;
	environ[j] = 0;
	if (cp)
		mem_free(cp);
	--nenvirons;
	for ( ; j < nenvirons; ++j)
		environ[j] = environ[j + 1];
}

/*
 *	cook - file construction tool
 *	Copyright (C) 1997, 2001, 2004 Peter Miller;
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
 * MANIFEST: functions to manipulate languages
 */

#include <ac/locale.h>
#include <ac/libintl.h>
#include <ac/stdlib.h>

#include <env.h>
#include <error.h>
#include <language.h>
#include <libdir.h>
#include <progname.h>


enum state_ty
{
	state_uninitialized,
	state_C,
	state_human
};
typedef enum state_ty state_ty;

static state_ty state;


/*
 * NAME
 *	language_init - initialize language functions
 *
 * DESCRIPTION
 *	The language_init function must be called at the start of the
 *	program, to set the various locale features.
 *
 *	This function must be called after the setuid initialization.
 *	If you forget to call me, all bets are off.
 */

void
language_init()
{
	const char      *lib;
	const char      *package;

	if (state != state_uninitialized)
		return;
	state = state_C;

	/*
	 * Default the error message language to English if not set.
	 * Since we expect to be using GNU Gettext, only set the LANGUAGE
	 * environment variable.
	 */
	if (!getenv("LANGUAGE") && !getenv("LANG"))
		env_set("LANGUAGE", "en");

	/*
	 * Set the locale to the default (as defined by the environment
	 * variables) and set the message domain information.
	 */
	package = progname_get();
#ifdef DEBUG
	if (!package || !*package)
	{
		fatal_raw
		(
			"you must call progname_set before language_init (bug)"
		);
	}
#endif
	lib = getenv("COOK_MESSAGE_LIBRARY");
	if (!lib || !*lib)
		lib = configured_nlsdir();
#ifdef HAVE_SETLOCALE
#ifdef HAVE_GETTEXT
	setlocale(LC_ALL, "");
	bindtextdomain(package, lib);
	textdomain(package);
#endif /* HAVE_GETTEXT */

	/*
	 * set the main body of the program use use the C locale
	 */
	setlocale(LC_ALL, "C");
#endif /* HAVE_SETLOCALE */
}


/*
 * NAME
 *	language_human - set for human conversation
 *
 * DESCRIPTION
 *	The language_human function must be called to change the general
 *	mode over to the default locale (usually dictated by the LANG
 *	environment variable, et al).
 *
 *	The language_human and language_C functions MUST bracket human
 *	interactions, otherwise the mostly-english C locale will be
 *	used.  The default locale through-out the program is otherwise
 *	assumed to be C.
 */

void
language_human()
{
#ifdef DEBUG
	switch (state)
	{
	case state_uninitialized:
		fatal_raw("you must call language_init() in main (bug)");

	case state_human:
		fatal_raw("unbalanced language_human() call (bug)");

	case state_C:
		break;
	}
#endif
	state = state_human;
#ifdef HAVE_SETLOCALE
#ifdef HAVE_GETTEXT
	/*
	 * only need to flap the locale about like this
	 * if we are using the gettext function
	 */
	setlocale(LC_ALL, "");
#endif /* HAVE_GETTEXT */
#endif /* HAVE_SETLOCALE */
}


/*
 * NAME
 *	language_C - set for program conversation
 *
 * DESCRIPTION
 *	The language_C function must be called to restore the locale to
 *	C, so that all the non-human stuff will work.
 *
 *	The language_human and language_C functions MUST bracket human
 *	interactions, otherwise the mostly-english C locale will be
 *	used.  The default locale through-out the program is otherwise
 *	assumed to be C.
 */

void
language_C()
{
#ifdef DEBUG
	switch (state)
	{
	case state_uninitialized:
		fatal_raw("you must call language_init() in main (bug)");

	case state_C:
		fatal_raw("unbalanced language_C() call (bug)");

	case state_human:
		break;
	}
#endif
	state = state_C;
#ifdef HAVE_SETLOCALE
#ifdef HAVE_GETTEXT
	/*
	 * only need to flap the locale about like this
	 * if we are using the gettext function
	 */
	setlocale(LC_ALL, "C");
#endif /* HAVE_GETTEXT */
#endif /* HAVE_SETLOCALE */
}

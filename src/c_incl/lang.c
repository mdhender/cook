/*
 *	cook - file construction tool
 *	Copyright (C) 1997, 1998 Peter Miller;
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
 * MANIFEST: functions to manipulate source language seclection
 */

#include <ac/string.h>

#include <error_intl.h>
#include <lang.h>
#include <lang_c.h>
#include <lang_optimis.h>
#include <lang_roff.h>
#include <lang_m4.h>
#include <symtab.h>


typedef struct table_ty table_ty;
struct table_ty
{
	char		*name;
	sniff_ty	*sniff;
};

static table_ty table[] =
{
	/* name must be lower case */
	{ "c",		&lang_c,	},
	{ "c++",	&lang_c,	},
	{ "ditroff",	&lang_roff,	},
	{ "groff",	&lang_roff,	},
	{ "m4",		&lang_m4,	},
	{ "nroff",	&lang_roff,	},
	{ "optimistic", &lang_optimistic, },
	{ "roff",	&lang_roff,	},
	{ "troff",	&lang_roff,	},
};

static symtab_ty *stp;


sniff_ty *
lang_from_name(name)
	char		*name;
{
	table_ty	*tp;
	string_ty	*s;
	string_ty	*s2;
	sniff_ty	*result;
	sub_context_ty	*scp;

	/*
	 * OK, so building a symbol table is over-kill, but it allows
	 * some simple manipulations.
	 */
	if (!stp)
	{
		stp = symtab_alloc(SIZEOF(table));
		for (tp = table; tp < ENDOF(table); ++tp)
		{
			s = str_from_c(tp->name);
			symtab_assign(stp, s, tp->sniff);
			str_free(s);
		}
	}

	/*
	 * look for the name
	 */
	s = str_from_c(name);
	s2 = str_downcase(s);
	str_free(s);
	result = symtab_query(stp, s2);
	if (result)
	{
		str_free(s2);
		return result;
	}

	/*
	 * Complain if we don't understand.
	 */
	if (symtab_query_fuzzy(stp, s2, &s))
	{
		scp = sub_context_new();
		sub_var_set(scp, "Name", "%s", name);
		sub_var_set(scp, "Guess", "%S", s);
		fatal_intl
		(
			scp,
			i18n("input language $name unknown, closest is $guess")
		);
		/* NOTREACHED */
		sub_context_delete(scp);
		return 0;
	}
	scp = sub_context_new();
	sub_var_set(scp, "Name", "%s", name);
	fatal_intl(scp, i18n("input language $name unknown"));
	/* NOTREACHED */
	sub_context_delete(scp);
	return 0;
}

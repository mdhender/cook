/*
 *	cook - file construction tool
 *	Copyright (C) 1991, 1992, 1993, 1994, 1997, 1998, 2001 Peter Miller;
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
 * MANIFEST: functions to perform lexical analysis on command line arguments
 */

#include <ac/stddef.h>
#include <ac/stdlib.h>
#include <ac/string.h>
#include <ac/ctype.h>

#include <main.h>
#include <arglex.h>
#include <error_intl.h>
#include <language.h>
#include <mem.h>
#include <progname.h>
#include <trace.h>
#include <str_list.h>

static arglex_table_ty table[] =
{
	{ "-",			arglex_token_stdio,		},
	{ "-Help",		arglex_token_help,		},
	{ "-Page_Width",	arglex_token_page_width,	},
	{ "-Page_Length",	arglex_token_page_length,	},
	{ "-TRACIng",		arglex_token_tracing,		},
	{ "-Verbose",		arglex_token_verbose,		},
	{ "-VERSion",		arglex_token_version,		},
};

static int	argc;
static char	**argv;
arglex_value_ty	arglex_value;
arglex_token_ty	arglex_token;
static arglex_table_ty *utable;
static const char *partial;


static char *base_name _((char *));

static char *
base_name(s)
	char	*s;
{
	char	*bp;
	char	*ep;

	bp = s;
	ep = 0;
	while (*s)
	{
		if (s[0] == '/' && s[1] && s[1] != '/')
			bp = s + 1;
		if (s > bp && s[0] == '/' && s[-1] != '/')
			ep = s;
		++s;
	}
	if (!ep)
		ep = s;
	*s = 0;
	return bp;
}


/*
 * NAME
 *	arglex_init
 *
 * SYNOPSIS
 *	void arglex_init(int ac, char **av, arglex_table-t *tp);
 *
 * DESCRIPTION
 *	The arglex_init function is used to initialize the
 *	command line processing.
 *
 * ARGUMENTS
 *	ac	- aergument count, from main
 *	av	- argument values, from main
 *	tp	- pointer to table of options
 *
 * CAVEAT
 *	Must be called before the arglex() function.
 */

void
arglex_init(ac, av, tp)
	int	ac;
	char	**av;
	arglex_table_ty *tp;
{
	progname_set(av[0]);
	language_init();

	argc = ac - 1;
	argv = av + 1;
	utable = tp;
}


/*
 * NAME
 *	arglex_compare
 *
 * SYNOPSIS
 *	int arglex_compare(char *formal, char *actual);
 *
 * DESCRIPTION
 *	The arglex_compare function is used to compare
 *	a command line string with a formal spec of the option,
 *	to see if they compare equal.
 *
 *	The actual is case-insensitive.  Uppercase in the formal
 *	means a mandatory character, while lower case means optional.
 *	Any number of consecutive optional characters may be supplied
 *	by actual, but none may be skipped, unless all are skipped to
 *	the next non-lower-case letter.
 *
 *	The underscore (_) is like a lower-case minus,
 *	it matches "", "-" and "_".
 *
 *	The "*" in a pattern matches everything to the end of the line,
 *	anything after the "*" is ignored.  The rest of the line is pointed
 *	to by the "partial" variable as a side-effect (else it will be 0).
 *	This rather ugly feature is to support "-I./dir" type options.
 *
 *	A backslash in a pattern nominates an exact match required,
 *	case must matche excatly here.
 *	This rather ugly feature is to support "-I./dir" type options.
 *
 *	For example: "-project" and "-P' both match "-Project",
 *	as does "-proJ", but "-prj" does not.
 *
 *	For example: "-devDir" and "-d_d' both match "-Development_Directory",
 *	but "-dvlpmnt_drctry" does not.
 *
 *	For example: to match include path specifications, use a pattern
 *	such as "-\\I*", and the partial global variable will have the
 *	path in it on return.
 *
 * ARGUMENTS
 *	formal	- the "pattern" for the option
 *	actual	- what the user supplied
 *
 * RETURNS
 *	int;	zero if no match,
 *		non-zero if they do match.
 */

int
arglex_compare(formal, actual)
	const char	*formal;
	const char	*actual;
{
	char		fc;
	char		ac;
	int		result;

	trace(("arglex_compare(formal = \"%s\", actual = \"%s\")\n{\n",
		formal, actual));
	for (;;)
	{
		trace_string(formal);
		trace_string(actual);
		ac = *actual++;
		if (isupper(ac))
			ac = tolower(ac);
		fc = *formal++;
		switch (fc)
		{
		case 0:
			result = !ac;
			goto done;

		case '_':
			if (ac == '-')
				break;
			/* fall through... */

		case 'a': case 'b': case 'c': case 'd': case 'e':
		case 'f': case 'g': case 'h': case 'i': case 'j':
		case 'k': case 'l': case 'm': case 'n': case 'o':
		case 'p': case 'q': case 'r': case 's': case 't':
		case 'u': case 'v': case 'w': case 'x': case 'y':
		case 'z':
			/*
			 * optional characters
			 */
			if (ac == fc && arglex_compare(formal, actual))
			{
				result = 1;
				goto done;
			}
			/*
			 * skip forward to next
			 * mandatory character, or after '_'
			 */
			while (islower(*formal))
				++formal;
			if (*formal == '_')
			{
				++formal;
				if (ac == '_' || ac == '-')
					++actual;
			}
			--actual;
			break;

		case '*':
			/*
			 * This is a hack, it should really
			 * check for a match match the stuff after
			 * the '*', too, a la glob.
			 */
			if (!ac)
			{
				result = 0;
				goto done;
			}
			partial = actual - 1;
			result = 1;
			goto done;

		case '\\':
			if (actual[-1] != *formal++)
			{
				result = 0;
				goto done;
			}
			break;

		case 'A': case 'B': case 'C': case 'D': case 'E':
		case 'F': case 'G': case 'H': case 'I': case 'J':
		case 'K': case 'L': case 'M': case 'N': case 'O':
		case 'P': case 'Q': case 'R': case 'S': case 'T':
		case 'U': case 'V': case 'W': case 'X': case 'Y':
		case 'Z':
			fc = tolower(fc);
			/* fall through... */

		default:
			/*
			 * mandatory characters
			 */
			if (fc != ac)
			{
				result = 0;
				goto done;
			}
			break;
		}
	}
	done:
	trace(("return %d;\n}\n", result));
	return result;
}


/*
 * NAME
 *	is_a_number
 *
 * SYNOPSIS
 *	int is_a_number(char *s);
 *
 * DESCRIPTION
 *	The is_a_number function is used to determine if the
 *	argument is a number.
 *
 *	The value is placed in arglex_value.alv_number as
 *	a side effect.
 *
 *	Negative and positive signs are accepted.
 *	The C conventions for decimal, octal and hexadecimal are understood.
 *
 *	There may be no white space anywhere in the string,
 *	and the string must end after the last digit.
 *	Trailing garbage will be interpreted to mean it is not a string.
 *
 * ARGUMENTS
 *	s	- string to be tested and evaluated
 *
 * RETURNS
 *	int;	zero if not a number,
 *		non-zero if is a number.
 */

static int is_a_number _((char *));

static int
is_a_number(s)
	char		*s;
{
	long		n;
	int		sign;

	n = 0;
	switch (*s)
	{
	case '-':
		++s;
		sign = -1;
		break;

	case '+':
		++s;
		sign = 1;
		break;

	default:
		sign = 1;
		break;
	}
	switch (*s)
	{
	case '0':
		if ((s[1] == 'x' || s[1] == 'X') && s[2])
		{
			s += 2;
			for (;;)
			{
				switch (*s)
				{
				case '0': case '1': case '2': case '3':
				case '4': case '5': case '6': case '7':
				case '8': case '9':
					n = n * 16 + *s++ - '0';
					continue;

				case 'A': case 'B': case 'C':
				case 'D': case 'E': case 'F':
					n = n * 16 + *s++ - 'A' + 10;
					continue;

				case 'a': case 'b': case 'c':
				case 'd': case 'e': case 'f':
					n = n * 16 + *s++ - 'a' + 10;
					continue;
				}
				break;
			}
		}
		else
		{
			for (;;)
			{
				switch (*s)
				{
				case '0': case '1': case '2': case '3':
				case '4': case '5': case '6': case '7':
					n = n * 8 + *s++ - '0';
					continue;
				}
				break;
			}
		}
		break;

	case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
		for (;;)
		{
			switch (*s)
			{
			case '0': case '1': case '2': case '3':
			case '4': case '5': case '6': case '7':
			case '8': case '9':
				n = n * 10 + *s++ - '0';
				continue;
			}
			break;
		}
		break;

	default:
		return 0;
	}
	if (*s)
		return 0;
	arglex_value.alv_number = n * sign;
	return 1;
}


/*
 * NAME
 *	arglex
 *
 * SYNOPSIS
 *	arglex_token_ty arglex(void);
 *
 * DESCRIPTION
 *	The arglex function is used to perfom lexical analysis
 *	on the command line arguments.
 *
 *	Unrecognised options are returned as arglex_token_option
 *	for anything starting with a '-', or
 *	arglex_token_string otherwise.
 *
 * RETURNS
 *	The next token in the token stream.
 *	When the end is reached, arglex_token_eoln is returned forever.
 *
 * CAVEAT
 *	Must call arglex_init befor this function is called.
 */

arglex_token_ty
arglex()
{
	arglex_table_ty	*tp;
	int		j;
	arglex_table_ty	*hit[20];
	int		nhit;
	char		*arg;
	static char	*pushback[3];
	static int	pushback_depth;

	trace(("arglex()\n{\n"/*}*/));
	if (pushback_depth)
	{
		/*
		 * the second half of a "-foo=bar" style argument.
		 */
		arg = pushback[--pushback_depth];
	}
	else
	{
		if (argc <= 0)
		{
			arglex_token = arglex_token_eoln;
			arg = "";
			goto done;
		}
		arg = argv[0];
		argc--;
		argv++;

		/*
		 * See if it looks like a GNU "-foo=bar" option.
		 * Split it at the '=' to make it something the
		 * rest of the code understands.
		 */
		if (arg[0] == '-' && arg[1] != '=')
		{
			char	*eqp;

			eqp = strchr(arg, '=');
			if (eqp)
			{
				pushback[pushback_depth++] = eqp + 1;
				*eqp = 0;
			}
		}

		/*
		 * Turn the GNU-style leading "--"
		 * into "-" if necessary.
		 */
		if
		(
			arg[0] == '-'
		&&
			arg[1] == '-'
		&&
			arg[2]
		&&
			!is_a_number(arg + 1)
		)
			++arg;
	}

	/*
	 * see if it is a number
	 */
	if (is_a_number(arg))
	{
		arglex_token = arglex_token_number;
		goto done;
	}

	/*
	 * scan the tables to see what it matches
	 */
	nhit = 0;
	partial = 0;
	for (tp = table; tp < ENDOF(table); tp++)
	{
		if (arglex_compare(tp->name, arg))
			hit[nhit++] = tp;
	}
	if (utable)
	{
		for (tp = utable; tp->name; tp++)
		{
			if (arglex_compare(tp->name, arg))
				hit[nhit++] = tp;
		}
	}

	/*
	 * deal with unknown or ambiguous options
	 */
	switch (nhit)
	{
	case 0:
		/*
		 * not found in the tables
		 */
		if (*arg == '-')
			arglex_token = arglex_token_option;
		else
			arglex_token = arglex_token_string;
		break;

	case 1:
		if (partial)
			pushback[pushback_depth++] = (char *)partial;
		arg = hit[0]->name;
		arglex_token = hit[0]->token;
		break;

	default:
		{
			string_ty	*s1;
			string_ty	*s2;
			sub_context_ty	*scp;

			s1 = str_from_c(hit[0]->name);
			for (j = 1; j < nhit; ++j)
			{
				s2 = str_format("%S, %s", s1, hit[j]->name);
				str_free(s1);
				s1 = s2;
			}
			scp = sub_context_new();
			sub_var_set(scp, "Name", "%s", arg);
			sub_var_set(scp, "Guess", "%S", s1);
			fatal_intl
			(
				scp,
				i18n("option \"$name\" ambiguous ($guess)")
			);
			/* NOTREACHED */
		}
	}

	/*
	 * here for all exits
	 */
	done:
	arglex_value.alv_string = arg;
	trace(("return %d; /* \"%s\" */\n", arglex_token, arg));
	trace((/*{*/"}\n"));
	return arglex_token;
}


/*
 *  NAME
 *      arglex_init_from_env - initialize analyzer
 *
 *  SYNOPSIS
 *      void arglex_init_from_env(arglex_table_ty *);
 *
 *  DESCRIPTION
 *      The arglex_init_from_env function is used to initialize the command
 *      line lexical analyzer form the COOK environment variable.
 *
 *  RETURNS
 *      void
 *
 *  CAVEAT
 *      Must be called befor the arglex function.
 */

void
arglex_init_from_env(av0, tp)
	char		*av0;
	arglex_table_ty *tp;
{
	char		*cp1;
	char		*cp2;
	static size_t	ac;
	static char	**av;	/* static silences purify */

	trace(("arglex_init_from_env()\n{\n"));
	av0 = base_name(av0);
	cp1 = mem_alloc(strlen(av0) + 1);
	strcpy(cp1, av0);
	for (cp2 = cp1; *cp2; ++cp2)
	{
		if (islower(*cp2))
			*cp2 = toupper(*cp2);
	}

	ac = 0;
	av = mem_alloc(2 * sizeof(char *));
	av[ac++] = av0;

	cp2 = getenv(cp1);
	mem_free(cp1);
	if (!cp2)
		goto done;

	/* make a copy so we don't damage the environment copy */
	cp1 = mem_alloc(strlen(cp2) + 1);
	strcpy(cp1, cp2);

	for (;;)
	{
		size_t	nbytes;

		while (*cp1 == ' ')
			++cp1;
		if (!*cp1)
			break;
		cp2 = cp1;
		while (*++cp1 != ' ' && *cp1)
			;
		if (*cp1)
			*cp1++ = 0;
		nbytes = (ac + 2) * sizeof(char *);
		av = mem_change_size(av, nbytes);
		av[ac++] = cp2;
	}
	done:
	av[ac] = 0;
	arglex_init(ac, av, tp);
	trace((/*{*/"}\n"));
}


char *
arglex_token_name(n)
	arglex_token_ty	n;
{
	arglex_table_ty	*tp;

	switch (n)
	{
	case arglex_token_eoln:
		return "end of command line";

	case arglex_token_number:
		return "number";

	case arglex_token_option:
		return "option";

	case arglex_token_stdio:
		return "standard input or output";

	case arglex_token_string:
		return "string";

	default:
		break;
	}
	for (tp = table; tp < ENDOF(table); tp++)
	{
		if (tp->token == n)
			return tp->name;
	}
	if (utable)
	{
		for (tp = utable; tp->name; tp++)
		{
			if (tp->token == n)
				return tp->name;
		}
	}

	assert(0);
	return "unknown command line token";
}

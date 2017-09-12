/*
 *	cook - file construction tool
 *	Copyright (C) 1999 Peter Miller;
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
 * MANIFEST: functions to manipulate regular expressions
 */

#include <ac/stddef.h>
#include <ac/regex.h>

#include <str/re.h>
#include <stracc.h>


int
str_re_match(formal, actual, error_callback)
	string_ty	*formal;
	string_ty	*actual;
	void		(*error_callback)_((const char *));
{
#ifdef HAVE_REGCOMP
	regex_t		preg;
	int		err;

	/*
	 * compile the regular expression
	 */
	err = regcomp(&preg, formal->str_text, REG_BASIC | REG_NOSUB);
	if (err != 0)
	{
		bomb:
		if (error_callback)
		{
			char		buffer[100];

			regerror(err, &preg, buffer, sizeof(buffer));
			error_callback(buffer);
		}
		return -1;
	}

	/*
	 * execute the regular expression
	 */
	err = regexec(&preg, actual->str_text, (size_t)0, NULL, 0);
	regfree(&preg);
	if (err == REG_NOMATCH)
		return 0;
	if (err != 0)
		goto bomb;
	return 1;
#else
	if (error_callback)
		error_callback("Regular expressions not available.");
	return -1;
#endif
}


string_ty *
str_re_substitute(lhs, rhs, actual, error_callback, how_many_times)
	string_ty	*lhs;
	string_ty	*rhs;
	string_ty	*actual;
	void		(*error_callback)_((const char *));
	int		how_many_times;
{
#ifdef HAVE_REGCOMP
	static stracc	sa;
	regex_t		preg;
	int		err;
	const char	*ip;
	const char	*ip_end;
	int		suppress_on_zero;

	/*
	 * compile the regular expression
	 */
	err = regcomp(&preg, lhs->str_text, REG_BASIC);
	if (err != 0)
	{
		bomb:
		if (error_callback)
		{
			char		buffer[100];

			regerror(err, &preg, buffer, sizeof(buffer));
			error_callback(buffer);
		}
		return NULL;
	}

	/*
	 * execute the regular expression
	 */
	if (how_many_times <= 0)
		how_many_times = actual->str_length + 1;
	ip = actual->str_text;
	ip_end = actual->str_text + actual->str_length;
	sa_open(&sa);
	suppress_on_zero = 0;
	while (ip < ip_end)
	{
		regmatch_t	match[10];
		char		*cp;

		err =
			regexec
			(
				&preg,
				ip,
				SIZEOF(match),
				match,
				(ip == actual->str_text ? 0 : REG_NOTBOL)
			);
		if (err == REG_NOMATCH)
			break;
		if (err)
			goto bomb;

		if (match[0].rm_eo == 0 && suppress_on_zero)
			goto yuck;

		/*
		 * copy the fixed portion
		 */
		sa_chars(&sa, ip, match[0].rm_so);

		/*
		 * replace the matched portion with the right hand side
		 */
		for (cp = rhs->str_text; *cp; ++cp)
		{
			regmatch_t	*rm;

			switch (*cp)
			{
			default:
				sa_char(&sa, *cp);
				break;

			case '&':
				rm = &match[0];
				sa_chars
				(
					&sa,
					ip + rm->rm_so,
					rm->rm_eo - rm->rm_so
				);
				break;

			case '\\':
				switch (*++cp)
				{
				default:
					sa_char(&sa, *cp);
					break;

				case '0': case '1': case '2': case '3':
				case '4': case '5': case '6': case '7':
				case '8': case '9':
					rm = &match[*cp - '0'];
					if (rm->rm_so < 0)
						break;
					sa_chars
					(
						&sa,
						ip + rm->rm_so,
						rm->rm_eo - rm->rm_so
					);
					break;

				case 0:
					--cp;
					break;
				}
				break;
			}
		}
		suppress_on_zero = 1;

		/*
		 * Move past the matched portion.
		 */
		ip += match[0].rm_eo;

		/*
		 * There is a nasty boundary condition: we need to
		 * move past the infinite loop of a zero-length match.
		 * (This can happen for non-trivial patterns.)
		 */
		if (match[0].rm_so == match[0].rm_eo)
		{
			yuck:
			if (ip >= ip_end)
				break;
			sa_char(&sa, *ip++);
			suppress_on_zero = 0;
		}

		/*
		 * Limit how many times we go through this loop.
		 */
		if (--how_many_times <= 0)
			break;
	}

	/*
	 * Collect the tail-end of the input.
	 */
	sa_chars(&sa, ip, ip_end - ip);
	regfree(&preg);

	/*
	 * Build ther answer.
	 */
	return sa_close(&sa);
#else
	if (error_callback)
		error_callback("Regular expressions not available.");
	return NULL;
#endif
}

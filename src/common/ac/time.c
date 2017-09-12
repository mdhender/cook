/*
 *	cook - file construction tool
 *	Copyright (C) 1994, 1996, 1997, 2001 Peter Miller;
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
 * MANIFEST: impliment missing functions from <time.h>
 */

#include <ac/time.h>


/*
 *  NAME
 *      strftime - string from time
 *
 *  SYNOPSIS
 *      size_t strftime(char *s, size_t maxsize, const char *format,
 *		const struct tm *timeptr);
 *
 *  DESCRIPTION
 *      The strftime function places characters into the array pointed to
 *	by s as controlled by the string pointed to by format.  The format
 *	string consists of zero or more directives and ordinary characters.
 *	A directive consists of a % character followed by a character that
 *	determined the dirrective's behaviour.  All ordinary characters
 *	(including the terminating null character) are copied unchanged
 *	into the array.  No more thaqn maxsize characters are placed into
 *	the array.  Each directive is replaced by appropriate characters as
 *	described in the following list.  The appropriate characters are
 *	determined by the program's locale and by the values contained in
 *	the structure pointed to by timeptr.
 *
 *	%a is replaced by the locale's abbreviated weekday name
 *	%A is replaced by the locale's full weekday name
 *	%b is replaced by the locale's abbreviated month name
 *	%B is replaced by the locale's full month name
 *	%c is replaced by the locale's appropriae date and time representation
 *	%d is replaced by the day of the month as decimal number (01-31)
 *	%H is replaced by the hour (24-hour clock) as decimal number (00-23)
 *	%I is replaced by the hour (12-hour clock) as decimal number (01-12)
 *	%j is replaced by the day of te year as decimal number (001-366)
 *	%m is replaced by the month as decomal number (01-12)
 *	%M is replaced by the minute as decimal number (00-59)
 *	%p is replaced by the locale's equivalent of either AM or PM
 *	%S is replaced by the second as decimal number (00-59)
 *	%U is replaced by the week number of they year (Sunday as the first
 *		day of the week) as decimal number (00-52)
 *	%w is replaced by the weekday as decimal number (0=Sunday to 6=Saturday)
 *	%W is replaced by the week number of the year (Monday as the first
 *		day of the week) as decimal number (00-52)
 *	%x is replaced by the locale's appropriate date representation
 *	%X is replaced by the locale's appropriate time representation
 *	%y is replaced by the year without century as decimal number (00-99)
 *	%Y is replaced by the year with century as decimal number
 *	%Z is replaced by the time zone name, or no characters in no time
 *		zone name is available
 *	%% is replaced by %
 *
 *  RETURNS
 *	If the total number of resulting characters including the terminating
 *	null character is not more than maxsize, the strftime function returns
 *	the number of characters placed into the array pointed to by s not
 *	including the terminating null character.  Otherwise, zero is returned
 *	and the contents of the array are indeterminate.
 *
 *  CAVEAT
 *      This suffers from a serious design flaw: there is no way to
 *	distinguish between a result which is the empty string, and a result
 *	which is more than maxsize characters.
 *
 *	The behaviour for unknow directivbes is not only undefined,
 *	it is unmentioned!  (Normally the standard specifically allows
 *	implementation defined behaviour on weird boundary conditions.)
 *	This implementation will echo unknown directives into the output.
 */

#ifndef HAVE_STRFTIME

#ifndef HAVE_tm_zone
extern char *tzname[2];
#endif

size_t
strftime(buf, max, fmt, tm)
	char		*buf;
	size_t		max;
	char		*fmt;
	struct tm	*tm;
{
	char		*cp;
	char		*end;
	char		output[1000];
	int		n;
	size_t		len;

	static char *weekday[] =
	{
		"Sunday",
		"Monday",
		"Tuesday",
		"Wednesday",
		"Thursday",
		"Friday",
		"Saturday",
	};

	static char *month[] =
	{
		"January",
		"February",
		"March",
		"April",
		"May",
		"June",
		"July",
		"August",
		"September",
		"October",
		"November",
		"December",
	};

	end = buf + max - 1;
	cp = buf;
	while (*fmt)
	{
		if (*fmt++ != '%')
		{
			if (cp >= end)
				return 0;
			*cp++ = fmt[-1];
			continue;
		}
		switch (*fmt++)
		{
		case 0:
			--fmt;
			output[0] = '%';
			output[1] = 0;
			break;

		default:
			output[0] = '%';
			output[1] = fmt[-1];
			output[2] = 0;
			break;

		case '%':
			output[0] = '%';
			output[1] = 0;
			break;

		case 'a':
			/*
			 * the abbreviated weekday name
			 */
			sprintf(output, "%3.3s", weekday[tm->tm_wday]);
			break;

		case 'A':
			/*
			 * the full weekday name
			 */
			strcpy(output, weekday[tm->tm_wday]);
			break;

		case 'b':
		case 'h':
			/*
			 * the abbreviated month name
			 */
			sprintf(output, "%3.3s", month[tm->tm_mon]);
			break;

		case 'B':
			/*
			 * the full month name
			 */
			strcpy(output, month[tm->tm_mon]);
			break;

		case 'c':
			/*
			 * the date and time
			 */
			len =
				strftime
				(
					output,
					sizeof(output),
					"%b %e %X %Y",
					tm
				);
			if (!len)
				output[0] = 0;
			break;

		case 'C':
			/*
			 * This looks like a Sun extra.
			 * Local date.
			 */
			len =
				strftime
				(
					output,
					sizeof(output),
					"%A, %B %e, %Y",
					tm
				);
			if (!len)
				output[0] = 0;
			break;

		case 'd':
			/*
			 * the day of the month,
			 * zero padded.
			 */
			sprintf(output, "%2.2d", tm->tm_mday);
			break;

		case 'D':
			/*
			 * This looks like a Sun extra.
			 * Local date.
			 */
			len = strftime(output, sizeof(output), "%m/%d/%y", tm);
			if (!len)
				output[0] = 0;
			break;

		case 'e':
			/*
			 * This looks like a Sun extra.
			 * the day of the month,
			 * blank padded.
			 */
			sprintf(output, "%2d", tm->tm_mday);
			break;

		case 'H':
			/*
			 * the hour of a 24-hour day
			 * zero padded
			 */
			sprintf(output, "%2.2d", tm->tm_hour);
			break;

		case 'I':
			/*
			 * the hour of a 12-hour day,
			 * zero padded
			 */
			n = tm->tm_hour % 12;
			sprintf(output, "%2.2d", n ? n : 12);
			break;

		case 'j':
			/*
			 * the day of the year,
			 * zero padded, one based
			 */
			sprintf(output, "%3.3d", tm->tm_yday + 1);
			break;

		case 'k':
			/*
			 * This looks like a Sun extra.
			 * the hour of the 24-hour day,
			 * blank padded.
			 */
			sprintf(output, "%2d", tm->tm_hour);
			break;

		case 'l':
			/*
			 * This looks like a Sun extra.
			 * the hour of the 12-hour day,
			 * blank padded.
			 */
			n = tm->tm_hour % 12;
			sprintf(output, "%2d", n ? n : 12);
			break;

		case 'm':
			/*
			 * the month of the year,
			 * zero padded, one based.
			 */
			sprintf(output, "%2.2d", tm->tm_mon + 1);
			break;

		case 'M':
			/*
			 * the minute of the hour,
			 * zero padded
			 */
			sprintf(output, "%2.2d", tm->tm_min);
			break;

		case 'n':
			/*
			 * This looks like a Sun extra.
			 * like \n
			 */
			output[0] = '\n';
			output[1] = 0;
			break;

		case 'p':
			/*
			 * meridian indicator
			 */
			if (tm->tm_hour >= 12)
				strcpy(output, "PM");
			else
				strcpy(output, "AM");
			break;

		case 'r':
			/*
			 * this looks like a Sun extra.
			 * like %X, but 12-hour clock with meridian.
			 */
			len =
				strftime
				(
					output,
					sizeof(output),
					"%I:%M:%S %p",
					tm
				);
			if (!len)
				output[0] = 0;
			break;

		case 'R':
			/*
			 * this looks like a Sun extra.
			 * the 24-hour time as HH:MM
			 */
			len = strftime(output, sizeof(output), "%H:%M", tm);
			if (!len)
				output[0] = 0;
			break;

		case 'S':
			/*
			 * seconds of the minute
			 */
			sprintf(output, "%2.2d", tm->tm_sec);
			break;

		case 't':
			/*
			 * this looks like a Sun extra.
			 * like \t
			 */
			output[0] = '\t';
			output[1] = 0;
			break;

		case 'T':
			/*
			 * This looks like a Sun extra.
			 * the 24-hour time as HH:MM:SS
			 */
			len = strftime(output, sizeof(output), "%H:%M:%S", tm);
			if (!len)
				output[0] = 0;
			break;

		case 'U':
			/*
			 * the Sunday week of the year
			 */
			n = (tm->tm_yday - tm->tm_wday + 5) / 7;
			sprintf(output, "%2.2d", n);
			break;

		case 'w':
			/*
			 * the day of the week,
			 * Sunday = 0
			 */
			sprintf(output, "%d", tm->tm_wday);
			break;

		case 'W':
			/*
			 * the Monday week of the year
			 */
			n = (tm->tm_yday - ((tm->tm_wday + 6) % 7) + 5) / 7;
			sprintf(output, "%2.2d", n);
			break;

		case 'x':
			/*
			 * the date, as mmm dd yyyy
			 */
			len = strftime(output, sizeof(output), "%b %d %Y", tm);
			if (!len)
				output[0] = 0;
			break;

		case 'X':
			/*
			 * the time as hh:mm:ss
			 */
			len = strftime(output, sizeof(output), "%H:%M:%S", tm);
			if (!len)
				output[0] = 0;
			break;

		case 'y':
			/*
			 * the year of the century
			 */
			sprintf(output, "%2.2d", tm->tm_year % 100);
			break;

		case 'Y':
			/*
			 * the year including century
			 */
			sprintf(output, "%4.4d", tm->tm_year + 1900);
			break;

		case 'Z':
			/*
			 * the timezone name, if any
			 */
#ifndef HAVE_tm_zone
			if (tm->tm_isdst >= 0 && tm->tm_isdst <= 1)
				strcpy(output, tzname[tm->tm_isdst]);
			else
				output[0] = 0;
#else
			/* Berkeley derivatives have extra tm field */
			strcpy(output, tm->tm_zone);
#endif
			break;
		}

		/*
		 * make sure it fits in the buffer
		 */
		len = strlen(output);
		if (cp + len > end)
			return -1;
		memcpy(cp, output, len);
		cp += len;
	}
	*cp = 0;
	return (cp - buf);
}

#endif /* !HAVE_STRFTIME */

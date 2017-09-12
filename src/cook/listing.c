/*
 *	cook - file construction tool
 *	Copyright (C) 1993, 1994, 1997, 1998, 1999, 2000, 2001 Peter Miller;
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
 * MANIFEST: functions to open and close the listing file
 */

#include <ac/stddef.h>
#include <ac/stdio.h>
#include <ac/string.h>
#include <ac/time.h>
#include <ac/signal.h>
#include <ac/unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <error_intl.h>
#include <listing.h>
#include <os_interface.h>
#include <os/wait.h>
#include <option.h>
#include <quit.h>
#include <trace.h>


static int pid;


#ifdef HAVE_GETPGRP
#ifndef HAVE_TCGETPGRP

#include <sys/termio.h>

int
tcgetpgrp(fd)
	int		fd;
{
	int		result;

#ifdef TIOCGETPGRP
	if (ioctl(fd, TIOCGETPGRP, &result))
		result = -1;
#else
#ifdef TIOCGPGRP
	if (ioctl(fd, TIOCGPGRP, &result))
		result = -1;
#else
	result = -1;
#endif
#endif
	return result;
}

#endif /* !HAVE_TCGETPGRP */
#endif /* HAVE_GETPGRP */


/*
 * NAME
 *	background - test for backgroundness
 *
 * SYNOPSIS
 *	int background(void);
 *
 * DESCRIPTION
 *	The background function is used to determin e if the curent process is
 *	in the background.
 *
 * RETURNS
 *	int: zero if process is not in the background, nonzero if the process
 *	is in the background.
 *
 * CAVEAT
 *	This function has a huge chance of being wrong for your system.
 *	If you need to modify this function, please let the author know.
 */

static int background _((void));

static int
background()
{
	RETSIGTYPE (*x)_((int));

	/*
	 * csh changes the progess group of jobs.
	 * you are a background job if the terminal is not
	 * in your process group.
	 */
#ifdef HAVE_GETPGRP
	if (getpgrp(CONF_getpgrp_arg) != tcgetpgrp(0))
		return 1;
#endif

	/*
	 * bourne shell tells you to ignore interrupts
	 */
	x = signal(SIGINT, SIG_IGN);
	if (x == SIG_IGN)
		return 1;
	signal(SIGINT, x);

	/*
	 * must be forground
	 */
	return 0;
}


/*
 * NAME
 *	log_close - terminate logging
 *
 * SYNOPSIS
 *	void log_close(void);
 *
 * DESCRIPTION
 *	Log_close is used to terminate logging this session,
 *	and to close any como or comi files opened.
 *
 * CAVEAT
 *	Do not call any of the fatal error functions
 *	from this function.
 */

static void log_close _((void));

static void
log_close()
{
	if (pid)
	{
		fclose(stdout);
		fclose(stderr);
		for (;;)
		{
			int who;
			int status;

			who = os_waitpid(pid, &status);
			if (who < 0 || who == pid)
				break;
		}
		pid = 0;
	}
}


/*
 * NAME
 *	log_open - start logging this session
 *
 * SYNOPSIS
 *	void log_open(void);
 *
 * DESCRIPTION
 *	Log_open is used to commence logging a cook session.
 */

void
log_open()
{
	string_ty	*fullpath;
	sub_context_ty	*scp;

	trace(("log_open()\n{\n"/*}*/));

	/*
	 * If we are logging the output to a file
	 * and we are in the background,
	 * don't send the output to the terminal.
	 */
	if (option_test(OPTION_LOGGING) && background())
		option_set(OPTION_TERMINAL, OPTION_LEVEL_COMMAND_LINE, 0);

	/*
	 * redirect the output depending on the flags
	 */
	if (option_test(OPTION_LOGGING))
	{
		if (!option.o_logfile)
			fatal_intl(0, i18n("no list file specified"));
		if (option_test(OPTION_TERMINAL))
		{
			int	fd[2];
			char	*cmd[3];

			/*
			 * list both to a file and to the terminal
			 */
			if (pipe(fd))
			{
				scp = sub_context_new();
				sub_errno_set(scp);
				fatal_intl(scp, i18n("pipe(): $errno"));
				/* NOTREACHED */
			}
			switch (pid = fork())
			{
			case 0:
				cmd[0] = "tee";
				cmd[1] = option.o_logfile->str_text;
				cmd[2] = 0;
				close(fd[1]);
				close(0);
				if (dup(fd[0]) != 0)
					fatal_intl(0, i18n("dup was wrong"));
				close(fd[0]);
				signal(SIGINT, SIG_IGN);
				signal(SIGHUP, SIG_IGN);
				signal(SIGTERM, SIG_IGN);
				execvp(cmd[0], cmd);
				scp = sub_context_new();
				sub_errno_set(scp);
				sub_var_set(scp, "File_Name", "%s", cmd[0]);
				fatal_intl(scp, i18n("exec $filename: $errno"));
				/* NOTREACHED */

			case -1:
				scp = sub_context_new();
				sub_errno_set(scp);
				fatal_intl(scp, i18n("fork(): $errno"));
				/* NOTREACHED */

			default:
				close(fd[0]);
				close(1);
				if (dup(fd[1]) != 1)
					fatal_intl(0, i18n("dup was wrong"));
				close(fd[1]);
				break;
			}
		}
		else
		{
			/*
			 * list only to a file
			 */
			if (!freopen(option.o_logfile->str_text, "w", stdout))
				fatal_intl_open(option.o_logfile->str_text);
		}
		/*
		 * make sterr go to the same place as stdout
		 *	[will this work if stdout is already closed?]
		 */
		close(2);
		switch (dup(1))
		{
		case 0:
			/* oops, stdin is was closed */
			if (dup(1) != 2)
				fatal_intl(0, i18n("dup was wrong"));
			close(0);
			break;

		case 2:
			break;

		default:
			scp = sub_context_new();
			sub_errno_set(scp);
			fatal_intl(scp, i18n("dup(): $errno"));
			/* NOTREACHED */
		}
		fullpath = os_pathname(option.o_logfile);
		if (fullpath)
		{
			string_ty	*s;

			scp = sub_context_new();
			sub_var_set(scp, "File_Name", "%S", fullpath);
			s = subst_intl(scp, i18n("/* $filename */"));
			sub_context_delete(scp);
			fputs(s->str_text, stderr);
			fputc('\n', stderr);
			str_free(fullpath);
			str_free(s);
		}
	}
	else
	{
		if (option_test(OPTION_TERMINAL))
		{
			/*
			 * list only to the terminal
			 */
		}
		else
		{
			static char dev_null[] = "/dev/null";

			/*
			 * list neither to a file nor to the terminal
			 */
			if
			(
				!freopen(dev_null, "w", stdout)
			||
				!freopen(dev_null, "w", stderr)
			)
			{
				fatal_intl_open(dev_null);
			}
		}
	}

	quit_handler(log_close);
	trace((/*{*/"}\n"));
}

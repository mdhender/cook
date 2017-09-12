/*
 *	cook - file construction tool
 *	Copyright (C) 2000, 2001 Peter Miller;
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
 * MANIFEST: functions to manipulate waits
 */

#include <ac/stddef.h>
#include <sys/types.h>
#if defined(HAVE_WAIT3) || defined(HAVE_WAIT4)
#include <sys/time.h>
#include <sys/resource.h>
#endif
#include <sys/wait.h>

#include <mem.h>
#include <os/wait.h>
#include <trace.h>


typedef struct cache_ty cache_ty;
struct cache_ty
{
	int		pid;
	int		status;
#if defined(HAVE_WAIT3) || defined(HAVE_WAIT4)
	struct rusage	rusage;
#endif
};

static	size_t		length;
static	size_t		length_max;
static	cache_ty	*cache;


/*
 * NAME
 *	os_wait4
 *
 * SYNOPSIS
 *	int os_wait4(int pid, int *status, int options, struct rusage *rusage);
 *
 * DESCRIPTION
 *	The os_wait4 function suspends execution of its calling process
 *	until exit status information and accumulated resource utilization
 *	statistics are available for a terminated child process, or a
 *	signal is received.
 *
 *	Other os_wait* functions are implemented using os_wait4().
 *
 * ARGUMENTS
 *	The pid parameter specifies the set of child processes for which
 *	to wait.  If pid is -1, the call waits for any child process.
 *	If pid is greater than zero, the call waits for the process with
 *	a process id of pid.  Values of pid of 0 or less than -1 are
 *	not supported (this is asymetric with the system wait4 function.)
 *
 *	The status parameter is as defined for the system wait4() function.
 *
 *	The options parameter is as defined for the system wait4() function.
 *
 *	If the rusage parameter is non-zero, a summary of the resources
 *	used by the terminated process and all its children is returned.
 *
 * RETURNS
 *	If an error or interrupt occurs, os_wait4() returns a value of -1,
 *	and errno is set suitably.  When the WNOHANG option is specified and
 *	no processes wish to report status, os_wait4() returns a process
 *	id of 0.  Otherwise, it returns the process id of the relevant
 *	child process.
 */

int
os_wait4(pid, status, options, rusage)
	int		pid;
	int		*status;
	int		options;
	struct rusage	*rusage;
{
	cache_ty	*cp;
	int		pid2;

	trace(("os_wait4(pid = %d, status = %08lX, options = 0x%X, \
rusage = %08lX)\n{\n",
		pid, status, options, rusage));
	assert(pid == -1 || pid > 0);
	assert(status);
	if (pid == -1)
	{
		if (length > 0)
		{
			trace(("use the cache...\n"));
			--length;
			cp = &cache[length];
			*status = cp->status;
#if defined(HAVE_WAIT3) || defined(HAVE_WAIT4)
			if (rusage)
				*rusage = cp->rusage;
#endif
			trace(("return %d;\n", cp->pid));
			trace(("}\n"));
			return cp->pid;
		}
		else
		{
			trace(("ask the system...\n"));
#ifdef HAVE_WAIT4
			pid2 = wait4(-1, status, options, rusage);
#else /* !HAVE_WAIT4 */
#ifdef HAVE_WAIT3
			pid2 = wait3(status, options, rusage);
#else /* !HAVE_WAIT3 */
			assert(options == 0);
			assert(rusage == 0);
			pid2 = wait(status);
#endif /* !HAVE_WAIT3 */
#endif /* !HAVE_WAIT4 */
			trace(("return %d;\n", pid2));
			trace(("}\n"));
			return pid2;
		}
	}
	else
	{
		size_t		j;

		/*
		 * Check to see if we already have it in the cache.
		 * (Plug the hole with the last entry.)
		 */
		for (j = 0; j < length; --j)
		{
			cp = &cache[j];
			if (pid == cp->pid)
			{
				trace(("use the cache...\n"));
				pid2 = cp->pid;
				*status = cp->status;
#if defined(HAVE_WAIT3) || defined(HAVE_WAIT4)
				if (rusage)
					*rusage = cp->rusage;
#endif
				if (j + 1 < length)
					cache[j] = cache[length - 1];
				--length;
				trace(("return %d;\n", pid2));
				trace(("}\n"));
				return pid2;
			}
		}

		/*
		 * Watch them go past.
		 */
		for (;;)
		{
#if defined(HAVE_WAIT3) || defined(HAVE_WAIT4)
			struct rusage	rusage2;
#endif
			int		status2;

			/*
			 * ask the operating system to tell us what happened
			 */
			trace(("ask the system...\n"));
#ifdef HAVE_WAIT4
			pid2 = wait4(-1, &status2, options, &rusage2);
#else /* !HAVE_WAIT4 */
#ifdef HAVE_WAIT3
			pid2 = wait3(&status2, options, &rusage2);
#else /* !HAVE_WAIT3 */
			assert(options == 0);
			assert(rusage == 0);
			pid2 = wait(&status2);
#endif /* !HAVE_WAIT3 */
#endif /* !HAVE_WAIT4 */
			/*
			 * Return on -1, for all errors.
			 * Return on 0, for the WNOHANG option.
			 */
			if (pid2 <= 0)
			{
				trace(("return %d;\n", pid2));
				trace(("}\n"));
				return pid2;
			}

			/*
			 * Stop if this is the process we were waiting for.
			 */
			if (pid2 == pid)
			{
				*status = status2;
#if defined(HAVE_WAIT3) || defined(HAVE_WAIT4)
				if (rusage)
					*rusage = rusage2;
#endif
				trace(("return %d;\n", pid2));
				trace(("}\n"));
				return pid2;
			}

			/*
			 * make sure we have enough room in the cache
			 */
			if (length >= length_max)
			{
				size_t		nbytes;

				length_max = length_max * 2 + 4;
				nbytes = length_max * sizeof(cache[0]);
				cache = mem_change_size(cache, nbytes);
			}

			/*
			 * stash the results of the wait into the cache
			 */
			trace(("append to the cache...\n"));
			cp = &cache[length++];
			cp->pid = pid2;
			cp->status = status2;
#if defined(HAVE_WAIT3) || defined(HAVE_WAIT4)
			cp->rusage = rusage2;
#endif
		}
	}
}



/*
 * NAME
 *	os_wait3
 *
 * SYNOPSIS
 *	int os_wait3(int *status, int options, struct rusage *rusage);
 *
 * DESCRIPTION
 *	The os_wait3 function suspends execution of its calling process
 *	until exit status information and accumulated resource utilization
 *	statistics are available for a terminated child process, or a
 *	signal is received.
 *
 * ARGUMENTS
 *	The status parameter is as defined for the system wait3() function.
 *
 *	The options parameter is as defined for the system wait3() function.
 *
 *	If the rusage parameter is non-zero, a summary of the resources
 *	used by the terminated process and all its children is returned.
 *
 * RETURNS
 *	If an error or interrupt occurs, os_wait3() returns a value of -1,
 *	and errno is set suitably.  When the WNOHANG option is specified and
 *	no processes wish to report status, os_wait3() returns a process
 *	id of 0.  Otherwise, it returns the process id of the relevant
 *	child process.
 */

int
os_wait3(status, options, rusage)
	int		*status;
	int		options;
	struct rusage	*rusage;
{
	return os_wait4(-1, status, options, rusage);
}


/*
 * NAME
 *	os_waitpid
 *
 * SYNOPSIS
 *	int os_waitpid(int pid, int *status);
 *
 * DESCRIPTION
 *	The os_waitpid function suspends execution of its calling process
 *	until exit status information and accumulated resource utilization
 *	statistics are available for the specified child process, or a
 *	signal is received.
 *
 * ARGUMENTS
 *	The pid parameter specifies the set of child processes for which
 *	to wait.  If pid is -1, the call waits for any child process.
 *	If pid is greater than zero, the call waits for the process with
 *	a process id of pid.  Values of pid of 0 or less than -1 are
 *	not supported (this is asymetric with the system waitpid function.)
 *
 *	The status parameter is as defined for the system waitpid() function.
 *
 * RETURNS
 *	If an error or interrupt occurs, os_waitpid() returns a value
 *	of -1, and errno is set suitably.  Otherwise, it returns the
 *	process id of the terminated child process.
 */

int
os_waitpid(pid, status)
	int		pid;
	int		*status;
{
	return os_wait4(pid, status, 0, (struct rusage *)0);
}


/*
 * NAME
 *	os_wait
 *
 * SYNOPSIS
 *	int os_wait(int *status);
 *
 * DESCRIPTION
 *	The os_wait function suspends execution of its calling process
 *	until exit status information and accumulated resource utilization
 *	statistics are available for a terminated child process, or a
 *	signal is received.
 *
 * ARGUMENTS
 *	The status parameter is as defined for the system wait() function.
 *
 * RETURNS
 *	If an error or interrupt occurs, os_wait() returns a value of -1,
 *	and errno is set suitably.  Otherwise, it returns the process
 *	id of the a terminated child process.
 */

int
os_wait(status)
	int		*status;
{
	return os_wait4(-1, status, 0, (struct rusage *)0);
}

/*
 *	cook - file construction tool
 *	Copyright (C) 1997-2001, 2003 Peter Miller;
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
 * MANIFEST: functions to manipulate opcode contexts
 */

#include <ac/errno.h>
#include <ac/stddef.h>
#include <sys/wait.h>

#include <desist.h>
#include <error_intl.h>
#include <id.h>
#include <id/global.h>
#include <id/variable.h>
#include <match/stack.h>
#include <mem.h>
#include <meter.h>
#include <opcode.h>
#include <opcode/context.h>
#include <opcode/list.h>
#include <opcode/thread-id.h>
#include <option.h>
#include <os/wait.h>
#include <str_list.h>
#include <symtab.h>
#include <trace.h>


/*
 * NAME
 *	opcode_context_new
 *
 * SYNOPSIS
 *	opcode_context_ty *opcode_context_new(opcode_list_ty *);
 *
 * DESCRIPTION
 *	The opcode_context_new function is used to create a new instance
 *	of an opcode context in dynamic memory, for the purpose of
 *	executing the given opcode stream.
 *
 * CAVEAT
 *	Use opcode_context_delete when you are done with it.
 *
 *	The opcode list is expected to endure until executing
 *	terminates.  The reference count in the opcode list is not
 *	exploited.
 */

opcode_context_ty *
opcode_context_new(olp, mp)
	opcode_list_ty	*olp;
	const match_ty	*mp;
{
	opcode_context_ty *ocp;

	trace(("opcode_context_new(olp = %08lX)\n{\n"/*}*/, (long)olp));
	ocp = mem_alloc(sizeof(opcode_context_ty));
	ocp->call_stack_length = 0;
	ocp->call_stack_maximum = 0;
	ocp->call_stack = 0;
	ocp->value_stack_length = 0;
	ocp->value_stack_maximum = 0;
	ocp->value_stack = 0;
	ocp->thread_id = opcode_thread_id_borrow();
	ocp->msp = 0;
	ocp->pid = 0;
	ocp->exit_status = 0;
	ocp->meter_p = 0;
	ocp->wlp = 0;
	ocp->need_age = 0;
	ocp->flags = 0;
	ocp->mp = 0;
	ocp->thread_stp = 0;
	ocp->host_binding = 0;
	ocp->gp = 0;

	opcode_context_match_push(ocp, mp);
	if (olp)
		opcode_context_call(ocp, olp);
	trace((/*{*/"}\n"));
	return ocp;
}


/*
 * NAME
 *	opcode_context_delete
 *
 * SYNOPSIS
 *	void opcode_context_delete(opcode_context_ty *);
 *
 * DESCRIPTION
 *	The opcode_context_delete function is used to release the
 *	resources held by an opcode context.
 */

void
opcode_context_delete(ocp)
	opcode_context_ty *ocp;
{
	trace(("opcode_context_delete(ocp = %08lX)\n{\n"/*}*/, (long)ocp));
	if (ocp->call_stack)
		mem_free(ocp->call_stack);
	ocp->call_stack_length = 0;
	ocp->call_stack_maximum = 0;
	ocp->call_stack = 0;
	while (ocp->value_stack_length > 0)
	{
		string_list_ty	*slp;

		slp = opcode_context_string_list_pop(ocp);
		string_list_delete(slp);
	}
	if (ocp->value_stack)
		mem_free(ocp->value_stack);
	ocp->value_stack_length = 0;
	ocp->value_stack_maximum = 0;
	ocp->value_stack = 0;
	if (ocp->meter_p)
		meter_free(ocp->meter_p);
	if (ocp->thread_stp)
		symtab_free(ocp->thread_stp);
	if (ocp->msp)
		match_stack_delete(ocp->msp);

	trace(("mark\n"));
	if (ocp->flags)
		mem_free(ocp->flags);
	/* do not free the match pointer, it does not belong to us */
	if (ocp->host_binding)
		str_free(ocp->host_binding);

	opcode_thread_id_return(ocp->thread_id);

	mem_free(ocp);
	trace((/*{*/"}\n"));
}


/*
 * NAME
 *	opcode_context_execute_inner
 *
 * SYNOPSIS
 *	opcode_status_ty opcode_context_execute_inner(opcode_context_ty *ocp,
 *		opcode_status_ty (*func)(const opcode_ty *,
 *		opcode_context_ty *));
 *
 * DESCRIPTION
 *	The opcode_context_execute_inner function is used to drive the
 *	execution of an opcode context until it terminates.
 *
 * RETURNS
 *	 opcode_status_ty to indicate the result
 *
 * CAVEAT
 *	Some termination states are restartable.
 */

static opcode_status_ty opcode_context_execute_inner _((opcode_context_ty *ocp,
	opcode_status_ty (*)(const opcode_ty *, opcode_context_ty *)));

static opcode_status_ty
opcode_context_execute_inner(ocp, func)
	opcode_context_ty *ocp;
	opcode_status_ty (*func)_((const opcode_ty *, opcode_context_ty *));
{
	opcode_status_ty status;

	/*
	 * keep executing until the last call exits
	 */
	trace(("opcode_context_execute(ocp = %08lX)\n{\n"/*}*/, (long)ocp));
	status = opcode_status_success;
	while (ocp->call_stack_length > 0)
	{
		opcode_frame_ty	*frame;
		const opcode_ty	*op;

		/*
		 * stop if we have been asked to
		 */
		if (desist_requested())
		{
			status = opcode_status_interrupted;
			break;
		}

		/*
		 * If we have run off the end of the opcodes, we have
		 * reached an implicit return.
		 */
		frame = &ocp->call_stack[ocp->call_stack_length - 1];
		if (frame->pc >= frame->olp->length)
		{
			trace(("rtn\n"));
			ocp->call_stack_length--;
			if (frame->stp)
				symtab_free(frame->stp);
			frame->stp = 0;
			frame->olp = 0;
			frame->pc = 0;
			continue;
		}

		/*
		 * run the opcode
		 */
		trace(("pc = %ld;\n", (long)frame->pc));
		op = frame->olp->list[frame->pc++];
		status = func(op, ocp);
		if (status != opcode_status_success)
		{
			/* back-up so can re-start */
			frame->pc--;
			break;
		}
	}
	trace(("return %s;\n", opcode_status_name(status)));
	trace((/*{*/"}\n"));
	return status;
}


/*
 * NAME
 *	opcode_context_execute
 *
 * SYNOPSIS
 *	opcode_status_ty opcode_context_execute(opcode_context_ty *);
 *
 * DESCRIPTION
 *	The opcode_context_execute function is used to execute an opcode
 *	context until it terminates.
 *
 * RETURNS
 *	opcode_status_ty to indicate the result
 *
 * CAVEAT
 *	Some termination states are restartable.
 */

opcode_status_ty
opcode_context_execute(ocp)
	opcode_context_ty *ocp;
{
	return opcode_context_execute_inner(ocp, opcode_execute);
}


/*
 * NAME
 *	opcode_context_execute_nowait
 *
 * SYNOPSIS
 *	opcode_status_ty opcode_context_execute_nowait(opcode_context_ty *);
 *
 * DESCRIPTION
 *	The opcode_context_execute_nowait function is used to execute an
 *	opcode context until it terminates.  You will never see the
 *	opcode_status_wait return code.
 *
 * RETURNS
 *	opcode_status_ty to indicate the result
 *
 * CAVEAT
 *	Some termination states are restartable.
 */

opcode_status_ty
opcode_context_execute_nowait(ocp)
	opcode_context_ty *ocp;
{
	opcode_status_ty status;

	trace(("opcode_context_execute_nowait(ocp = %08lX)\n{\n"/*}*/,
		(long)ocp));
	for (;;)
	{
		status = opcode_context_execute(ocp);
		switch (status)
		{
		case opcode_status_wait:
			for (;;)
			{
				int	pid;

				pid = os_waitpid(ocp->pid, &ocp->exit_status);
				if (pid < 0)
				{
					sub_context_ty	*scp;

					if (errno == EINTR)
						continue;
					scp = sub_context_new();
					sub_errno_set(scp);
					fatal_intl(scp, i18n("wait(): $errno"));
					/* NOTREACHED */
				}
				assert(pid == ocp->pid);
				break;
			}
			continue;

		case opcode_status_success:
		case opcode_status_error:
		case opcode_status_interrupted:
			break;
		}
		break;
	}
	trace(("return %s;\n", opcode_status_name(status)));
	trace((/*{*/"}\n"));
	return status;
}


/*
 * NAME
 *	opcode_context_script
 *
 * SYNOPSIS
 *	opcode_status_ty opcode_context_script(opcode_context_ty *);
 *
 * DESCRIPTION
 *	The opcode_context_script function is used to script an opcode
 *	context until it terminates.
 *
 * RETURNS
 *	opcode_status_ty to indicate the result
 */

opcode_status_ty
opcode_context_script(ocp)
	opcode_context_ty *ocp;
{
	return opcode_context_execute_inner(ocp, opcode_script);
}


/*
 * NAME
 *	opcode_context_call
 *
 * SYNOPSIS
 *	void opcode_context_call(opcode_context_ty *, opcode_list_ty *);
 *
 * DESCRIPTION
 *	The opcode_context_call function is used to perform a function
 *	call.  The call stack and value stack are separate.
 *
 * CAVEAT
 *	To be used only internally to the interpratation by individial
 *	opcodes.
 */

void
opcode_context_call(ocp, olp)
	opcode_context_ty *ocp;
	opcode_list_ty	*olp;
{
	opcode_frame_ty	*frame;

	trace(("opcode_context_call(ocp = %08lX, olp = %08lX)\n{\n"/*}*/,
		(long)ocp, (long)olp));
	if (ocp->call_stack_length >= ocp->call_stack_maximum)
	{
		size_t		nbytes;

		ocp->call_stack_maximum = ocp->call_stack_maximum * 2 + 4;
		nbytes = ocp->call_stack_maximum * sizeof(ocp->call_stack[0]);
		ocp->call_stack = mem_change_size(ocp->call_stack, nbytes);
	}
	frame = &ocp->call_stack[ocp->call_stack_length++];
	frame->olp = olp;
	frame->pc = 0;
	frame->stp = 0;
	trace((/*{*/"}\n"));
}


/*
 * NAME
 *	opcode_context_string_list_push
 *
 * SYNOPSIS
 *	void opcode_context_string_list_push(opcode_context_ty *);
 *
 * DESCRIPTION
 *	The opcode_context_string_list_push function is used to push a
 *	fresh string list onto the value stack.  This is the normal
 *	mechanism for accumulating arguments.  The call stack and value
 *	stack are separate.
 *
 * CAVEAT
 *	To be used only internally to the interpratation by individial
 *	opcodes.
 */

void
opcode_context_string_list_push(ocp)
	opcode_context_ty *ocp;
{
	trace(("opcode_context_string_list_push(ocp = %08lX)\n{\n"/*}*/,
		(long)ocp));
	assert(ocp);
	if (ocp->value_stack_length >= ocp->value_stack_maximum)
	{
		size_t		nbytes;

		ocp->value_stack_maximum = ocp->value_stack_maximum * 2 + 4;
		nbytes = ocp->value_stack_maximum * sizeof(ocp->value_stack[0]);
		ocp->value_stack = mem_change_size(ocp->value_stack, nbytes);
	}
	ocp->value_stack[ocp->value_stack_length++] = string_list_new();
	trace((/*{*/"}\n"));
}


/*
 * NAME
 *	opcode_context_string_push
 *
 * SYNOPSIS
 *	void opcode_context_string_push(opcode_context_ty *, string_ty *);
 *
 * DESCRIPTION
 *	The opcode_context_string_push function is used to append a
 *	string to the string list on the top of the value stack.  This
 *	is the normal mechanism for accumulating argument lists.
 *
 * CAVEAT
 *	To be used only internally to the interpratation by individial
 *	opcodes.
 */

void
opcode_context_string_push(ocp, s)
	opcode_context_ty *ocp;
	string_ty	*s;
{
	string_list_ty	*slp;

	trace(("opcode_context_string_push(ocp = %08lX)\n{\n"/*}*/, (long)ocp));
	assert(ocp);
	assert(ocp->value_stack_length > 0);
	slp = ocp->value_stack[ocp->value_stack_length - 1];
	string_list_append(slp, s);
	trace((/*{*/"}\n"));
}


void
opcode_context_string_push_list(ocp, i)
	opcode_context_ty *ocp;
	const string_list_ty *i;
{
	string_list_ty	*slp;

	trace(("opcode_context_string_push(ocp = %08lX)\n{\n"/*}*/, (long)ocp));
	assert(ocp);
	assert(ocp->value_stack_length > 0);
	slp = ocp->value_stack[ocp->value_stack_length - 1];
	string_list_append_list(slp, i);
	trace((/*{*/"}\n"));
}


/*
 * NAME
 *	opcode_context_string_list_pop
 *
 * SYNOPSIS
 *	string_list_ty *opcode_context_string_list_pop(opcode_context_ty *);
 *
 * DESCRIPTION
 *	The opcode_context_string_list_pop function is used to obtain
 *	the top-most string list from the value stack (it is removed
 *	from the stack).  This is the normal mechanism for obtaining
 *	argument lists.  Use string_list_delete when you are done with it.
 *
 * CAVEAT
 *	To be used only internally to the interpratation by individial
 *	opcodes.
 */

string_list_ty *
opcode_context_string_list_pop(ocp)
	opcode_context_ty *ocp;
{
	string_list_ty	*slp;

	trace(("opcode_context_string_list_pop(ocp = %08lX)\n{\n"/*}*/,
		(long)ocp));
	assert(ocp);
	assert(ocp->value_stack_length > 0);
	ocp->value_stack_length--;
	slp = ocp->value_stack[ocp->value_stack_length];
	trace(("return %08lX;\n", (long)slp));
	trace((/*{*/"}\n"));
	return slp;
}


string_list_ty *
opcode_context_string_list_peek(ocp)
	const opcode_context_ty *ocp;
{
	string_list_ty	*slp;

	trace(("opcode_context_string_list_peek(ocp = %08lX)\n{\n"/*}*/,
		(long)ocp));
	assert(ocp);
	assert(ocp->value_stack_length > 0);
	slp = ocp->value_stack[ocp->value_stack_length - 1];
	trace(("return %08lX;\n", (long)slp));
	trace((/*{*/"}\n"));
	return slp;
}


/*
 * NAME
 *	opcode_context_goto
 *
 * SYNOPSIS
 *	void opcode_context_goto(opcode_context_ty *, size_t);
 *
 * DESCRIPTION
 *	The opcode_context_goto function is used to move the execution
 *	location pointer.  This uis the normal ``jump'' mechanism.
 *
 * CAVEAT
 *	To be used only internally to the interpratation by individial
 *	opcodes.
 */

void
opcode_context_goto(ocp, pc)
	opcode_context_ty *ocp;
	size_t		pc;
{
	opcode_frame_ty	*frame;

	trace(("opcode_context_goto(ocp = %08lX, pc = %ld)\n{\n"/*}*/,
		(long)ocp, (long)pc));
	assert(ocp);
	assert(ocp->call_stack_length > 0);
	frame = &ocp->call_stack[ocp->call_stack_length - 1];
	assert(pc <= frame->olp->length);
	frame->pc = pc;
	trace((/*{*/"}\n"));
}


int
opcode_context_getpid(ocp)
	opcode_context_ty *ocp;
{
	return ocp->pid;
}


void
opcode_context_waited(ocp, es)
	opcode_context_ty *ocp;
	int		es;
{
	ocp->exit_status = es;
}


void
opcode_context_suspend(ocp)
	opcode_context_ty *ocp;
{

	/*
	 * save and clear flag state
	 */
	trace(("opcode_context_suspend(ocp = %08lX)\n{\n"/*}*/, (long)ocp));
	ocp->flags = option_flag_state_get();
	option_undo_level(OPTION_LEVEL_EXECUTE);
	option_undo_level(OPTION_LEVEL_RECIPE);
	trace((/*{*/"}\n"));
}


void
opcode_context_resume(ocp)
	opcode_context_ty *ocp;
{
	/*
	 * restore flag state
	 */
	trace(("opcode_context_resume(ocp = %08lX)\n{\n"/*}*/, (long)ocp));
	assert(ocp->flags);
	if (ocp->flags)
		option_flag_state_set(ocp->flags);
	ocp->flags = 0;
	trace((/*{*/"}\n"));
}


void
opcode_context_host_binding_set(ocp, host_binding)
	opcode_context_ty *ocp;
	string_ty	*host_binding;
{
	assert(ocp);
	assert(host_binding);
	assert(!ocp->host_binding);
	ocp->host_binding = str_copy(host_binding);
}


const match_ty *
opcode_context_match_top(ocp)
	const opcode_context_ty *ocp;
{
	if (!ocp->msp)
		return 0;
	return match_stack_top(ocp->msp);
}


const match_ty *
opcode_context_match_pop(ocp)
	opcode_context_ty *ocp;
{
	assert(ocp->msp);
	return match_stack_pop(ocp->msp);
}


void
opcode_context_match_push(ocp, mp)
	opcode_context_ty *ocp;
	const match_ty	*mp;
{
	if (!ocp->msp)
		ocp->msp = match_stack_new();
	match_stack_push(ocp->msp, mp);
}


void
opcode_context_id_assign(ocp, name, value, local)
	opcode_context_ty *ocp;
	string_ty	*name;
	id_ty		*value;
	int		local;
{
	opcode_frame_ty	*frame;

	assert(ocp);
	if (ocp->call_stack_length < 1)
		frame = 0;
	else
		frame = &ocp->call_stack[ocp->call_stack_length - 1];
	if (local > 0 && frame)
	{
		if (!frame->stp)
		{
			frame->stp = symtab_alloc(5);
			frame->stp->reap = id_global_reap;
		}
		symtab_assign(frame->stp, name, value);
	}
	else if (local < 0)
	{
		if (!ocp->thread_stp)
		{
			ocp->thread_stp = symtab_alloc(5);
			ocp->thread_stp->reap = id_global_reap;
		}
		symtab_assign(ocp->thread_stp, name, value);
	}
	else if (frame && frame->stp && symtab_query(frame->stp, name))
		symtab_assign(frame->stp, name, value);
	else if (ocp->thread_stp && symtab_query(ocp->thread_stp, name))
		symtab_assign(ocp->thread_stp, name, value);
	else
		symtab_assign(id_global_stp(), name, value);
}


id_ty *
opcode_context_id_search(ocp, name)
	const opcode_context_ty *ocp;
	string_ty	*name;
{
	id_ty		*result;

	assert(ocp);
	if (ocp->call_stack_length >= 1)
	{
		opcode_frame_ty	*frame;

		frame = &ocp->call_stack[ocp->call_stack_length - 1];
		if (frame->stp)
		{
			result = symtab_query(frame->stp, name);
			if (result)
				return result;
		}
	}
	if (ocp->thread_stp)
	{
		result = symtab_query(ocp->thread_stp, name);
		if (result)
			return result;
	}
	return symtab_query(id_global_stp(), name);
}


id_ty *
opcode_context_id_search_fuzzy(ocp, name, guess)
	const opcode_context_ty *ocp;
	string_ty	*name;
	string_ty	**guess;
{
	symtab_ty	*argv[3];
	size_t		argc;

	*guess = 0;
	assert(ocp);
	assert(ocp->call_stack_length > 0);
	argc = 0;
	if (ocp->call_stack_length >= 1)
	{
		opcode_frame_ty	*frame;

		frame = &ocp->call_stack[ocp->call_stack_length - 1];
		if (frame->stp)
			argv[argc++] = frame->stp;
	}
	if (ocp->thread_stp)
		argv[argc++] = ocp->thread_stp;
	argv[argc++] = id_global_stp();
	return symtab_query_fuzzyN(argv, argc, name, guess);
}


/*
 * NAME
 *	opcode_context_run
 *
 * SYNOPSIS
 *	string_list_ty *opcode_context_run(opcode_list_ty *);
 *
 * DESCRIPTION
 *	The opcode_context_run function is used to execute an opcode list
 *	with the expectation of extracting a string list result.  Used
 *	to evaluate the need1 and need2 ingredients.
 *
 * RETURNS
 *	string_list_ty *; or NULL pointer on error
 *
 * CAVEAT
 *	Use string_list_delete when you are done with it.
 */

string_list_ty *
opcode_context_run(ocp, olp)
	opcode_context_ty *ocp;
	opcode_list_ty	*olp;
{
	opcode_status_ty status;

	assert(ocp->call_stack_length == 0);
	if (!olp)
		return string_list_new();
	opcode_context_call(ocp, olp);
	status = opcode_context_execute(ocp);
	if (status != opcode_status_success)
		return 0;
	return opcode_context_string_list_pop(ocp);
}


/*
 * NAME
 *	opcode_context_run_bool
 *
 * SYNOPSIS
 *	string_list_ty *opcode_context_run_bool(opcode_list_ty *);
 *
 * DESCRIPTION
 *	The opcode_context_run_bool function is used to execute an opcode list
 *	with the expectation of extracting a boolean result.  Used
 *	to evaluate recipe predicates.
 *
 * RETURNS
 *	int; 0 if false, 1 if true, -1 on error
 */

int
opcode_context_run_bool(ocp, olp)
	opcode_context_ty *ocp;
	opcode_list_ty	*olp;
{
	string_list_ty	*slp;
	int		result;

	if (!olp)
		return 1;
	slp = opcode_context_run(ocp, olp);
	if (!slp)
		return -1;
	result = string_list_bool(slp);
	string_list_delete(slp);
	return result;
}

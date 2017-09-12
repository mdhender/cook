/*
 *	cook - file construction tool
 *	Copyright (C) 1997, 2001, 2003 Peter Miller;
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
 * MANIFEST: interface definition for cook/opcode/context.c
 */

#ifndef COOK_OPCODE_CONTEXT_H
#define COOK_OPCODE_CONTEXT_H

#include <ac/stddef.h>
#include <opcode/status.h>

struct string_ty; /* existence */
struct symtab_ty; /* existence */

typedef struct opcode_frame_ty opcode_frame_ty;
struct opcode_frame_ty
{
	struct opcode_list_ty *olp;
	size_t		pc;
	struct symtab_ty *stp;
};

typedef struct opcode_context_ty opcode_context_ty;
struct opcode_context_ty
{
	size_t		call_stack_length;
	size_t		call_stack_maximum;
	opcode_frame_ty	*call_stack;
	size_t		value_stack_length;
	size_t		value_stack_maximum;
	struct string_list_ty **value_stack;
	long		thread_id;

	struct symtab_ty *thread_stp;
	struct match_stack_ty *msp;

	int		pid;		/* used by opcode_command */
	int		exit_status;	/* used by opcode_command */
	struct meter_ty	*meter_p;	/* used by opcode_command */
	void		*wlp;		/* used by opcode_command */
	int		need_age;	/* used by graph_run */

	/* for suspend/resume */
	void		*flags;
	struct match_ty	*mp;
	struct string_ty *host_binding;

	/* for information about the graph */
	struct graph_ty *gp;
};

opcode_context_ty *opcode_context_new _((struct opcode_list_ty *,
	const struct match_ty *));
void opcode_context_delete _((opcode_context_ty *));
void opcode_context_call _((opcode_context_ty *, struct opcode_list_ty *));
void opcode_context_string_list_push _((opcode_context_ty *));
void opcode_context_string_push _((opcode_context_ty *, struct string_ty *));
void opcode_context_string_push_list _((opcode_context_ty *,
	const struct string_list_ty *));
struct string_list_ty *opcode_context_string_list_pop _((opcode_context_ty *));
struct string_list_ty *opcode_context_string_list_peek _((
	const opcode_context_ty *));
opcode_status_ty opcode_context_execute _((opcode_context_ty *));
opcode_status_ty opcode_context_execute_nowait _((opcode_context_ty *));
opcode_status_ty opcode_context_script _((opcode_context_ty *));
void opcode_context_goto _((opcode_context_ty *, size_t));

int opcode_context_getpid _((opcode_context_ty *));
void opcode_context_waited _((opcode_context_ty *, int));
void opcode_context_suspend _((opcode_context_ty *));
void opcode_context_resume _((opcode_context_ty *));

void opcode_context_host_binding_set _((opcode_context_ty *,
	struct string_ty *));

const struct match_ty *opcode_context_match_top _((const opcode_context_ty *));
const struct match_ty *opcode_context_match_pop _((opcode_context_ty *));
void opcode_context_match_push _((opcode_context_ty *,
	const struct match_ty *));

struct id_ty *opcode_context_id_search _((const opcode_context_ty *,
	struct string_ty *));
struct id_ty *opcode_context_id_search_fuzzy _((const opcode_context_ty *,
	struct string_ty *, struct string_ty **));
void opcode_context_id_assign _((opcode_context_ty *, struct string_ty *,
	struct id_ty *, int));
void opcode_context_id_unassign _((opcode_context_ty *, struct string_ty *));

struct string_list_ty *opcode_context_run _((opcode_context_ty *,
	struct opcode_list_ty *));
int opcode_context_run_bool _((opcode_context_ty *, struct opcode_list_ty *));

#endif /* COOK_OPCODE_CONTEXT_H */

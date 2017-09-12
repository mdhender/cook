/*
 *      cook - file construction tool
 *      Copyright (C) 1997, 2001, 2003, 2006, 2007 Peter Miller;
 *      All rights reserved.
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 3 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program. If not, see
 *      <http://www.gnu.org/licenses/>.
 */

#ifndef COOK_OPCODE_CONTEXT_H
#define COOK_OPCODE_CONTEXT_H

#include <common/ac/stddef.h>
#include <cook/opcode/status.h>

struct string_ty; /* existence */
struct symtab_ty; /* existence */

typedef struct opcode_frame_ty opcode_frame_ty;
struct opcode_frame_ty
{
        struct opcode_list_ty *olp;
        size_t          pc;
        struct symtab_ty *stp;
};

typedef struct opcode_context_ty opcode_context_ty;
struct opcode_context_ty
{
        size_t          call_stack_length;
        size_t          call_stack_maximum;
        opcode_frame_ty *call_stack;
        size_t          value_stack_length;
        size_t          value_stack_maximum;
        struct string_list_ty **value_stack;
        long            thread_id;

        struct symtab_ty *thread_stp;
        struct match_stack_ty *msp;

        int             pid;            /* used by opcode_command */
        int             exit_status;    /* used by opcode_command */
        struct meter_ty *meter_p;       /* used by opcode_command */
        void            *wlp;           /* used by opcode_command */
        int             need_age;       /* used by graph_run */

        /* for suspend/resume */
        void            *flags;
        struct match_ty *mp;
        struct string_ty *host_binding;

        /* for information about the graph */
        struct graph_ty *gp;
};

opcode_context_ty *opcode_context_new(struct opcode_list_ty *,
        const struct match_ty *);
void opcode_context_delete(opcode_context_ty *);
void opcode_context_call(opcode_context_ty *, struct opcode_list_ty *);
void opcode_context_string_list_push(opcode_context_ty *);
void opcode_context_string_push(opcode_context_ty *, struct string_ty *);
void opcode_context_string_push_list(opcode_context_ty *,
        const struct string_list_ty *);
struct string_list_ty *opcode_context_string_list_pop(opcode_context_ty *);
struct string_list_ty *opcode_context_string_list_peek(
        const opcode_context_ty *);
opcode_status_ty opcode_context_execute(opcode_context_ty *);
opcode_status_ty opcode_context_execute_nowait(opcode_context_ty *);
opcode_status_ty opcode_context_script(opcode_context_ty *);
void opcode_context_goto(opcode_context_ty *, size_t);

int opcode_context_getpid(opcode_context_ty *);
void opcode_context_waited(opcode_context_ty *, int);
void opcode_context_suspend(opcode_context_ty *);
void opcode_context_resume(opcode_context_ty *);

void opcode_context_host_binding_set(opcode_context_ty *,
        struct string_ty *);

const struct match_ty *opcode_context_match_top(const opcode_context_ty *);
const struct match_ty *opcode_context_match_pop(opcode_context_ty *);
void opcode_context_match_push(opcode_context_ty *,
        const struct match_ty *);

struct id_ty *opcode_context_id_search(const opcode_context_ty *,
        struct string_ty *);
struct id_ty *opcode_context_id_search_fuzzy(const opcode_context_ty *,
        struct string_ty *, struct string_ty **);
void opcode_context_id_assign(opcode_context_ty *, struct string_ty *,
        struct id_ty *, int);
void opcode_context_id_unassign(opcode_context_ty *, struct string_ty *);

struct string_list_ty *opcode_context_run(opcode_context_ty *,
        struct opcode_list_ty *);
int opcode_context_run_bool(opcode_context_ty *, struct opcode_list_ty *);

#endif /* COOK_OPCODE_CONTEXT_H */

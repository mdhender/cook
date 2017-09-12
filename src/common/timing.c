/*
 *      cook - file construction tool
 *      Copyright (C) 1997, 2001, 2006-2008 Peter Miller
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

#include <common/ac/stdio.h>
#include <common/ac/time.h>
#include <common/ac/unistd.h>

#ifdef HAVE_GETRUSAGE
#include <sys/resource.h>
#endif

#include <common/error.h>       /* if getrusage fails */
#include <common/mem.h>
#include <common/timing.h>
#include <common/star.h>
#include <common/symtab.h>


typedef struct timing_ty timing_ty;
struct timing_ty
{
    char            *name;
    double          inclusive;
    double          exclusive;
    long            ncalls;
};

static size_t   stack_size;
static size_t   stack_size_max;
static timing_ty **stack;
static symtab_ty *stp;


static double
what_time_is_it_now(void)
{
#ifdef HAVE_GETRUSAGE
    struct rusage   ru;
    if (getrusage(RUSAGE_SELF, &ru))
        nerror_raw("getrusage");
    return (ru.ru_utime.tv_sec + ru.ru_utime.tv_usec * 1e-6);
#else
    return (clock() * (1. / CLOCKS_PER_SEC));
#endif
}


void
timing_register(char *name, void **pp)
{
    string_ty       *key;
    timing_ty       *tp;

    if (!stp)
        stp = symtab_alloc(20);
    key = str_from_c(name);
    tp = symtab_query(stp, key);
    if (!tp)
    {
        tp = mem_alloc(sizeof(timing_ty));
        tp->name = name;
        tp->inclusive = 0;
        tp->exclusive = 0;
        tp->ncalls = 0;
        symtab_assign(stp, key, tp);
    }
    str_free(key);
    *pp = tp;
}


void
timing_push(void *p)
{
    timing_ty       *tp;
    double          now;

    tp = p;
    tp->ncalls++;
    now = what_time_is_it_now();
    if (stack_size >= stack_size_max)
    {
        size_t          nbytes;

        stack_size_max = stack_size_max * 2 + 8;
        nbytes = stack_size_max * sizeof(stack[0]);
        stack = mem_change_size(stack, nbytes);
    }
    stack[stack_size++] = tp;
    if (stack_size >= 2)
    {
        if (stack[stack_size - 2] == tp)
            return;
        stack[stack_size - 2]->exclusive += now;
    }
    tp->inclusive -= now;
    tp->exclusive -= now;
}


void
timing_pop(void *p)
{
    timing_ty       *tp;
    double          now;

    now = what_time_is_it_now();
    tp = p;
    stack_size--;
    if (stack_size >= 1)
    {
        if (stack[stack_size - 1] == tp)
            return;
        stack[stack_size - 1]->exclusive -= now;
    }
    tp->inclusive += now;
    tp->exclusive += now;
}


static void
printer(symtab_ty *stp2, string_ty *key, void *data, void *aux)
{
    timing_ty       *tp;
    double          avg;

    (void)stp2;
    (void)key;
    (void)aux;
    tp = data;
    avg = (tp->ncalls > 0 ? 1. / tp->ncalls : 0);
    fprintf
    (
        stderr,
        "%9.3f %9.3f %5ld %9.6f %9.6f %s\n",
        tp->inclusive,
        tp->exclusive,
        tp->ncalls,
        tp->inclusive * avg,
        tp->exclusive * avg,
        tp->name
    );
}


void
timing_print(void)
{
    if (!stp)
        return;
    star_eoln();
    fprintf
    (
        stderr,
        "\n"
        "Inclusive Exclusive Calls I.Average E.Average Filename:Function\n"
    );
    symtab_walk(stp, printer, (void *)0);
    fprintf(stderr, "\n");
}

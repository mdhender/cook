/*
 *      cook - file construction tool
 *      Copyright (C) 1991, 1993, 1994, 1997, 2001, 2005-2007 Peter Miller;
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
 *
 * This code is based on the heart of a file comparison program
 * written by David I. Bell, and used by kind permission.
 * This notice must be retained in all copies and derivatives.
 * Contact the author of aegis for a copy of the file comparison program.
 *
 * This code is based on the algorithm in:
 *      An O(ND) Difference Algorithm and Its Variations
 *      Eugene W. Myers
 *      (TR 85-6, April 10, 1985)
 *      Department of Computer Science
 *      The University of Arizona
 *      Tuscon, Arizona 85721
 *
 * Also see:
 *      A File Comparison Program
 *      Webb Miller and Eugene W. Myers
 *      Software Practice and Experience
 *      (Volume 15, No. 11, November 1985)
 */

#include <common/ac/string.h>

#include <common/fstrcmp.h>
#include <common/mem.h>
#include <common/trace.h>


typedef struct snake_t snake_t;
struct snake_t
{
    long            line1;
    long            line2;
    long            count;
    snake_t         *next;
};

static long     tablesize;      /* needed table size */
static long     tablesize_max;  /* allocated table size */
static long     *V1;            /* the row containing the last d */
static long     *V1_table;
static long     *V2;            /* another row */
static long     *V2_table;
static snake_t  *nextsnake;     /* next allocable snake structure */
static snake_t  *snake_table;   /* allocable snake structures */

typedef struct file file;
struct file
{
    const char      *f_lines;
    long            f_linecount;
};

typedef struct fc_t fc_t;
struct fc_t
{
    file            fileA;
    file            fileB;
    long            maxlines;
    long            minlines;
    long            inserts;
    long            deletes;
    long            matches;
};

static fc_t     fc;


/*
 * Routine to find the middle snake of an optimial D-path spanning
 * lines A to A+N in file A to lines B to B+N in file B.  Returns the
 * length D of the D-path as a return value, and the upper left and
 * lower right relative coordinates of a snake midway through the D-path.
 */

static long
midsnake(int depth, long A, long N, long B, long M, long *ulx, long *uly,
    long *lrx, long *lry)
{
    long            x;
    long            y;
    long            k;
    long            oldx;
    const char      *lp1;
    const char      *lp2;
    long            DELTA;
    long            odd;
    long            MAXD;
    long            changes;
    long            D;

    trace(("midsnake(depth = %d, A = %ld, N = %ld, B = %ld, M = %ld)\n{\n",
        depth, A, N, B, M));
    trace(("searching: %ld,%ld to %ld,%ld\n", A, B, A + N, B + M));
    (void)depth;

    DELTA = N - M;
    odd = DELTA & 1;
    MAXD = (M + N + 1) / 2;
    V1[1] = 0;
    V2[-1] = 0;
    changes = -odd - 2;

    /*
     * This is the main loop for searching for the snake.
     * D is the distance off the diagonals, and is the number
     * of changes needed to get from the upper left to the
     * lower right corner of the region.
     */
    for (D = 0; D <= MAXD; D++)
    {
        changes += 2;

        /*
         * Examine all diagonals within current distance.
         * First search from upper left to lower right,
         * and then search from lower right to upper left.
         */
        for (k = -D; k <= D; k += 2)
        {
            /*
             * Find the end of the furthest forward D-path
             * in diagonal k.
             */
            if (k == -D || (k != D && (V1[k - 1] < V1[k + 1])))
                x = V1[k + 1];
            else
                x = V1[k - 1] + 1;
            y = x - k;
            lp1 = &fc.fileA.f_lines[A + x];
            lp2 = &fc.fileB.f_lines[B + y];
            oldx = x;
            while (x < N && y < M && *lp1 == *lp2)
            {
                x++;
                y++;
                lp1++;
                lp2++;
            }
            V1[k] = x;

            /*
             * See if path overlaps furthest reverse D-path.
             * If so, then we have found the snake.
             */
            if (odd && k >= (DELTA - (D - 1)) && k <= (DELTA + (D - 1)))
            {
                if ((x + V2[k - DELTA]) >= N)
                {
                    *ulx = oldx;
                    *uly = oldx - k;
                    *lrx = x;
                    *lry = y;
                    trace(("midsnake: %ld,%ld to %ld,%ld (odd)\n", *ulx, *uly,
                            *lrx, *lry));
                    trace(("return %ld;\n", changes));
                    trace(("}\n"));
                    return changes;
                }
            }
        }

        for (k = -D; k <= D; k += 2)
        {
            /*
             * Find the end of the furthest reaching reverse
             * path in diagonal k+DELTA.
             */
            if (k == D || (k != -D && (V2[k + 1] < V2[k - 1])))
                x = V2[k - 1];
            else
                x = V2[k + 1] + 1;
            y = x + k;
            lp1 = &fc.fileA.f_lines[A + N - x - 1];
            lp2 = &fc.fileB.f_lines[B + M - y - 1];
            oldx = x;
            while (x < N && y < M && *lp1 == *lp2)
            {
                x++;
                y++;
                lp1--;
                lp2--;
            }
            V2[k] = x;

            /*
             * See if path overlaps furthest forward D-path.
             * If so, then we have found the snake.
             */
            if (!odd && (k <= D - DELTA) && (k >= -D - DELTA))
            {
                if ((x + V1[k + DELTA]) >= N)
                {
                    *ulx = N - x;
                    *uly = M - y;
                    *lrx = N - oldx;
                    *lry = *lrx + *uly - *ulx;
                    trace(("midsnake: %ld,%ld to %ld,%ld (even)\n", *ulx, *uly,
                            *lrx, *lry));
                    trace(("return %ld;\n", changes));
                    trace(("}\n"));
                    return changes;
                }
            }
        }
    }

    /*
     * Middle snake procedure failed!
     */
    assert(0);
    return 0;
}


/*
 * Recursive routine to find a minimal D-path through the edit graph
 * of the two input files.  Arguments are the beginning line numbers in
 * the files, and the number of lines to examine.  This is basically a
 * divide-and-conquer routine which finds the middle snake of an optimal
 * D-path, then calls itself to find the remainder of the path before the
 * snake and after the snake.
 */

static void
findsnake(int depth, long A, long N, long B, long M)
{
    snake_t         *sp;
    long            ulx = 0;
    long            uly = 0;
    long            lrx = 0;
    long            lry = 0;
    long            D;
    long            count;

    trace(("findsnake(depth = %d, A = %ld, N = %ld, B = %ld, M = %ld)\n{\n",
        depth, A, N, B, M));

    /*
     * If more than one change needed, then call ourself for each part.
     */
    D = midsnake(depth, A, N, B, M, &ulx, &uly, &lrx, &lry);

    if (D > 1)
    {
        if (ulx > 0 && uly > 0)
            findsnake(depth + 1, A, ulx, B, uly);
        count = lrx - ulx;
        sp = nextsnake++;
        sp->line1 = A + ulx;
        sp->line2 = B + uly;
        sp->count = count;
        N -= lrx;
        M -= lry;
        if (N > 0 && M > 0)
            findsnake(depth + 1, A + lrx, N, B + lry, M);
        trace(("}\n"));
        return;
    }

    /*
     * Only 0 or 1 change needed, so we can compute the result directly.
     * First compute the snake coming from the upper left corner if any.
     */
    if (N > M)
        count = uly;
    else
        count = ulx;
    sp = nextsnake++;
    sp->line1 = A;
    sp->line2 = B;
    sp->count = count;

    /*
     * Finally compute the snake coming from the lower right corner if any.
     */
    count = lrx - ulx;
    sp = nextsnake++;
    sp->line1 = A + ulx;
    sp->line2 = B + uly;
    sp->count = count;
    trace(("}\n"));
}


double
fstrcmp(const char *s1, const char *s2)
{
    double          result;
    snake_t         *sp;        /* current snake element */
    long            line1;      /* current line in file A */
    long            line2;      /* current line in file B */

    trace(("fstrcmp(s1 = %08lX, s2 = %08lX)\n{\n", s1, s2));
    trace(("s1 = \"%s\";\n", s1));
    trace(("s2 = \"%s\";\n", s2));
    fc.fileA.f_lines = s1;
    fc.fileA.f_linecount = strlen(s1);
    fc.fileB.f_lines = s2;
    fc.fileB.f_linecount = strlen(s2);

    /*
     * Check for trivial case of two empty strings.
     * This also avoids a division by zero at the end of is function.
     */
    if (!fc.fileA.f_linecount && !fc.fileB.f_linecount)
    {
        trace(("return 1;\n"));
        trace(("}\n"));
        return 1;
    }

    if (fc.fileA.f_linecount < fc.fileB.f_linecount)
    {
        fc.minlines = fc.fileA.f_linecount;
        fc.maxlines = fc.fileB.f_linecount;
    }
    else
    {
        fc.minlines = fc.fileB.f_linecount;
        fc.maxlines = fc.fileA.f_linecount;
    }

    tablesize = fc.maxlines * 2 + 1;
    if (tablesize > tablesize_max)
    {
        tablesize_max = tablesize;
        V1_table = mem_change_size(V1_table, sizeof(long) * tablesize_max);
        V2_table = mem_change_size(V2_table, sizeof(long) * tablesize_max);
        snake_table =
            mem_change_size(snake_table, sizeof(snake_t) * tablesize_max);
    }

    V1 = V1_table + fc.maxlines;
    V2 = V2_table + fc.maxlines;
    nextsnake = snake_table;
    if (fc.fileA.f_linecount > 0 && fc.fileB.f_linecount > 0)
    {
        findsnake(0, 0L, fc.fileA.f_linecount, 0L, fc.fileB.f_linecount);
    }

    /*
     * End the list with the lower right endpoint
     */
    sp = nextsnake++;
    sp->line1 = fc.fileA.f_linecount;
    sp->line2 = fc.fileB.f_linecount;
    sp->count = 0;

    /*
     * print out the snake list
     */
#ifdef DEBUG
    for (sp = snake_table; sp < nextsnake; sp++)
    {
        trace(("%d: line1 = %ld; line2 = %ld; count = %ld;\n",
            sp - snake_table, sp->line1, sp->line2, sp->count));
    }
#endif

    /*
     * Scan the snake list and calculate the number of inserted,
     * deleted, and matching lines.
     */
    line1 = 0;
    line2 = 0;
    fc.deletes = 0;
    fc.inserts = 0;
    fc.matches = 0;
    for (sp = snake_table; sp < nextsnake; sp++)
    {
        fc.deletes += (sp->line1 - line1);
        fc.inserts += (sp->line2 - line2);
        fc.matches += sp->count;
        line1 = sp->line1 + sp->count;
        line2 = sp->line2 + sp->count;
    }

    /*
     * the result is 0 if the strings are entirely unalike,
     * and 1 if the strings are identical, and somewhere in between
     * if the are in any way similar.
     */
    result =
        (
            1
        -
            (
                (double)(fc.inserts + fc.deletes)
            /
                (fc.fileA.f_linecount + fc.fileB.f_linecount)
            )
        );
    trace(("return %.6f;\n", result));
    trace(("}\n"));
    return result;
}

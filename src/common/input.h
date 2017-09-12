/*
 *      cook - file construction tool
 *      Copyright (C) 1999, 2001, 2006-2008 Peter Miller
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

#ifndef COMMON_INPUT_H
#define COMMON_INPUT_H

#include <common/main.h>

#define INPUT_EOF (-1)

typedef struct input_ty input_ty;
struct input_ty
{
        struct input_vtbl_ty *vptr;
        /* private: */
        unsigned char *pushback_buf;
        int pushback_len;
        int pushback_max;
};

/*
 * This structure is *not* to be accessed by clients of this interface.
 * It is only present to permit optimizations.
 */
typedef struct input_vtbl_ty input_vtbl_ty;
struct input_vtbl_ty
{
        int size;
        void (*destruct)(input_ty *);
        long (*read)(input_ty *, void *, long);
        int (*get)(input_ty *);
        struct string_ty *(*filename)(input_ty *);
};

long input_read(input_ty *, void *, long);
int input_getc(input_ty *);
void input_ungetc(input_ty *, int);
struct string_ty *input_filename(input_ty *);
void input_delete(input_ty *);

void input_format_error(input_ty *);

struct output_ty; /* existence */
void input_to_output(input_ty *, struct output_ty *);
struct string_ty *input_one_line(input_ty *);

#if 0 /* def __GNUC__ */

/*
 * This optimization has been turned off because
 * gcc-2.96 (rh7.0) 3 tests fail
 * egcs-2.91.66 (rh7.0) no tests fail
 *
 * If you turn this optimization back on, you do so at your own risk.
 * Remember to `make sure' before you install, if you do.
 */

extern __inline long input_read(input_ty *fp, void *data, long len)
        { if (len <= 0) return 0; if (fp->pushback_len > 0) {
        fp->pushback_len--; *(char *)data = fp->pushback_buf[
        fp->pushback_len ]; return 1; } return fp->vptr->read(fp, data, len); }
extern __inline int input_getc(input_ty *fp) { if (fp->pushback_len >
        0) { fp->pushback_len--; return fp->pushback_buf[ fp->pushback_len
        ]; } return fp->vptr->get(fp); }
extern __inline struct string_ty *input_filename(input_ty *fp)
        { return fp->vptr->filename(fp); }
#else /* !__GNUC__ */
#ifdef DEBUG
#define input_filename(fp) ((fp)->vptr->filename(fp))
#endif /* DEBUG */
#endif /* !__GNUC__ */

#endif /* COMMON_INPUT_H */

/*
 *      cook - file construction tool
 *      Copyright (C) 1997, 2006-2008 Peter Miller
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

#ifndef COOK_OPCODE_STATUS_H
#define COOK_OPCODE_STATUS_H

#include <common/main.h>

enum opcode_status_ty
{
        opcode_status_success,
        opcode_status_interrupted,
        opcode_status_error,
        opcode_status_wait
};
typedef enum opcode_status_ty opcode_status_ty;

char *opcode_status_name(opcode_status_ty);

#endif /* COOK_OPCODE_STATUS_H */

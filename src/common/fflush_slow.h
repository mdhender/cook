/*
 *      cook - file construction tool
 *      Copyright (C) 2004, 2006, 2007 Peter Miller;
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

#ifndef COMMON_FFLUSH_SLOW_H
#define COMMON_FFLUSH_SLOW_H

#include <common/ac/stdio.h>
#include <common/main.h>

/**
  * The fflush_slowly function is used to flush a file stream and check
  * success.  If errors are encountered allow for a pause and retry.
  *
  * The functionality and calling interface is identical to the fflush()
  * function:
  *
  *     "The function fflush forces a write of all user-space buffered
  *     data for the given output or update stream via the stream's
  *     underlying write function.  The open status of the stream is
  *     unaffected."
  *
  * If errors are encountered allow for a pause and retry.  This is
  * cultural imperialist tourist mode: When English doesn't work try
  * loud, slow English.
  *
  * @param stream.
  *     The stream to flush If the stream argument is NULL this function
  *     flushes all open output streams.
  * @returns
  *     Upon successful completion 0 is returned.  Otherwise, EOF is
  *     returned and the global variable errno is set to indicate the
  *     error.
  */
int fflush_slowly(FILE *stream);

/**
  * The fflush_slowly_report function is used to print how many fflush
  * retries were required by the program.  Usually only printed in DEBUG
  * mode.
  */
void fflush_slowly_report(void);

#endif /* COMMON_FFLUSH_SLOW_H */

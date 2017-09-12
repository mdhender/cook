/*
 *	cook - file construction tool
 *	Copyright (C) 2004 Peter Miller;
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
 * MANIFEST: interface definition for fflush_slow.c
 */

#ifndef COMMON_FFLUSH_SLOW_H
#define COMMON_FFLUSH_SLOW_H

#include <ac/stdio.h>
#include <main.h>

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
int fflush_slowly _((FILE *stream));

/**
  * The fflush_slowly_report function is used to print how many fflush
  * retries were required by the program.  Usually only printed in DEBUG
  * mode.
  */
void fflush_slowly_report _((void));

#endif /* COMMON_FFLUSH_SLOW_H */

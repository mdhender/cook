/*
 *	cook - file construction tool
 *	Copyright (C) 1999 Peter Miller;
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
 * MANIFEST: functions to manipulate syncs
 */

#include <ac/time.h>

#include <fingerprint/find.h>
#include <fingerprint/sync.h>


/*
 * NAME
 *	fp_sync
 *
 * SYNOPSIS
 *	void fp_sync(void);
 *
 * DESCRIPTION
 *	The fp_sync function is used to write the fingerprint cache
 *	out to disk periodically.  No matter how often it it called,
 *	the fingerprint cache will only be written out once a minute.
 *	This should be called reasonably often.
 */

void
fp_sync()
{
	static time_t	next_time;
	time_t		now;

	time(&now);
	if (!next_time)
		next_time = now + 60;
	else if (now >= next_time)
	{
		next_time = now + 60;
		fp_find_flush();
	}
}

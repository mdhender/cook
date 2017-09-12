#!/bin/sh
#
#	cook - file construction tool
#	Copyright (C) 1994, 1996, 1997, 2002 Peter Miller;
#	All rights reserved.
#
#	This program is free software; you can redistribute it and/or modify
#	it under the terms of the GNU General Public License as published by
#	the Free Software Foundation; either version 2 of the License, or
#	(at your option) any later version.
#
#	This program is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU General Public License for more details.
#
#	You should have received a copy of the GNU General Public License
#	along with this program; if not, write to the Free Software
#	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
#
# MANIFEST: shell script to generate doc/system.list.so
#
sortflag=
if test "$1" = "-r"; then
	sortflag=r
	shift
fi
echo $* |
tr ' ' '\12' |
sort -t. +1n$sortflag -2 +2n$sortflag -3 +3n$sortflag -5 |
while read f
do
	echo ".br"
	echo ".ne 3i"
	echo ".so $f"
done
exit 0

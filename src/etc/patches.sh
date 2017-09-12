#!/bin/sh
#
#	cook - file construction tool
#	Copyright (C) 1993, 1994, 1997 Peter Miller;
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
# MANIFEST: shell script to generate the patch file
#
echo "#!/bin/sh"
echo "#"
echo "# This is a self-applying patch in a shell script."
echo "#"
echo "# Change directory to the top of the cook source tree"
echo "# before running this shell script."
echo "#"
echo "# Don't forget the -p0 option if you apply this patch manually."
echo "#"
echo "# ------------------------------------------------------------"
echo "patch -p0 << 'EndOfThePatch'"
cat $*
echo "fubar"
echo 'EndOfThePatch'
exit 0

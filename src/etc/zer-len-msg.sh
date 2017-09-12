#!/bin/sh
#
#       cook - file construction tool
#       Copyright (C) 1997, 2007 Peter Miller;
#       All rights reserved.
#
#       This program is free software; you can redistribute it and/or modify
#       it under the terms of the GNU General Public License as published by
#       the Free Software Foundation; either version 3 of the License, or
#       (at your option) any later version.
#
#       This program is distributed in the hope that it will be useful,
#       but WITHOUT ANY WARRANTY; without even the implied warranty of
#       MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#       GNU General Public License for more details.
#
#       You should have received a copy of the GNU General Public License
#       along with this program. If not, see
#       <http://www.gnu.org/licenses/>.
#
if test -f $1 -a -s $1
then
        ok=lib/en/LC_MESSAGES
        echo ""
        echo "  There should be no messages common to any commands which"
        echo "  are not in the lib/en/LC_MESSAGES/common.po file."
        echo "  Move the mesages found in $1 into the"
        echo "  lib/en/LC_MESSAGES/common.po file."
        echo ""
        exit 1
fi
exit 0

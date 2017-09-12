#
#       cook - a program construction tool
#       Copyright (C) 1991-1993, 1997, 1999, 2001, 2007 Peter Miller;
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
length <= 72
length > 72 {
        if (substr($0, 1, 1) == "\t")
        {
                printf "\t"
                pos = 8
        }
        else
                pos = 0
        for (j = 1; j <= NF; ++j)
        {
                len = length($j)
                if (j == 1)
                        ;
                else if (pos + 1 + len > 72)
                {
                        printf " \\\n"
                        pos = 0
                        if (pos + len < 70)
                        {
                                printf "\t"
                                pos += 8
                        }
                        if (pos + len < 70)
                        {
                                printf "\t"
                                pos += 8
                        }
                }
                else
                {
                        printf " "
                        ++pos
                }
                printf("%s", $j)
                pos += len
        }
        printf "\n"
}

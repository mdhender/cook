#!/bin/sh
#
#       cook - file construction tool
#       Copyright (C) 1993, 1994, 1997, 2001, 2007, 2008 Peter Miller
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
case $# in
4)
        project=$1
        change=$2
        filename=$3
        path=$4
        ;;
*)
        echo "usage: $0 <project> <change> <filename> <path>" 1>&2
        exit 1
        ;;
esac

tmp=/tmp/$$
tmp2=/tmp/$$.2

#
# the patchlevel patch can be generated accurately
#
if test "$filename" = "common/patchlevel.h"
then
        echo "Index: common/patchlevel.h"
        prev=`aegis -list version -p $project -c $change | \
awk -F'"' '/previous/{print $2}'`
        echo "Prereq: \"$prev\""
        echo "#define PATCHLEVEL \"$prev\"" > $tmp
        diff -c $tmp common/patchlevel.h | sed '1,2d'
        rm -f $tmp
        exit 0
fi

#
# fake patches for the generated files
#
changes=`echo $project | awk -F. '{print "etc/CHANGES."$1"."$2}'`
if test "$filename" = "$changes"
then
        echo "Index: $filename"
        diff -c /dev/null $path | sed '1,2d'
        exit 0
fi

#
# construct a diff listing for the file
#
if aegis -cp $filename -delta 1 -output $tmp -p $project -c $change
then
        in=$tmp
else
        in=/dev/null
fi
if diff -c $in $path > $tmp2 2> /dev/null
then
        echo $filename unchanged 1>&2
else
        echo "Index: $filename"
        sed '1,2d' < $tmp2
fi

#
# clean up and go home
#
rm -f $tmp $tmp2
exit 0

#!/bin/sh
#
#       cook - file construction tool
#       Copyright (C) 1990-1994, 1997, 1998, 2007 Peter Miller;
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
tmp=${COOK_TMP:-/tmp}/$$
PAGER=cat
export PAGER
umask 022
unset COOK
here=`pwd`
if test $? -ne 0 ; then exit 2; fi

bin="$here/${1-.}/bin"

pass()
{
        cd $here
        rm -rf $tmp
        exit 0
}
fail()
{
        echo 'FAILED test of the "%0" pattern' 1>&2
        cd $here
        rm -rf $tmp
        exit 1
}
no_result()
{
        echo 'NO RESULT for test of the "%0" pattern' 1>&2
        cd $here
        rm -rf $tmp
        exit 2
}
trap "no_result" 1 2 3 15

mkdir $tmp $tmp/lib
cd $tmp

#
# Use the default error messages.  There is no other way to get
# predictable test behaviour on the unknown systems we will be tested on.
#
COOK_MESSAGE_LIBRARY=$work/no-such-dir
export COOK_MESSAGE_LIBRARY
unset LANG

cat > Howto.cook << 'foobar'
test:
{
        if [not [match_mask  %0%1  /foo/bar ]] then fail;
        if [not [match_mask /%0%1  /foo/bar ]] then fail;
        if [not [match_mask  %0%1.c foo/bar.c ]] then fail;
}
foobar
if test $? -ne 0 ; then no_result; fi

$bin/cook -nl
if test $? -ne 0 ; then fail; fi

# probably OK
pass
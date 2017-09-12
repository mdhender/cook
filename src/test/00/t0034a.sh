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
if test $? -ne 0 ; then exit 1; fi

bin="$here/${1-.}/bin"

fail()
{
        echo 'FAILED test of the "%0" pattern replacement' 1>&2
        cd $here
        rm -rf $tmp
        exit 1
}
pass()
{
        cd $here
        rm -rf $tmp
        exit 0
}
trap "fail" 1 2 3 15

mkdir $tmp $tmp/lib
cd $tmp

#
# Use the default error messages.  There is no other way to get
# predictable test behaviour on the unknown systems we will be tested on.
#
COOK_MESSAGE_LIBRARY=$work/no-such-dir
export COOK_MESSAGE_LIBRARY
unset LANG

mkdir a a/b a/b/c
if test $? -ne 0 ; then fail; fi

cat > a.c << 'foo'
void
main()
{
        exit(0);
}
foo
if test $? -ne 0 ; then fail; fi

ln a.c a/b.c
if test $? -ne 0 ; then fail; fi

ln a.c a/b/c.c
if test $? -ne 0 ; then fail; fi

ln a.c a/b/c/d.c
if test $? -ne 0 ; then fail; fi

cat > Howto.cook << 'foobar'
cc = [getenv CC]; if [not [cc]] then
{ cc = [find_command gcc]; if [not [cc]] then cc = cc; }

test: a.o a/b.o a/b/c.o a/b/c/d.o;

%0%1.o: %0%1.c
{
        [cc] -c %0%1.c;
        if %0 then
                mv %1.o %0%1.o;
}
foobar
if test $? -ne 0 ; then fail; fi

$bin/cook -nl > /dev/null 2>&1
if test $? -ne 0 ; then fail; fi

# probably OK
pass

#!/bin/sh
#
#       cook - file construction tool
#       Copyright (C) 1993, 1994, 1997, 1998, 2003, 2007, 2008 Peter Miller
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
work=${COOK_TMP:-/tmp}/$$
PAGER=cat
export PAGER
umask 022
unset COOK
here=`pwd`
if test $? -ne 0 ; then exit 1; fi

bin="$here/${1-.}/bin"

fail()
{
        set +x
        echo 'FAILED test of the builtin function "resolve"' 1>&2
        cd $here
        rm -rf $work
        exit 1
}
pass()
{
        set +x
        cd $here
        rm -rf $work
        exit 0
}
trap "fail" 1 2 3 15

mkdir $work $work/lib
if test $? -ne 0 ; then exit 1; fi
cd $work
if test $? -ne 0 ; then fail; fi

#
# Use the default error messages.  There is no other way to get
# predictable test behaviour on the unknown systems we will be tested on.
#
COOK_MESSAGE_LIBRARY=$work/no-such-dir
export COOK_MESSAGE_LIBRARY
unset LANG

#
# test cookbook
#
cat > book << 'fubar'
cc = [getenv CC]; if [not [cc]] then
{ cc = [find_command gcc]; if [not [cc]] then cc = cc; }

exe = ;
if [in [substr 1 6 [os]] CYGWIN NUTCRA] then
        exe = .exe;

search_list  = . bl;
%.o: %.c
{
        [cc] -O -c [resolve %.c];
}

test[exe]: a.o b.o
{
        [cc] -o test[exe] [resolve a.o b.o];
}
fubar
if test $? -ne 0 ; then fail; fi

#
# the first source file
#
cat > a.c << 'fubar'
main()
{
        b();
        exit(0);
}
fubar
if test $? -ne 0 ; then fail; fi

#
# the second source file
#
mkdir bl
if test $? -ne 0 ; then fail; fi
cat > bl/b.c << 'fubar'
b()
{
        printf("Hello, World!\n");
}
fubar
if test $? -ne 0 ; then fail; fi

#
# try it out
#
sleep 2
$bin/cook -book book -nl > LOG 2>&1
if test $? -ne 0 ; then cat LOG; fail; fi

#
# make sure works for object in search list, too
#
mv b.o bl/b.o
if test $? -ne 0 ; then fail; fi
rm test > LOG 2>&1 || rm test.exe > LOG 2>&1
if test $? -ne 0 ; then fail; fi
sleep 2
$bin/cook -book book -nl > LOG 2>&1
if test $? -ne 0 ; then cat LOG; fail; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass

#!/bin/sh
#
#       cook - file construction tool
#       Copyright (C) 1998, 2007, 2008 Peter Miller
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
if test $? -ne 0 ; then exit 2; fi
SHELL=/bin/sh
export SHELL

bin="$here/${1-.}/bin"
PATH=${bin}:${PATH}
export PATH

pass()
{
        set +x
        cd $here
        rm -rf $work
        exit 0
}
fail()
{
        set +x
        echo 'FAILED test of the [interior_files] functionality' 1>&2
        cd $here
        rm -rf $work
        exit 1
}
no_result()
{
        set +x
        echo 'NO RESULT for test of the [interior_files] functionality' 1>&2
        cd $here
        rm -rf $work
        exit 2
}
trap \"no_result\" 1 2 3 15

mkdir $work $work/lib
if test $? -ne 0 ; then no_result; fi
cd $work
if test $? -ne 0 ; then no_result; fi

#
# Use the default error messages.  There is no other way to get
# predictable test behaviour on the unknown systems we will be tested on.
#
COOK_MESSAGE_LIBRARY=$work/no-such-dir
export COOK_MESSAGE_LIBRARY
unset LANG
unset LANGUAGE

#
# test the [interior_files] functionality
#
cat > howto.cook << 'fubar'
all: fred test;

set time-adjust;

src = [glob *.c] [fromto %.y %.c [glob *.y]];

fred: [fromto %.c %.o [src]]
{
        date > [target];
}

%.o: %.c
{
        date > [target];
}

%.c %.h: %.y
{
        date > %.c;
        date > %.h;
}

%.c.d: %.c
        set no-cascade
{
        c_incl -ns -api -ali -nc
                %.c
                --interior-files [interior_files]
                "--prefix='cascade-for %.c ='"
                "--suffix=';'"
                -o [target]
                ;
}

cascade-for lex.c = parse.h;

#include-cooked-nowarn [addsuffix ".d" [src]]

test:
{
        echo interior '=' [interior_files] > [target];
        echo leaf '=' [leaf_files] >> [target];
}

fubar
if test $? -ne 0 ; then no_result; fi

date > parse.y
if test $? -ne 0 ; then no_result; fi
date > main.c
if test $? -ne 0 ; then no_result; fi
cat > lex.c << 'fubar'
#include "parse.h"
yylex(){return 0;}
yyerror(){}
fubar

if test $? -ne 0 ; then no_result; fi
cat > test.ok << 'fubar'
interior = all fred lex.o parse.c main.o parse.h parse.o test
leaf = lex.c parse.y main.c
fubar
if test $? -ne 0 ; then no_result; fi

cook -nl > LOG 2>&1
if test $? -ne 0 ; then cat LOG; fail; fi

diff test.ok test
if test $? -ne 0 ; then fail; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass

#!/bin/sh
#
#	cook - file construction tool
#	Copyright (C) 1997, 1998, 2001, 2003 Peter Miller;
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
# MANIFEST: Test the graph functionality
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
	echo 'FAILED test of the graph functionality' 1>&2
	cd $here
	rm -rf $work
	exit 1
}
no_result()
{
	set +x
	echo 'NO RESULT for test of the graph functionality' 1>&2
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

#
# test the graph functionality
#
cat > howto.cook << 'fubar'
%.c %.h: %.y
{
	/*
	 * Should not be run in this test, but it exersizes a variety of
	 * backtracking in the build phase.
	 */
	yacc -d %.y;
	mv y.tab.c %.c;
	mv y.tab.h %.h;
}

%.o: %.c
{
	echo %.c + [need] > [target];
}

prog: a.o b.o c.o
{
	echo [need] > [target];
}

a.o: a.h b.h c.h(weak);
b.o: b.h c.h;
c.o: c.h;
fubar
if test $? -ne 0 ; then no_result; fi

for f in b.o a.c b.c c.c a.h b.h c.h; do date > $f; sleep 2; done
if test $? -ne 0 ; then no_result; fi

#
# Test creating a pairs file.  This is a simple way to see of the graph
# was built correctly, and the core of the walker is working correctly.
#
$bin/cook -nl -pairs > test.out.1
if test $? -ne 0 ; then fail; fi

# sort because order is allowed to change
sort < test.out.1 > test.out
if test $? -ne 0 ; then no_result; fi

cat > test.ok << 'fubar'
a.o a.c
a.o a.h
a.o b.h
a.o c.h
b.o b.c
b.o b.h
b.o c.h
c.o c.c
c.o c.h
prog a.o
prog b.o
prog c.o
fubar
if test $? -ne 0 ; then no_result; fi

diff test.ok test.out
if test $? -ne 0 ; then fail; fi

#
# Test creating a script file.
#
$bin/cook -nl -script > test.out
if test $? -ne 0 ; then fail; fi

cat > test.ok << 'fubar'
#!/bin/sh

#line 12 howto.cook
if test ! -e a.o \
	-o ! a.o -nt a.c \
	-o ! a.o -nt a.h \
	-o ! a.o -nt b.h \
	-o c.h -nt a.o
then
echo 'echo a.c + a.c a.h b.h c.h > a.o'
( echo a.c + a.c a.h b.h c.h > a.o )
test $? -eq 0 || exit 1
fi

#line 12 howto.cook
if test ! -e b.o \
	-o ! b.o -nt b.c \
	-o ! b.o -nt b.h \
	-o ! b.o -nt c.h
then
echo 'echo b.c + b.c b.h c.h > b.o'
( echo b.c + b.c b.h c.h > b.o )
test $? -eq 0 || exit 1
fi

#line 12 howto.cook
if test ! -e c.o \
	-o ! c.o -nt c.c \
	-o ! c.o -nt c.h
then
echo 'echo c.c + c.c c.h > c.o'
( echo c.c + c.c c.h > c.o )
test $? -eq 0 || exit 1
fi

#line 17 howto.cook
if test ! -e prog \
	-o ! prog -nt a.o \
	-o ! prog -nt b.o \
	-o ! prog -nt c.o
then
echo 'echo a.o b.o c.o > prog'
( echo a.o b.o c.o > prog )
test $? -eq 0 || exit 1
fi
exit 0
fubar
if test $? -ne 0 ; then no_result; fi

diff test.ok test.out
if test $? -ne 0 ; then fail; fi

#
# Now actually run the build and make sure it does the right things.
#
$bin/cook -nl > test.log 2>&1
if test $? -ne 0 ; then cat test.log; fail; fi

test -f a.o || fail
test -f b.o || fail
test -f c.o || fail
test -f prog || fail

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass

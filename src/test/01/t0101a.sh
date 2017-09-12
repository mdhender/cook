#!/bin/sh
#
#	cook - file construction tool
#	Copyright (C) 1997, 1998, 2001 Peter Miller;
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
# MANIFEST: Test the make2cook archive pattern functionality
#
work=${COOK_TMP:-/tmp}/$$
PAGER=cat
export PAGER
umask 022
unset COOK
here=`pwd`
if test $? -ne 0 ; then exit 2; fi

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
	echo 'FAILED test of the make2cook archive pattern functionality' 1>&2
	cd $here
	rm -rf $work
	exit 1
}
no_result()
{
	set +x
       echo 'NO RESULT test of the make2cook archive pattern functionality' 1>&2
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
# test the make2cook archive pattern functionality
#
cat > test.in << 'fubar'
.SUFFIXES:

a: b
	(echo hello) > $@
fubar
if test $? -ne 0 ; then no_result; fi

cat > test.ok << 'fubar'
#line 3 "test.in"
a: b
{
#line 4 "test.in"
	(echo hello) > [target];
}
fubar
if test $? -ne 0 ; then no_result; fi

$bin/make2cook -ln test.in test.out
if test $? -ne 0 ; then fail; fi

diff test.ok test.out
if test $? -ne 0 ; then fail; fi

#
# test the make2cook archive pattern functionality
#
cat > test.in << 'fubar'
.SUFFIXES:
.SUFFIXES: .a .o

libfoo.a: libfoo.a(x.o) libfoo.a(y.o z.o)
	ranlib $@

(%.o): %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $*.o
	$(AR) r $@ $*.o
	$(RM) $*.o
fubar
if test $? -ne 0 ; then no_result; fi

cat > test.ok << 'fubar'
if [not [defined ARFLAGS]] then
	ARFLAGS = rv;
if [not [defined LDLIBS]] then
	LDLIBS = ;
if [not [defined LOADLIBES]] then
	LOADLIBES = ;
if [not [defined TARGET_ARCH]] then
	TARGET_ARCH = ;
if [not [defined LDFLAGS]] then
	LDFLAGS = ;
if [not [defined CC]] then
	CC = cc;
if [not [defined LINK.o]] then
	LINK.o = [CC] [LDFLAGS] [TARGET_ARCH];
if [not [defined RM]] then
	RM = rm -f;
if [not [defined AR]] then
	AR = ar;
if [not [defined CPPFLAGS]] then
	CPPFLAGS = ;
if [not [defined CFLAGS]] then
	CFLAGS = ;

#line 4 "test.in"
libfoo.a: libfoo.a(x.o) libfoo.a(y.o) libfoo.a(z.o)
{
#line 5 "test.in"
	ranlib [target];
}
%0%1(%.o): %0%.c
{
#line 8 "test.in"
	[CC] [CFLAGS] [CPPFLAGS] -c [resolve [head [need]]] -o %0%.o;
	[AR] r %0%1 %0%.o;
	[RM] %0%.o;
}

%0%: %0%.o
{
	[LINK.o] [resolve [need]] [LOADLIBES] [LDLIBS] -o [target];
}
%0%1.a(%.o): %0%.o
	single-thread %0%1.a
{
	[AR] [ARFLAGS] %0%1.a [resolve [head [need]]];
}
%0%1(%): %0%
	single-thread %0%1
{
	[AR] [ARFLAGS] %0%1 [resolve [head [need]]];
}
fubar
if test $? -ne 0 ; then no_result; fi

$bin/make2cook -ln test.in test.out
if test $? -ne 0 ; then fail; fi

diff test.ok test.out
if test $? -ne 0 ; then fail; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass

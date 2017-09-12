#!/bin/sh
#
#	cook - file construction tool
#	Copyright (C) 1994, 1997, 1998 Peter Miller;
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
# MANIFEST: Test the make2cook vpath functionality
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
	echo 'FAILED test of the make2cook vpath functionality' 1>&2
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
trap \"fail\" 1 2 3 15

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
# test the make2cook vpath functionality
#
cat > test.in << 'fubar'
VPATH = $(SRC)/nasal:$(SRC)/mucus
override SRC = $(HOME)/src
vpath %.o obj:old-obj
vpath % other:/junk
.SUFFIXES:
.SUFFIXES: .o .c
test: foo
fubar
if test $? -ne 0 ; then fail; fi

sed 's/X$//' > test.ok << 'fubar'
if [not [defined CPPFLAGS]] thenX
	CPPFLAGS = [getenv CPPFLAGS];X
if [not [defined CFLAGS]] thenX
	CFLAGS = [getenv CFLAGS];X
if [not [defined TARGET_ARCH]] thenX
	TARGET_ARCH = [getenv TARGET_ARCH];X
if [not [defined CC]] thenX
{X
	CC = [getenv CC];X
	if [not [CC]] thenX
		CC = cc;X
}X
if [not [defined COMPILE.c]] thenX
{X
	COMPILE.c = [getenv COMPILE.c];X
	if [not [COMPILE.c]] thenX
		COMPILE.c = [CC] [CFLAGS] [CPPFLAGS] [TARGET_ARCH] -c;X
}X
if [not [defined LDFLAGS]] thenX
	LDFLAGS = [getenv LDFLAGS];X
if [not [defined LINK.c]] thenX
{X
	LINK.c = [getenv LINK.c];X
	if [not [LINK.c]] thenX
		LINK.c = [CC] [CFLAGS] [CPPFLAGS] [LDFLAGS] [TARGET_ARCH];X
}X
if [not [defined LDLIBS]] thenX
	LDLIBS = [getenv LDLIBS];X
if [not [defined LOADLIBES]] thenX
	LOADLIBES = [getenv LOADLIBES];X
if [not [defined LINK.o]] thenX
{X
	LINK.o = [getenv LINK.o];X
	if [not [LINK.o]] thenX
		LINK.o = [CC] [LDFLAGS] [TARGET_ARCH];X
}X
if [not [defined HOME]] thenX
	HOME = [getenv HOME];X
#line 2 "test.in"X
SRC = [HOME]/src;X
X
X
X
X
test: foo;X
search_list = .X
#line 3 "test.in"X
obj old-objX
other /junkX
#line 1 "test.in"X
[SRC]/nasal [SRC]/mucus;X
X
%0%: %0%.oX
{X
	[LINK.o] [resolve [need]] [LOADLIBES] [LDLIBS] -o [target];X
}X
%0%: %0%.cX
{X
	[LINK.c] [resolve [need]] [LOADLIBES] [LDLIBS] -o [target];X
}X
%0%.o: %0%.cX
{X
	[COMPILE.c] [resolve [head [need]]];X
}X
fubar
if test $? -ne 0 ; then fail; fi

$bin/make2cook -e test.in test.out -ln
if test $? -ne 0 ; then fail; fi

diff test.ok test.out
if test $? -ne 0 ; then fail; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass

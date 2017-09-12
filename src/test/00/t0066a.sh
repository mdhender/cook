#!/bin/sh
#
#       cook - file construction tool
#       Copyright (C) 1994, 1997, 1998, 2007, 2008 Peter Miller
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
        echo 'FAILED test of the make2cook functionality' 1>&2
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
# make2cook
#
cat > test.in << 'fubar'
.SUFFIXES:
.SUFFIXES: .c .o
CC = gcc
all: test
override GET = get
fubar
if test $? -ne 0 ; then fail; fi

cat > test.ok << 'fubar'
if [not [defined GFLAGS]] then
        GFLAGS = [getenv GFLAGS];
if [not [defined COFLAGS]] then
        COFLAGS = [getenv COFLAGS];
if [not [defined CO]] then
{
        CO = [getenv CO];
        if [not [CO]] then
                CO = co;
}
if [not [defined CHECKOUT,v]] then
{
        CHECKOUT,v = [getenv CHECKOUT,v];
        if [not [CHECKOUT,v]] then
                CHECKOUT,v = [CO] [COFLAGS];
}
if [not [defined CPPFLAGS]] then
        CPPFLAGS = [getenv CPPFLAGS];
if [not [defined CFLAGS]] then
        CFLAGS = [getenv CFLAGS];
if [not [defined TARGET_ARCH]] then
        TARGET_ARCH = [getenv TARGET_ARCH];

if [not [defined CC]] then
        CC = gcc;

if [not [defined COMPILE.c]] then
{
        COMPILE.c = [getenv COMPILE.c];
        if [not [COMPILE.c]] then
                COMPILE.c = [CC] [CFLAGS] [CPPFLAGS] [TARGET_ARCH] -c;
}
if [not [defined LDFLAGS]] then
        LDFLAGS = [getenv LDFLAGS];
if [not [defined LINK.c]] then
{
        LINK.c = [getenv LINK.c];
        if [not [LINK.c]] then
                LINK.c = [CC] [CFLAGS] [CPPFLAGS] [LDFLAGS] [TARGET_ARCH];
}
if [not [defined LDLIBS]] then
        LDLIBS = [getenv LDLIBS];
if [not [defined LOADLIBES]] then
        LOADLIBES = [getenv LOADLIBES];
if [not [defined LINK.o]] then
{
        LINK.o = [getenv LINK.o];
        if [not [LINK.o]] then
                LINK.o = [CC] [LDFLAGS] [TARGET_ARCH];
}

all: test;
GET = get;

%0%: %0%.o
{
        [LINK.o] [resolve [need]] [LOADLIBES] [LDLIBS] -o [target];
}
%0%: %0%.c
{
        [LINK.c] [resolve [need]] [LOADLIBES] [LDLIBS] -o [target];
}
%0%.o: %0%.c
{
        [COMPILE.c] [resolve [head [need]]];
}
%0%: %0%,v
        set no-implicit-ingredients
{
        [CHECKOUT,v] [resolve [need]] [target]
                set notouch;
}
%0%: RCS/%0%,v
        set no-implicit-ingredients
{
        [CHECKOUT,v] [resolve [need]] [target]
                set notouch;
}
%0%: s.%
        set no-implicit-ingredients
{
        [GET] [GFLAGS] [resolve [need]];
}
%0%: SCCS/s.%
        set no-implicit-ingredients
{
        [GET] [GFLAGS] [resolve [need]];
}
fubar
if test $? -ne 0 ; then fail; fi

$bin/make2cook -e test.in test.out -hc
if test $? -ne 0 ; then fail; fi

diff test.ok test.out
if test $? -ne 0 ; then fail; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass

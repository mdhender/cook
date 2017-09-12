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
# MANIFEST: Test the opcode functionality
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
	echo 'FAILED test of the opcode functionality' 1>&2
	cd $here
	rm -rf $work
	exit 1
}
no_result()
{
	set +x
	echo 'NO RESULT for test of the opcode functionality' 1>&2
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

#
# test the opcode functionality
#
cat > howto.cook << 'fubar'
if 0 then
{
	search_list = .;
	search_tmp = [tail [split ':' [search_path]]];
	search_name = bl;
	loop
	{
		search_dir = [head [search_tmp]];
		if [not [search_dir]] then
			loopstop;
		search_tmp = [tail [search_tmp]];

		if [not [exists [search_name]]] then
			ln -s [search_dir] [search_name]
				set clearstat;
		search_list = [search_list] [search_name];
		search_name = b[search_name];
	}
}

test: { fail not this time; }
fubar
if test $? -ne 0 ; then no_result; fi

cat > test.ok << 'fubar'

   0:	push
   1:	string	"0"
   2:	jmpf	101
   3:	push
   4:	string	"search_list"
   5:	push
   6:	string	"."
   7:	assign		# howto.cook:3
   8:	push
   9:	string	"search_tmp"
  10:	push
  11:	push
  12:	string	"tail"
  13:	push
  14:	string	"split"
  15:	string	":"
  16:	push
  17:	string	"search_path"
  18:	function	# howto.cook:4
  19:	function	# howto.cook:4
  20:	function	# howto.cook:4
  21:	assign		# howto.cook:4
  22:	push
  23:	string	"search_name"
  24:	push
  25:	string	"bl"
  26:	assign		# howto.cook:5
  27:	push
  28:	string	"search_dir"
  29:	push
  30:	push
  31:	string	"head"
  32:	push
  33:	string	"search_tmp"
  34:	function	# howto.cook:8
  35:	function	# howto.cook:8
  36:	assign		# howto.cook:8
  37:	push
  38:	push
  39:	string	"not"
  40:	push
  41:	string	"search_dir"
  42:	function	# howto.cook:9
  43:	function	# howto.cook:9
  44:	jmpf	46
  45:	goto	101
  46:	push
  47:	string	"search_tmp"
  48:	push
  49:	push
  50:	string	"tail"
  51:	push
  52:	string	"search_tmp"
  53:	function	# howto.cook:11
  54:	function	# howto.cook:11
  55:	assign		# howto.cook:11
  56:	push
  57:	push
  58:	string	"not"
  59:	push
  60:	string	"exists"
  61:	push
  62:	string	"search_name"
  63:	function	# howto.cook:13
  64:	function	# howto.cook:13
  65:	function	# howto.cook:13
  66:	jmpf	79
  67:	push
  68:	string	"ln"
  69:	string	"-s"
  70:	push
  71:	string	"search_dir"
  72:	function	# howto.cook:14
  73:	push
  74:	string	"search_name"
  75:	function	# howto.cook:14
  76:	push
  77:	string	"clearstat"
  78:	command	0	# howto.cook:15
  79:	push
  80:	string	"search_list"
  81:	push
  82:	push
  83:	string	"search_list"
  84:	function	# howto.cook:16
  85:	push
  86:	string	"search_name"
  87:	function	# howto.cook:16
  88:	assign		# howto.cook:16
  89:	push
  90:	string	"search_name"
  91:	push
  92:	push
  93:	string	"b"
  94:	push
  95:	push
  96:	string	"search_name"
  97:	function	# howto.cook:17
  98:	catenate
  99:	assign		# howto.cook:17
 100:	goto	27

   0:	push
   1:	string	"not"
   2:	string	"this"
   3:	string	"time"
   4:	fail

   0:	push
   1:	string	"test"
   2:	push
   3:	recipe

#!/bin/sh

#line 21 howto.cook
if test ! -e test
then
echo 'not this time' 1>&2
exit 1
fi
exit 0
fubar
if test $? -ne 0 ; then no_result; fi

$bin/cook -disassemble -script -nl search_path=a:a:b:c > test.out
if test $? -ne 0 ; then fail; fi

diff test.ok test.out
if test $? -ne 0 ; then fail; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass

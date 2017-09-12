#!/bin/sh
#
#       cook - file construction tool
#       Copyright (C) 1994, 1997, 1998, 2001, 2007, 2008 Peter Miller
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
        echo 'FAILED test of the fingerprint functionality' 1>&2
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
# file to be chewed on
#
cat > test.in << 'fubar'
hi
fubar
if test $? -ne 0 ; then fail; fi

cat > test.ok << 'fubar'
JoXEhdflGxZdS89tDd5zpFx0JRJyX0k67EH0Jod7aet4hIjB.hGhon0veN0BZRfi058HZQ06U0JD
fubar
if test $? -ne 0 ; then fail; fi

$bin/cookfp < test.in > test.out
if test $? -ne 0 ; then fail; fi

diff test.ok test.out
if test $? -ne 0 ; then fail; fi

cat > test.ok << 'fubar'
1479881546        3
fubar
if test $? -ne 0 ; then fail; fi

$bin/cookfp -checksum < test.in > test.out
if test $? -ne 0 ; then fail; fi

diff test.ok test.out
if test $? -ne 0 ; then fail; fi

cat > test.ok << 'fubar'
764EFA883DDA1E11DB47671C4A3BBD9E
fubar
if test $? -ne 0 ; then fail; fi

$bin/cookfp -md < test.in > test.out
if test $? -ne 0 ; then fail; fi

diff test.ok test.out
if test $? -ne 0 ; then fail; fi

cat > test.ok << 'fubar'
ADF394F6 55946D41 192DD655 3F028C7D 8D1838B6 136A947C 59DDAE07 D0A051A8
fubar
if test $? -ne 0 ; then fail; fi

$bin/cookfp -snefru < test.in > test.out
if test $? -ne 0 ; then fail; fi

diff test.ok test.out
if test $? -ne 0 ; then fail; fi

#
# file to be chewed on
#
cat > test.in << 'fubar'
64f9001bfeddcdf67c8ff1e211d715148b8c18d3dddf881e6eab505688ced8e14914895969c56fd5
b7994f030fbcee3e3c26494021557e58e14b3fc22e5cf591dceff8ce092a1648be812936ff7b0c6a
d5251037afa448f17dafc95a1ea69c3fa417abe75890e423b0cb70c0c85025f7244d97e31ff3595f
c4ec639659181e17e635b477354e7dbf796f775366eb52cc77c3f99532e3a92780ccaed64e2be89d
375bbd28ad1a3d052b1b42b316c44c714d54bfa8e57ddc7aec6d81445a71046bd822965087fc8f24
cbc60e09b6390366d9f76092d393a70b1d31a08a9cd971c95c1ef44586fab694fdb441658eaafcbe
4bcac6ebfb7a94e55789d04efa13cf35236b8da94133f0006224261cf412f23be75e56a430022116
baf17f1fd09872f9c1a3699cf1e802aa0dd145dc4fdce0938d8412f06cd0f3763de6b73d84ba737f
b43a30f244569f6900e4eacab58de3b0959113c8d62efee990861f83ced698742f793ceee8571c30
483665d1ab07b031914c844f15bf3be82c3f2a9a9eb95fd492e7472d2297cc5bee5f27825377b562
db8ebbcff961deddc59b5c601bd3910d26d206adb28514d85ecf6b527fea78bb504879aced34a884
36e51d3c1753741d8c47caed9d0a40ef3145e221da27eb70df730ba3183c8789739ac0a69a58dfc6
54b134c1ac3e242ecc4939027b2dda998f15bc0129fd38c727d5318f604aaff5f29c6818c38aa2ec
1019d4c3a8fb936e20ed7b390b68611989a0906f1cc7829e9952ef4b850e9e8ccd063a9067002f8e
cfac8cb7eaa24b11988b4e6c46f066dfca7eec08c7bba664831d17bd63f575e69764350e47870d42
026ca4a28167d58761b6adabaa6564d270da237b25e1c74aa1c901a00eb0a5da7670f74151c05aea
933dfa320759ff1a56010ab85fdecb783f32edf8aebedbb939f8326dd20858c59b638be4a572c80a
28e0a19f432099fc3a37c3cdbf95c585b392c12a6aa707d752f66a6112d483b196435b5e3e75802b
3ba52b33a99f51a5bda1e15778c2e70cfcae7ce0d16022672affac4d4a5109470ab2b83a7a04e579
340dfd80b916e922e29d5e9bf5624af44ca9d9af6bbd2cfee3b7f620c2746e075b42b9b6a06919bc
f0f2c40f72217ab514c19df3f3802daee094beb4a2101aff0529575d55cdb27ca33bddb26528b37d
740c05dbe96a62c4407828466d30d706bbf48e2cbce2d3de049e37fa01b5e6342d886d8d7e5a2e7e
d741201306e90f97e45d3ebab8ad338613051b250c03535471c89b75c638fbd0197f11a1ef0f08fb
f844865138409563452f44435d464d5503d8764cb1b8d638a70bba2f94b3d210eb6692a7d409c2d9
68838526a6db8a15751f6c98de769a88c9ee46681a82a3730896aa4942233681f62c55cb9f1c5404
f74fb15cc06e43126ffe5d728aa8678b337cd129
fubar
if test $? -ne 0 ; then fail; fi

cat > test.ok << 'fubar'
qBidy0xYOJYudpCrAbOnH.h2sqZL.0J4bqQ04sjtuWpdKCIB2Zx:P5SiUm1TJySw0HonIo0pSd3I
fubar
if test $? -ne 0 ; then fail; fi

$bin/cookfp < test.in > test.out
if test $? -ne 0 ; then fail; fi

diff test.ok test.out
if test $? -ne 0 ; then fail; fi

cat > test.ok << 'fubar'
4084908062     2066
fubar
if test $? -ne 0 ; then fail; fi

$bin/cookfp -checksum < test.in > test.out
if test $? -ne 0 ; then fail; fi

diff test.ok test.out
if test $? -ne 0 ; then fail; fi

cat > test.ok << 'fubar'
6EF86FBFAC3674A2C9FBA415813F46B3
fubar
if test $? -ne 0 ; then fail; fi

$bin/cookfp -md < test.in > test.out
if test $? -ne 0 ; then fail; fi

diff test.ok test.out
if test $? -ne 0 ; then fail; fi

cat > test.ok << 'fubar'
9A1E964D 21DFDCDA 664499F1 42665FAD 86113627 35726F4A ABD85F4B 33356E1B
fubar
if test $? -ne 0 ; then fail; fi

$bin/cookfp -snefru < test.in > test.out
if test $? -ne 0 ; then fail; fi

diff test.ok test.out
if test $? -ne 0 ; then fail; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass

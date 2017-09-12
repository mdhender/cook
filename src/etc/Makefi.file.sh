#!/bin/sh
#
#	cook - file construction tool
#	Copyright (C) 1993-1997, 1999, 2001-2003 Peter Miller;
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
# MANIFEST: shell script to generate Makefile fragment for each source file
#
case $# in
2 | 3)
	;;
*)
	echo "usage: $0 filename resolved-filename" 1>&2
	exit 1
	;;
esac
file="$1"
rfn="$2"
depfile="$3"

case $file in

*/*.y)
	root=`basename $file .y`
	stem=`echo $file | sed 's/\.y$//'`
	dir=`echo $file | sed 's|/.*||'`

	case $file in
	cooktime/date.y)
		numconf="8 shift/reduce"
		;;
	make2cook/gram.y)
		numconf="2 shift/reduce"
		;;
	make2cook/vargram.y)
		numconf="2 shift/reduce, 10 reduce/reduce"
		;;
	*)
		numconf="no"
		;;
	esac

	yy=`echo $stem | sed -e 's|^[^/]*/||' -e 's|[^a-zA-Z0-9]|_|g'`
	echo ""
	echo "${stem}.gen.c ${stem}.gen.h: $file"
	echo "	@echo Expect $numconf conflicts:"
	echo "	\$(YACC) -d $file"
	echo "	sed -e 's/[yY][yY]/${yy}_/g'" \
			"-e '/<stdio.h>/d'" \
			"-e '/<stdlib.h>/d'" \
			"-e '/<stddef.h>/d'" \
			"y.tab.c > ${stem}.gen.c"
	echo "	sed -e 's/[yY][yY]/${yy}_/g' -e 's/Y_TAB_H/${yy}_TAB_H/g' \
			y.tab.h > ${stem}.gen.h"
	echo "	rm y.tab.c y.tab.h"

	if [ -n "$depfile" -a -r "$depfile" ]; then
		dep=`cat $depfile`
	fi

	echo ""
	echo "${stem}.gen.\$(OBJEXT): ${stem}.gen.c" $dep
	echo "	\$(CC) -I$dir -Icommon \$(CPPFLAGS) \$(CFLAGS) -c ${stem}.gen.c"
	echo "	mv ${root}.gen.\$(OBJEXT) \$@"
	;;

*/*.c)
	root=`basename $file .c`
	stem=`echo $file | sed 's/\.c$//'`
	dir=`echo $file | sed 's|/.*||'`

	if [ -n "$depfile" -a -r "$depfile" ]; then
		dep=`cat $depfile`
	fi

	echo ""
	echo "${stem}.\$(OBJEXT): $file" $dep
	echo "	\$(CC) -I$dir -Icommon \$(CPPFLAGS) \$(CFLAGS) -c $file"
	echo "	mv ${root}.\$(OBJEXT) \$@"
	;;

lib/*/LC_MESSAGES/common.po)
	;;

lib/*/LC_MESSAGES/*.po)
	#
	# Assume that we are using the GNU Gettext program.  All others
	# will fail, because they do not have the -o option.
	#
	stem=`echo $file | sed -e 's|^lib/\(.*\)\.po$|\1|'`
	dir=`dirname $file`
	echo ""
	echo "lib/$stem.mo: $file $dir/common.po etc/msgfmt.sh"
	echo "	sh etc/msgfmt.sh --msgfmt=\$(MSGFMT)" \
		"--msgcat=\$(MSGCAT) --output=lib/$stem.mo" \
		"$file $dir/common.po"
	echo ""
	echo "\$(libdir)/$stem.mo: lib/$stem.mo $dir/.libdir"
	echo "	\$(INSTALL_DATA) lib/$stem.mo \$@"
	;;

lib/*/man?/*.[0-9])
	dir=`echo $file | sed 's|^\(lib/.*/man.\)/.*|\1|'`
	base=`echo $file | sed 's|^lib/.*/man./\(.*\)|\1|'`
	stem=`echo $file | sed 's|^lib/\(.*\)|\1|'`
	part=`echo $file | sed 's|^lib/.*/\(man./.*\)|\1|'`

	if [ -n "$depfile" -a -r "$depfile" ]; then
		dep=`cat $depfile`
	fi

	echo ""
	echo "\$(datadir)/$stem: $file bin/roffpp\$(EXEEXT) $dir/.datadir" $dep
	echo "	bin/roffpp\$(EXEEXT) -I$dir -Ietc $file > tmp"
	echo "	\$(INSTALL_DATA) tmp \$@"
	echo "	@rm -f tmp"

	case $file in
	lib/en/* )
		mansubdir=$dir/.mandir
		dep=`echo $dep | sed 's|/en/|/$(MANLANG)/|g'`
		file=`echo $file | sed 's|/en/|/$(MANLANG)/|'`
		dir=`echo $dir | sed 's|/en/|/$(MANLANG)/|'`
		echo ""
		echo "\$(mandir)/$part: $file bin/roffpp\$(EXEEXT)" \
			$dep $mansubdir
		echo "	bin/roffpp\$(EXEEXT) -I$dir -Ietc $file > tmp"
		echo "	\$(INSTALL_DATA) tmp \$@"
		echo "	@rm -f tmp"
	;;
	esac
	;;

lib/*/man?/*)
	;;

lib/*/*/*.so)
	;;

lib/*/*/*.pic)
	# the "train track" syntax diagrams
	# in the User Guide
	;;

lib/*/building/*)
	;;
lib/*/lsm/*)
	;;
lib/*/readme/*)
	;;
lib/*/release/*)
	;;

lib/*/*/main.*)
	macros=`echo $file | sed 's|^lib/.*/.*/main.\(.*\)$|\1|'`
	stem=`echo $file | sed 's|^lib/\(.*/.*/main\).*$|\1|'`
	dir=`dirname $file`
	dirdir=`dirname $dir`

	dep=""
	if [ -n "$depfile" -a -r "$depfile" ]; then
		dep=`cat $depfile`
	fi

	case $macros in
	ms)
		macros="-ms"
		;;
	mm)
		macros="-mm"
		;;
	roff)
		macros=""
		;;
	*)
		macros="-$macros"
		;;
	esac
	stem2=`dirname $stem`
	stem3=`dirname $stem2`

	echo ""
	echo "lib/$stem2.ps: $file bin/roffpp\$(EXEEXT)" $dep
	echo "	bin/roffpp\$(EXEEXT) -I$dir -Ietc -I$dirdir/man1 \
-I$dirdir/readme $file | \$(GROFF) -s -t -p $macros -mpic -mpspic > \$@"

	echo ""
	echo "\$(datadir)/$stem2.ps: lib/$stem2.ps lib/$stem3/.datadir"
	echo "	\$(INSTALL_DATA) lib/$stem2.ps \$@"

	echo ""
	echo "lib/$stem2.dvi: $file bin/roffpp\$(EXEEXT)" $dep
	echo "	bin/roffpp\$(EXEEXT) -I$dir -Ietc -I$dirdir/man1 \
-I$dirdir/readme $file | \$(GROFF) -Tdvi -s -t -p $macros -mpic > \$@"

	echo ""
	echo "\$(datadir)/$stem2.dvi: lib/$stem2.dvi lib/$stem3/.datadir"
	echo "	\$(INSTALL_DATA) lib/$stem2.dvi \$@"

	echo ""
	echo "lib/$stem2.txt: $file bin/roffpp\$(EXEEXT)" $dep
	echo "	bin/roffpp\$(EXEEXT) -I$dir -Ietc -I$dirdir/man1 \
-I$dirdir/readme $file | \$(GROFF) -Tascii -s -t -p $macros -mpic > \$@"

	echo ""
	echo "\$(datadir)/$stem2.txt: lib/$stem2.txt lib/$stem3/.datadir"
	echo "	\$(INSTALL_DATA) lib/$stem2.txt \$@"
	;;

lib/*)
	dir=`dirname $file`
	root=`basename $file`
	echo ""
	echo "\$(datadir)/$root: $file $dir/.datadir"
	echo "	\$(INSTALL_DATA) $file \$@"
	;;

test/*/*.sh)
	root=`basename $file .sh`
	echo ""
	echo "$root: $file all"
	echo "	CC=\"\$(CC)\" \$(SH) $file"
	;;

script/*.in)
	prog=`echo $file | sed 's|^script/\(.*\)[.]in$|\1|'`
	echo ""
	echo "bin/$prog\$(EXEEXT): $file .bin"
	echo "	CONFIG_FILES=\$@:$file CONFIG_HEADERS= ./config.status"
	echo "	chmod a+rx \$@"
	echo ""
echo "\$(bindir)/\$(PROGRAM_PREFIX)${prog}\$(PROGRAM_SUFFIX)\$(EXEEXT): \
bin/${prog}\$(EXEEXT) .bindir"
	echo "	\$(INSTALL_SCRIPT) bin/${prog}\$(EXEEXT) \$@"
	;;

*)
	;;
esac
exit 0

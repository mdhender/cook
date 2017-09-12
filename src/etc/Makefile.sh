#!/bin/sh
#
#       cook - file construction tool
#       Copyright (C) 1994-1999, 2001, 2003, 2007 Peter Miller;
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
clean_files="core y.tab.c y.tab.h y.output .bin .bindir common/lib.\$(LIBEXT)"

TAB=`echo | tr '\12' '\11'`

#
# list of progams to be linked and installed
#
progs=

#
# Emit the rules to build a directory, and all the ones above it.
#
recursive_mkdir()
{
        src_dir="$1"
        dst_dir="$2"
        flavor="${3-datadir}"
        while :
        do
                dirvar=`echo $src_dir | sed 's|[^a-zA-Z]|_|g'`
                dotdot1=`dirname ${src_dir-.}`
                dotdot2=`dirname ${dst_dir-.}`
                if eval "test \${${dirvar}_${flavor}-no} != yes" ; then
                        echo ""
                        if test "$dotdot1" != "." -a "$dotdot2" != "." ; then
                                echo "$src_dir/.${flavor}: $dotdot1/.${flavor}"
                        else
                                echo "$src_dir/.${flavor}:"
                        fi
                        echo "${TAB}-\$(INSTALL) -m 0755 -d $dst_dir"
                        echo "${TAB}@-test -d $dst_dir && touch \$@"
                        echo "${TAB}@sleep 1"
                        eval "${dirvar}_${flavor}=yes"
                        clean_files="$clean_files $src_dir/.${flavor}"
                fi
                src_dir=$dotdot1
                dst_dir=$dotdot2
                if test "$src_dir" = "." -o "$dst_dir" = "." ; then break; fi
        done
}

remember_prog()
{
        if eval "test \"\${prog_${1}-no}\" != yes"
        then
                progs="$progs $dir"
                eval "prog_${1}=yes"
        fi
}

all=
install_bin=
for file in $*
do
        case $file in

        *.y)
                dir=`echo $file | sed 's|^\([^/]*\)/.*$|\1|'`
                stem=`echo $file | sed 's/\.y$//'`
                eval "${dir}_files=\"\$${dir}_files ${stem}.gen.\\\$(OBJEXT)\""
                clean_files="$clean_files ${stem}.gen.c ${stem}.gen.h \
${stem}.gen.\$(OBJEXT)"
                remember_prog $dir
                ;;

        *.c)
                dir=`echo $file | sed 's|^\([^/]*\)/.*$|\1|'`
                stem=`echo $file | sed 's/\.c$//'`
                eval "${dir}_files=\"\$${dir}_files ${stem}.\\\$(OBJEXT)\""
                clean_files="$clean_files ${stem}.\$(OBJEXT)"
                remember_prog $dir
                ;;

        test/*/*)
                root=`basename $file .sh`
                test_files="$test_files ${root}"
                ;;

        lib/*/LC_MESSAGES/common.po)
                ;;
        lib/*/LC_MESSAGES/fstrcmp.po)
                ;;

        lib/*.po)
                stem=`echo $file | sed 's|^lib/\(.*\)\.po$|\1|'`
                src="lib/$stem.mo"
                po_files="$po_files $src"
                dst="\$(libdir)/$stem.mo"
                po_install_files="$po_install_files $dst"
                recursive_mkdir `dirname $src` `dirname $dst` libdir
                ;;

        lib/*/*/*.so)
                ;;

        lib/*/*/*.pic)
                # the "train track" syntax diagrams
                # in the User Guide
                ;;

        lib/*/man?/*)
                stem=`echo $file | sed 's|^lib/||'`
                man_files="$man_files \$(datadir)/$stem"

                case $file in
                lib/en/*)
                        stem2=`echo $file | sed 's|^lib/en/||'`
                        man_files="$man_files \$(mandir)/$stem2"

                        src="lib/$stem"
                        dst="\$(mandir)/$stem2"
                        recursive_mkdir `dirname $src` `dirname $dst` mandir
                        ;;
                esac

                src="lib/$stem"
                dst="\$(datadir)/$stem"
                recursive_mkdir `dirname $src` `dirname $dst` datadir
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
                stem=`echo $file | sed 's|^lib/\(.*\)/main.*$|\1|'`
                groff_all="$groff_all lib/$stem.ps lib/$stem.dvi lib/$stem.txt"
                clean_files="$clean_files \
lib/$stem.ps lib/$stem.dvi lib/$stem.txt"
                groff_install="$groff_install \$(datadir)/$stem.ps \
\$(datadir)/$stem.dvi \$(datadir)/$stem.txt"
                ;;

        lib/*)
                rest=`echo $file | sed 's|^lib/||'`
                dst="\$(datadir)/$rest"
                lib_files="$lib_files $dst"
                recursive_mkdir `dirname lib/$rest` `dirname $dst` datadir
                ;;

        script/*.in)
                prog=`echo $file | sed 's|^script/\(.*\)[.]in$|\1|'`
                all="${all} bin/${prog}\$(EXEEXT)"
                install_bin="${install_bin} \
\$(bindir)/\$(PROGRAM_PREFIX)${prog}\$(PROGRAM_SUFFIX)\$(EXEEXT)"
                ;;

        *)
                ;;
        esac
done

for prog in $progs
do
        echo ""
        eval "echo \"${prog}_obj =\" \${${prog}_files}"

        if test "$prog" = common; then continue; fi

        echo ""
        echo "bin/${prog}\$(EXEEXT): \$(${prog}_obj) common/lib.\$(LIBEXT) .bin"
        echo "${TAB}\$(CC) \$(LDFLAGS) -o \$@ \$(${prog}_obj) \
common/lib.\$(LIBEXT) \$(LIBS)"

        all="${all} bin/${prog}\$(EXEEXT)"

        if test "$prog" = fstrcmp; then continue; fi
        if test "$prog" = file_check; then continue; fi

        echo ""
        echo "\$(bindir)/\$(PROGRAM_PREFIX)${prog}\$(PROGRAM_SUFFIX)\$(EXEEXT):\
 bin/${prog}\$(EXEEXT) .bindir"
        echo "${TAB}\$(INSTALL_PROGRAM) bin/${prog}\$(EXEEXT) \$@"

        install_bin="${install_bin} \
\$(bindir)/\$(PROGRAM_PREFIX)${prog}\$(PROGRAM_SUFFIX)\$(EXEEXT)"
done

echo ""
echo "#"
echo "# The real default target"
echo "#"
echo "all:" ${all} po groff_all

echo ""
echo "common/lib.\$(LIBEXT): \$(common_obj)"
echo "${TAB}rm -f \$@"
echo "${TAB}\$(AR) qc \$@ \$(common_obj)"
echo "${TAB}\$(RANLIB) \$@"

echo ""
echo ".bin:"
echo "${TAB}-mkdir bin"
echo "${TAB}-chmod 0755 bin"
echo "${TAB}@-test -d bin && touch .bin"
echo "${TAB}@sleep 1"

echo ""
echo ".bindir:"
echo "${TAB}-\$(INSTALL) -m 0755 -d \$(bindir)"
echo "${TAB}@-test -d \$(bindir) && touch .bindir"
echo "${TAB}@sleep 1"

echo ""
echo "sure:" $test_files
echo "${TAB}@echo Passed All Tests"

echo ""
echo "po_files_yes =" $po_files
echo ""
echo "po_files_no ="
echo ""
echo "po: \$(po_files_@po_files@)"

echo ""
echo "groff_all_yes =" $groff_all
echo ""
echo "groff_all_no ="
echo ""
echo "groff_all: \$(groff_all_@have_groff@)"

echo ""
echo "clean-obj:"
echo $clean_files | tr ' ' '\12' | gawk '{
        if (pos > 0 && pos + length($1) > 71) { printf("\n"); pos = 0; }
        if (pos == 0) { printf "\trm -f"; pos = 13; }
        printf " %s", $1
        pos += 1 + length($1);
}
END { if (pos) printf "\n"; }'

echo ""
echo "clean: clean-obj"
echo $all | tr ' ' '\12' | gawk '{
        if (pos > 0 && pos + length($1) > 71) { printf("\n"); pos = 0; }
        if (pos == 0) { printf "\trm -f"; pos = 13; }
        printf " %s", $1
        pos += 1 + length($1);
}
END { if (pos) printf "\n"; }'
echo "${TAB}rm -f \$(po_files_yes)"
echo "${TAB}rm -f \$(groff_all_yes)"

echo ""
echo "distclean: clean"
echo "${TAB}rm -f Makefile common/config.h etc/libdir.so etc/libdir-h"
echo "${TAB}rm -f config.status config.cache config.log"

echo ""
echo "install-bin:" ${install_bin}

echo ""
echo "install-man:" $man_files

echo ""
echo "po_install_files_yes =" $po_install_files
echo ""
echo "po_install_files_no ="

echo ""
echo "install-po: \$(po_install_files_@po_files@)"

echo ""
echo "groff_install_yes =" $groff_install
echo ""
echo "groff_install_no ="

echo ""
echo "install-groff: \$(groff_install_@have_groff@)"

echo ""
echo "install-lib:" $lib_files install-po install-groff

echo ""
echo "install: install-bin install-man install-lib"

exit 0

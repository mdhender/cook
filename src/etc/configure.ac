dnl
dnl     cook - file construction tool
dnl     Copyright (C) 1994-2004, 2006-2008 Peter Miller
dnl
dnl     This program is free software; you can redistribute it and/or modify
dnl     it under the terms of the GNU General Public License as published by
dnl     the Free Software Foundation; either version 3 of the License, or
dnl     (at your option) any later version.
dnl
dnl     This program is distributed in the hope that it will be useful,
dnl     but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl     GNU General Public License for more details.
dnl
dnl     You should have received a copy of the GNU General Public License
dnl     along with this program. If not, see
dnl     <http://www.gnu.org/licenses/>.
dnl
AC_INIT(install-sh)
AC_CONFIG_HEADER(common/config.h)
AC_PROG_CC
AC_PROG_CPP

dnl
dnl     Avoid the /usr/ucb/cc compiler, it screws up readdir,
dnl     some systems seem to have the ucb one early in the path.
dnl
dnl     ranlib is unneccessary, but setting it to : will
dnl     not work, so set it to echo.
dnl
dnl     This test must be before the stdarg test
dnl     and before AC_CONST
dnl
if test -z "$GCC" ; then
AC_MSG_CHECKING([for Pyramid])
AC_PROGRAM_EGREP(yes,[#ifdef __pyrsoft
  yes
#endif],CC="/usr/ccs/bin/cc"
RANLIB="echo no need to ranlib"
AC_MSG_RESULT(yes), AC_MSG_RESULT(no))
fi

dnl AC_AIX - omitted because it hangs?!?
AC_MINIX
AC_ISC_POSIX
AC_EXEEXT
AC_OBJEXT
AC_PROG_INSTALL
AC_PROG_YACC
AC_PROG_RANLIB
AC_HAVE_LIBRARY(w32)    dnl DG/UX wide character support
dnl Solaris 2.5.1 and below wide-character support
AC_CHECK_FUNC(wschr, ,
        unset ac_cv_func_wschr
        AC_CHECK_LIB(w, wschr))
AC_CHECK_FUNC(gettext, ,
        unset ac_cv_func_gettext
        AC_CHECK_LIB(intl, gettext))
AC_HAVE_LIBRARY(rx)
AC_CHECK_FUNC(setsockopt, ,
        unset ac_cv_func_setsockopt
        AC_CHECK_LIB(socket, setsockopt))
AC_CHECK_PROGS(MSGFMT, gmsgfmt msgfmt, msgfmt)
AC_CHECK_PROGS(MSGCAT, gmsgcat msgcat)
AC_CHECK_PROGS(GROFF, groff roff)
AC_CHECK_PROGS(RSH, remsh rsh, rsh)
AC_PATH_PROGS(PERL, perl, /usr/bin/perl)

dnl
dnl     Test to see if stdarg.h is available *and* works.
dnl
AC_MSG_CHECKING([for working stdarg.h])
AC_TRY_RUN([
#include <stdarg.h>
char *foo = "test";
#if defined(__STDC__) && __STDC__
int test(char*,...);
int test(char *fmt, ...)
#else
int test(fmt)char *fmt;
#endif
{va_list ap;char*a;int x;
va_start(ap,fmt);a=va_arg(ap,char*);x=(a!=foo);va_end(ap);return x;}
int main(argc,argv)int argc;char**argv;{
exit(test("",foo));}]
, AC_DEFINE(HAVE_STDARG_H,1,
[Define this symbol of your system has <stdarg.h> AND it works.])
AC_MSG_RESULT(yes),
AC_MSG_RESULT(no),
AC_MSG_RESULT(cross))dnl

AC_HAVE_HEADERS(ar.h fcntl.h iso646.h libgettext.h libintl.h limits.h locale.h \
        memory.h mntent.h regex.h rxposix.h stddef.h stdlib.h string.h \
        sys/ioctl.h sys/utsname.h termios.h utime.h unistd.h wchar.h wctype.h \
        widec.h)
AC_HEADER_DIRENT
AC_RETSIGTYPE
AC_SIZE_T
AC_TIME_WITH_SYS_TIME
AC_CONST
AC_LONG_FILE_NAMES
AC_HAVE_FUNCS( \
        gethostname \
        getpgrp \
        getrusage \
        gettext \
        gettimeofday \
        iswctype \
        mblen \
        pathconf \
        regcomp \
        setlocale \
        snprintf \
        strcasecmp \
        strendcpy \
        strerror \
        strftime \
        strlcat \
        strlcpy \
        strncasecmp \
        strsignal \
        strtol \
        tcgetpgrp \
        uname \
        vsnprintf \
        wait3 \
        wait4 \
        wcslen \
        )

dnl
dnl     Check to see if wint_t is defined.
dnl     The ANSI C standard states that this symbol shall be defined
dnl     by <wchar.h> and <wctype.h>.  The GNU people also define it in
dnl     <stddef.h>, but this is incorrect.
dnl
AC_MSG_CHECKING([for wint_t])
AC_TRY_COMPILE([
#ifdef HAVE_WCHAR_H
#include <wchar.h>
#endif
],
[ wint_t x; ],
AC_DEFINE(HAVE_WINT_T,1,
[Define this symbol of your system has the wint_t type defined.
It is usually in <stddef.h> or <wctype.h>])
AC_MSG_RESULT(yes),
AC_MSG_RESULT(no))dnl

dnl
dnl     Test to see if iswprint is available *and* works.
dnl
AC_MSG_CHECKING([for working iswprint])
AC_TRY_RUN([
#include <ctype.h>
#ifdef HAVE_WIDEC_H
#include <widec.h>
#endif
#ifdef HAVE_WCTYPE_H
#include <wctype.h>
#endif
int main(argc,argv)int argc;char**argv;{
exit(iswprint('a') == 0);}]
, AC_DEFINE(HAVE_ISWPRINT,1,
[Define this symbol of your system has iswprint() AND it works.
(GNU libc v2 had a bug.)])
AC_MSG_RESULT(yes),
AC_MSG_RESULT(no),
AC_MSG_RESULT(cross))dnl

dnl
dnl     This came from the GNU Make configure.in file.
dnl     It is only used by the make2cook command.
dnl
AC_MSG_CHECKING([for location of SCCS get command])
if test -f /usr/sccs/get; then
  SCCS_GET=/usr/sccs/get
  AC_DEFINE(SCCS_GET, "/usr/sccs/get",
  [Define to the name of the SCCS `get' command. (Used only by make2cook)])
else
  SCCS_GET=get
  AC_DEFINE(SCCS_GET, "get",
  [Define to the name of the SCCS `get' command.  (Used only by make2cook)])
fi
AC_MSG_RESULT($SCCS_GET)
ac_clean_files="$ac_clean_files s.conftest conftoast" # Remove these later.

dnl
dnl     Test to see if getpgrp() or getpgrp(0) should be used to
dnl     discover the process group of the current process.
dnl
AC_MSG_CHECKING([for appropriate getpgrp argument])
AC_TRY_RUN([
int main(argc, argv) int argc; char **argv; {
#ifdef HAVE_GETPGRP
if (getpgrp(32767) == getpgrp(0)) exit(2);
#endif
exit(0); }]
, AC_DEFINE(CONF_getpgrp_arg, 0,
[Set this to a suitable argument for the getpgrp function to discover
the process group of the current process.])
AC_MSG_RESULT(zero),
AC_DEFINE(CONF_getpgrp_arg, [],
[Set this to a suitable argument for the getpgrp function to discover
the process group of the current process.])
AC_MSG_RESULT(empty),
AC_MSG_RESULT(cross))dnl

AC_CHECK_SIZEOF(long)
AC_CHECK_SIZEOF(int)
AC_CHECK_SIZEOF(short)

dnl
dnl     Test to see if the tm_zone field exists in
dnl     the passwd tm defined in the <time.h> include file.
dnl
AC_MSG_CHECKING([for tm_zone in struct tm])
AC_TRY_COMPILE([
#ifdef TIME_WITH_SYS_TIME
#include <sys/time.h>
#include <time.h>
#else
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#else
#include <time.h>
#endif
#endif], [struct tm *tm;
printf("%s\n", tm->tm_zone);],
AC_DEFINE(HAVE_tm_zone,1,
[Define this symbol if your system has
the tm_zone field in the tm struct defined in <time.h>])
AC_MSG_RESULT(yes),AC_MSG_RESULT(no))dnl

dnl
dnl     Test to find a Bourne shell which understands functions
dnl
AC_MSG_CHECKING([for a Bourne shell which understands functions])
if test "z$SH" = "z"; then
    if test -f /bin/sh5; then
        SH=/bin/sh5
    else
        SH=/bin/sh
    fi
fi
AC_SUBST(SH)
AC_DEFINE_UNQUOTED(CONF_SHELL, ["$SH"],
[Set this to be the absolute path of a Bourne shell
which understands functions.])
AC_MSG_RESULT($SH)

AC_DEFINE_UNQUOTED(CONF_REMOTE_SHELL, ["$RSH"],
[Set this to be the name of a BSD remote shell command (or equivalent).])

dnl
dnl     If the gettext function exists, assume the msgfmt program does
dnl     also, and arrange for the Makefile to install the .mo files
dnl     from the .po files.
dnl
if test "$ac_cv_func_gettext" = "yes" -o "$ac_cv_lib_intl_gettext" = "yes"
then
  po_files=yes
else
  po_files=no
  AC_MSG_RESULT([])
  AC_MSG_RESULT([       Cook will emit much more useful error messages])
  AC_MSG_RESULT([       if you install GNU \`\`gettext'' and then])
  AC_MSG_RESULT([       re-run this ./configure script.  Remember to])
  AC_MSG_RESULT([       use \`\`rm config.cache'' before you do.])
  AC_MSG_RESULT([])
fi
AC_SUBST(po_files)

dnl
dnl     Arrange for the existence of groff to affect the Makefile
dnl
if test -n "$GROFF"
then
        have_groff=yes
else
        have_groff=no
        AC_MSG_RESULT([])
        AC_MSG_RESULT([ The Cook documentation set and manual pages will])
        AC_MSG_RESULT([ be formatted and installed if you first install])
        AC_MSG_RESULT([ GNU Groff and then re-run this ./configure script.])
        AC_MSG_RESULT([ Remember to use \`\`rm config.cache'' before])
        AC_MSG_RESULT([ you do.])
        AC_MSG_RESULT([])
fi
AC_SUBST(have_groff)

# program prefix is the bit to add to the start of the name
if test ${PROGRAM_PREFIX-NONE} != NONE -a ${program_prefix-NONE} != NONE; then
PROGRAM_PREFIX=${program_prefix}
fi
AC_SUBST(PROGRAM_PREFIX)

# program suffix is the bit to add to the end of the name (before .exe)
if test ${PROGRAM_SUFFIX-NONE} != NONE -a ${program_suffix-NONE} != NONE; then
PROGRAM_SUFFIX=${program_suffix}
fi
AC_SUBST(PROGRAM_SUFFIX)

dnl
dnl Evaluate some of the variables, to remove ${prefix} references.
dnl This way, they can be used in C programs and Roff input.
dnl Make sure that cook is mentioned in the libdir and datadir paths;
dnl add it if it is not already there.
dnl
test "x$prefix" = xNONE && prefix="${ac_default_prefix-/usr/local}"
test "x$exec_prefix" = xNONE && exec_prefix="$prefix"
eval "exec_prefix=$exec_prefix"
eval "bindir=$bindir"
eval "datarootdir=$datarootdir"
eval "mandir=$mandir"
eval "libdir=$libdir"
eval "datadir=$datadir"
case "$libdir" in
*/cook/* | */cook )
        ;;
*)
        libdir="${libdir}/cook"
        AC_MSG_RESULT([Rewriting --libdir=$libdir])
        ;;
esac
case "$datadir" in
*/cook/* | */cook )
        ;;
*)
        datadir="${datadir}/cook"
        AC_MSG_RESULT([Rewriting --datadir=$datadir])
        ;;
esac

dnl
dnl support for NLSDIR option
dnl
AC_ARG_WITH(nlsdir,
    [  --with-nlsdir=PATH   specify where the locale files should be installed])

if test "x$NLSDIR" = "x"; then
        if test "x$with_nlsdir" != "x"; then
                NLSDIR=$with_nlsdir
        else
                NLSDIR=${libdir}
        fi
fi
AC_SUBST(NLSDIR)

AH_TOP(
[/*
 *      cook - file construction tool
 *      Copyright (C) 1995-2004, 2006-2008 Peter Miller
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 3 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program. If not, see
 *      <http://www.gnu.org/licenses/>.
 *
 * @configure_input@
 */

#ifndef COMMON_CONFIG_H
#define COMMON_CONFIG_H
])

AH_BOTTOM(
[/*
 * There is more to do, but we need to insulate it from config.status,
 * because it screws up the #undef lines.  They are all implications of
 * the above information, so there is no need for you to edit the file,
 * if you are configuring Aegis manually.
 */
#include <common/config.messy.h>

#endif /* COMMON_CONFIG_H */])

AC_OUTPUT(Makefile etc/libdir.so etc/libdir-h)

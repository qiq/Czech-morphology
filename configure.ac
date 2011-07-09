#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT(czmorph, 0.0.1-dev, qiq@ucw.cz)
AC_CONFIG_AUX_DIR(config)
AC_CONFIG_SRCDIR([config.h.in])
AM_CONFIG_HEADER([config.h])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE(subdir-objects -Wall -Werror)
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES])
dnl AH_TOP([#define __STDC_FORMAT_MACROS 1])

AC_ARG_ENABLE(debug, [  --enable-debug          turn on debugging support], use_debug=yes, use_debug=no)

# Checks for programs.
#AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_LIBTOOL
AC_PROG_LEX

# Checks for header files.
AC_HEADER_STDC

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

# Misc
dnl Check for debug options
if test "x$use_debug" = "xyes"; then
	CFLAGS="$CFLAGS -ggdb -O0 -Wall"
else
	CFLAGS="$CFLAGS -DNDEBUG"
fi

dnl set PREFIX in config.h
if test "x${prefix}" = "xNONE"; then
	AC_DEFINE_UNQUOTED(PREFIX, "${ac_default_prefix}", [Package prefix])
else
	AC_DEFINE_UNQUOTED(PREFIX, "${prefix}", [Package prefix] )
fi

AC_CONFIG_FILES([Makefile
                src/Makefile
		data/Makefile
		test/Makefile
		czmorph.pc
])
AC_OUTPUT

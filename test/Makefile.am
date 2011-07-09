AM_CFLAGS = -Wall -DHF_UPDATE -O0 -ggdb -I $(top_srcdir)/src
AM_LDFLAGS = -ggdb

TESTS = czm.sh
EXTRA_DIST = czm.sh czm.in czm.out
CLEANFILES = czm.out.tmp

bin_PROGRAMS = czm
czm_SOURCES = czm.c
czm_LDADD = $(top_builddir)/src/libczmorphology.la

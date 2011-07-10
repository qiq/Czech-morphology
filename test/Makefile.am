AM_CFLAGS = -Wall -DHF_UPDATE -O0 -ggdb -I $(top_srcdir)/src
AM_LDFLAGS = -ggdb

TESTS = czm_test.sh
EXTRA_DIST = czm_test.sh czm_test.in czm_test.out
CLEANFILES = czm_test.out.tmp

bin_PROGRAMS = czm
czm_SOURCES = czm.c
czm_LDADD = $(top_builddir)/src/libczmorphology.la

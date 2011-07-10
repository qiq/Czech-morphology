AM_CFLAGS = -Wall -DHF_UPDATE -I $(top_srcdir)/src

TESTS = czm_test.sh
EXTRA_DIST = czm_test.sh czm_test.in czm_test.out
CLEANFILES = czm_test.out.tmp

noinst_PROGRAMS = czm
czm_SOURCES = czm.c
czm_LDADD = $(top_builddir)/src/libczmorphology.la

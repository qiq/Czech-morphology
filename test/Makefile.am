AM_CFLAGS = -Wall -DHF_UPDATE -O0 -ggdb -I $(top_srcdir)/src
AM_LDFLAGS = -ggdb

TESTS = test.sh
EXTRA_DIST = test.sh test.in test.out
CLEANFILES = test.out.tmp

bin_PROGRAMS = test
test_SOURCES = test.c
test_LDADD = $(top_builddir)/src/libczmorph.la

AM_CFLAGS = -Wall -DHF_UPDATE -DLONG_IS_64 -D__alpha -I $(top_srcdir)/src/czmorphology $(PTHREAD_CFLAGS)
AM_LDFLAGFS = $(PTHREAD_LIBS)

lib_LTLIBRARIES = libczmorphology.la

nobase_include_HEADERS = czmorphology/hb_arg.h czmorphology/hb_base.h czmorphology/hb_cpd.h czmorphology/hb_hash.h czmorphology/hb_il2ca.h czmorphology/hb_proto.h czmorphology/hf_codex.h czmorphology/hf_isif.h czmorphology/hf_modd.h czmorphology/hh_cp.h czmorphology/proto.h czmorphology/interface.h

libczmorphology_la_SOURCES = \
	czmorphology/hb_arg.h czmorphology/hb_base.h czmorphology/hb_cpd.h czmorphology/hb_hash.h czmorphology/hb_il2ca.h czmorphology/hb_proto.h czmorphology/hf_codex.h czmorphology/hf_isif.h czmorphology/hf_modd.h czmorphology/hh_cp.h czmorphology/proto.h czmorphology/interface.h \
	hb_arg.c hb_base.c hb_cpd.c hb_hash.c hb_il2ca.c hf_codex.c hf_modd.c hh_cp.c interface.c \
	c4023.c

BUILT_SOURCES = c4023.c
c4023.c: c4023.fl
	$(LEX) -o c4023.c c4023.fl
EXTRA_DIST = c4023.fl

# original (Hajic) program, not library: for testing purposes only
#bin_PROGRAMS = test
#test_CFLAGS = $(AM_CFLAGS)
#test_SOURCES = $(libczmorphology_la_SOURCES)

/*************************************************************/
/*                                                           */
/* hf header file                                            */
/*                                                           */
/*                                                           */
/*                                                           */
/*                                                           */
/*                                                           */
/* 24.09.99 hajic Merged from O.C. + changes:                */
/*   #define HF_MAXNO_LEMMAS 24->28                          */
/*   #define HF_MAX_RESULT_REC 20->40                        */
/*                                                           */
/* 14.05.96 hajic nej- handling change: now flagged by ^tag  */
/* 05.06.95 hajic actual use of proto.h, cleanup             */
/* (C) Jan Hajic 26.05.91                                    */
/*                                                           */
/*     Changes                                               */
/*     (denoted XX_HR, original HAJ)                         */
/* (C) Jan Hric 21.3.1992                                    */
/*   4/ 28.06 ne-  do interface                              */
/*   5/ 06.09 zmena hr_is_ne je v *.c, v *.h je extern       */
/*                                                           */
/*************************************************************/
#ifndef HF_MODFH

#include <stdio.h>
#include <string.h>
#include "hb_base.h"
#include "hb_proto.h"
/* alloc not defined elsewhere... */
#define alloc malloc

/* JH 28.04.96 following defines moved from hf_modf.c */
#define HF_PATSEP_CHAR '|'
#define HF_PAT2SEP_CHAR ':'
#define HF_TAG_SEP '+'
#define HF_TAG_ALT '/'
#define HF_NEG_VAR '@'
#define HF_NEJ_VAR '#'
#define HF_NEG_AFF 'A'
#define HF_NEG_NEG 'N'
#define HF_NEJ_CMP '2'
#define HF_NEJ_SUP '3'
#define HF_EXPL_SYNT ' '
#define HF_EXPL_SEM '-'
#define HF_EXPL_ST '?'
#define HF_MAXLEMMA 200
#define HF_MAXTAG 1200
#define HF_MAXBUILD 1500
#define HF_MAXRESULT 29200
#define HF_BASE_MAX 80
#define HF_IDMAX 30
#define HF_EXPL_SPACE '_'
#define HF_ID_SEP '-'
#define HF_DERIV_INFO '*'
#define HF_SYN '*'
#define HF_MULTILEMMA_SEP '`'
#define HF_LEFT_COMMENT '('
#define HF_RIGHT_COMMENT ')'
/* JH 14.05.96: added following def */
#define HF_GRAD_TAGC '^'
/* JH 23.11.1994 30 -> 60 */
/* JH 28.04.1996 60 -> 120 */
#define HF_MAXNO_RESULTS 120
/* JH 01.08.1997: */
/* JH 23.09.2001: */
#define HF_MAXNO_LEMMAS 77
/* JH 23.09.2001: */
#define HF_MAXNO_TAGS 230
#define HF_MAX_ROOT_LEN 32
#define HF_MAX_ROOT_DER 50
#define HF_MAX_RESULT_REC 40
#define HF_MAX_FLAGS 35

PROTO(int hf_init, (char *, char *));
PROTO(int hf_lemma, (plangEnvRec, unsigned char *, int, int, int *));
PROTO(int hf_LemmaRawFlags, (plangEnvRec, unsigned char *, int, int, int *));
PROTO(unsigned char * hf_get_res, (void));
PROTO(int hf_cvs, (unsigned char *, unsigned char *));
PROTO(int hf_inv_cvs, (unsigned char *, unsigned char *));
PROTO(int hf_get_expl, (int, char *));
PROTO(void hf_end, (void));

extern unsigned hf_is_ne; /* (bool) nastavi hf_get_res, iff odtrhnute ne- XX_HR 4 */

/* language from dictionary: */
extern char szLangDict[3];

#define UpperCaseCode(c) ((c > 65 && ((c & 1) == 0) && (c != 100)))

#define HF_MODFH
#endif

/*
  passing parameters to programs
  (C) Jan Hajic 1995 
*/

#ifndef HB_ARGH

#include "hb_proto.h"
#include "hb_base.h"

#define HB_ARG_FILES_MAX_DEFAULT 100
#define HB_ARG_SQ '\''
#define HB_ARG_DQ '"'
#define HB_ARG_OPAR '('
#define HB_ARG_CPAR ')'
#define HB_ARG_FS '\6'
#define HB_ARG_BS '\\'
#define HB_ARG_PRE '#'
#define HB_ARG_VAR '$'
#define HB_ARG_SVAR '\1'
#define HB_ARG_EVAR '\2'
#define HB_ARG_ISVAR '{'
#define HB_ARG_IEVAR '}'
#define HB_ARG_VARSEP '\5'
#define HB_ARG_ARGSEP ','
#define HB_ARG_PAR_MAX 36

#define HB_ARG_STDIN "stdin"
#define HB_ARGLOGFILENAME "Log"

typedef struct hb_argAttr {
  char *szAttr;
  char *pszVal;
  char fLocked;
  struct hb_argAttr *paaNext;
} hb_argAttrType;
typedef hb_argAttrType *hb_argAttrPtr;

typedef struct hb_argFile {
  char szFileName[HB_STRINGMAX];
  hb_argAttrPtr paaFirst;
} hb_argFileType;
typedef hb_argFileType *hb_argFilePtr;

#if defined(__cplusplus)
extern "C" {
#endif
PROTO(int hb_ArgOpen, (char *szName));
PROTO(int hb_ArgOpenWithLog, (char *szName));
PROTO(int hb_ArgInit, (longint clMaxFiles));
PROTO(int hb_ArgClose, (void));
PROTO(int hb_ArgGetValue, (char *szArgFileName, char *szAttr,
                             char *szResult, shortint cMaxResult));
PROTO(int hb_ArgSetValue, (char *szArgFileName, char *szAttr, char *szVal));
PROTO(shortint hb_ArgCountFields, (char *szValue));
PROTO(int hb_ArgGetField, (char *szValue, shortint iField, char * szResult,
                           shortint cMaxResult));
#if defined(__cplusplus)
}
#endif

#define HB_ARGH
#endif




#ifndef __HB_CPDH
#define __HB_CPDH

/****************************************************************/
/* lingv. moduly - compiled dictionary retrieval                */
/*                                                              */
/* Changelog:                                                   */
/* v0.9 JH  95/05/08 pattern stuff added                        */
/* v0.1 JH  94/02/13                                            */
/*          initial version (adapted from hh_)                  */
/****************************************************************/

#include "hb_proto.h"
#include "hb_base.h"

#define HB_CPD_FIRST_CHAR ' '
#define HB_CPD_PATSEP '\t'

typedef struct hb_cpdBufferRec {
   Char *pchBufferR;        /* data buffer */
   longint iBufferOffsetR;  /* dynamic variable: current file offset */
                            /* loaded into data buffer */
   Shortint iDxIndexActR;   /* dynamic variable: current buffer # */
                            /* loaded into data buffer */
   Char *pchNextKeyR;       /* ptr to data buffer; 'same count' byte */
                            /* of next key, or the buffer closing 0 */
   Char *pchBufferEndR;     /* points to last buffer char (always 0) */
                            /* actually a 'same chars' count of non-exist */
                            /* data */
   Char szKeyLastR[HB_STRINGMAX]; /* last key found */
   Shortint cKeyCallLenR; /* length of key when FindFirst[Longer] called */
   Shortint cKeyLastLenR; /* length of full key (szKeyLast) */
   struct hb_cpdBufferRec *pcpdBufferNext; /* next buffer */
} hb_cpdBufferRecType;

typedef struct hb_cpdRec {
   int iFileHandle;
   char szFileName[HB_STRINGMAX];
   Char *pchDxSpace;
   Shortint cMaxDxSpace;     /* DxSpace size */
   longint *plDxIndex;
   Shortint cLastDxIndex;    /* last index in plDxIndex + 1 */
   Char *pchBuffer;     /* data buffer */
   Shortint cMaxBuffer;      /* data buffer size */
   Char iRwr[256];      /* frequent data seq - rewrite index */
   Shortint cRwr; /* actual size of rwr space */
   longint cPatNameSpaceMax; /* size of the space for Pattern names */
   Char *rgchPatNameSpace; /* space for Pattern names; */
                           /* 1st pat == 0, next 1, ... */
   longint cMaxData;
   Shortint cMaxKeyLen;
   Shortint cMaxBufNeed;
   tlangEnvRec langEnv;    /* language/environment data */
   longint iBufferOffset;  /* dynamic variable: current file offset */
                           /* loaded into data buffer */
   Shortint iDxIndexAct;   /* dynamic variable: current buffer # */
                           /* loaded into data buffer */
   Char *pchNextKey;       /* ptr to data buffer; 'same count' byte */
                           /* of next key, or the buffer closing 0 */
   Char *pchBufferEnd;     /* points to last buffer char (always 0) */
                           /* actually a 'same chars' count of non-exist */
                           /* data */
   Char szKeyLast[HB_STRINGMAX]; /* last key found */
   Shortint cKeyCallLen; /* length of key when FindFirst[Longer] called */
   Shortint cKeyLastLen; /* length of full key (szKeyLast) */
   hb_cpdBufferRecType *pcpdBufferTop; /* top buffer from stack */
} hb_cpdRecType;


PROTO(hb_cpdRecType *
hb_CpdOpen, (char *szCpdFileName));

PROTO(int
hb_CpdPush, (hb_cpdRecType *cpdFile));

PROTO(int
hb_CpdPop, (hb_cpdRecType *cpdFile));

PROTO(int
hb_CpdClose, (hb_cpdRecType *cpdFile));

PROTO(int
hb_CpdFindFirst, (hb_cpdRecType *pcpd, char *szKey,
                char *szcpdData, int cMaxDataLen,
                int *piPattern,
                int *fCannotFindLonger));

PROTO(int
hb_CpdFindFirstLonger, (hb_cpdRecType *pcpd, char *szKey,
                char *szcpdKey, int cMaxKeyLen,
                char *szcpdData, int cMaxDataLen,
                int *piPattern,
                int *fCannotFindLonger));

PROTO(int
hb_CpdGetNext, (hb_cpdRecType *pcpd,
                char *szcpdData, int cMaxDataLen,
                int *piPattern,
                int *fCannotFindLonger));

PROTO(int
hb_CpdGetNextLonger, (hb_cpdRecType *pcpd,
                char *szcpdKey, int cMaxKeyLen,
                char *szcpdData, int cMaxDataLen,
                int *piPattern,
                int *fCannotFindLonger));

PROTO(int
hb_CpdCvs, (Char *szFrom, Char *szTo));

PROTO(int
hb_CpdInvCvs, (Char *szFrom, Char *szTo));

PROTO(int
hb_CpdPatternOfIndex, (hb_cpdRecType *pcpd, 
                       Char *szPattern, Shortint cMaxLen,
                       Shortint iIndex));

PROTO(int
hb_CpdIndexOfPattern, (hb_cpdRecType *pcpd, 
                       Shortint *piIndex, Char *szPattern));

#endif


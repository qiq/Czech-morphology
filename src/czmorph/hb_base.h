#ifndef __HB_BASEH
#define __HB_BASEH

/****************************************************************/
/* lingv. moduly - spol. cast #include                          */
/*                                                              */
/* Changelog:                                                   */
/* v0.9 JH  95/02/01                                            */
/* v0.1 JH  94/02/16                                            */
/*          pocatecni verze                                     */
/****************************************************************/

#include <stddef.h>

#include "hb_proto.h"

typedef unsigned char Char;
typedef short int shortint;
typedef unsigned short int Shortint;
typedef double Double;
typedef long double Longdouble;
typedef unsigned char byte8;
/* we want longint to be always 32 bits: */
#if defined(__LP64__) || defined(LONG_IS_64)
typedef int longint;
typedef unsigned int Longint;
#else
typedef long int longint;
typedef unsigned long int Longint;
#endif

#define MATH_LOG2E 1.4426950408889634074
#define log2(d) (log(d)*MATH_LOG2E)
#define loge22(dloge) (dloge*MATH_LOG2E)

#define HB_MAXSSHORT 32767
#define HB_CNTSSHORT 32768
#define HB_MAXUSHORT 65535u
#define HB_CNTUSHORT 65536l
#define HB_MAXSLONG 0x7fffffffl

#define HB_STDERRLOG "stderr.log"

#define HB_EXPL_SPACE '_'
#define HB_STRINGMAX 250
#define HB_LONGSTRINGMAX 31000
#define HB_EOS '\0'

#define HB_MAX_READ_WRITE 32000

typedef struct langEnvRec {
	 char NEG_PREFIX[8];   /* negative prefix or empty string if not used
                                  or nonexistent for given language */
         int cNEG_PREFIX; /* length of neg prefix */
	 char GRAD_PREFIX[8];  /* superlative prefix or empty sz if not used
                                  or nonexistent for given language */
         int cGRAD_PREFIX; /* length of neg prefix */
         int fNegFirst;        /* 1 if neg. goes first in a prefix, 0 if sup.*/
         char szLangEnv[3];    /* lang. abbreviation */
         int fCZ; /* set if language is Czech (CZ) */
} tlangEnvRec;
typedef struct langEnvRec *plangEnvRec;

typedef struct hb_cbRec {
         shortint iFree;
	 shortint iStart;
	 longint *rgValue;
	 shortint crgMax;
	 shortint fFull;
	 shortint iAct;
	 shortint iFrame;
} thb_cbRec;
typedef thb_cbRec *phb_cbRec;

typedef struct hb_listszRec { /* list element (strings) */
  char *szStr; /* string -- allocated by insert routines */
  struct hb_listszRec *plistszNext; /* next string or NULL */
} hb_listszRecType;

typedef struct hb_llszRec { /* list element (list of strings) */
  hb_listszRecType *plistszHead; /* linked list head */
  struct hb_llszRec *pllszNext; /* next list element or NULL */
} hb_llszRecType;

/* type int also used when e.g. calling standard functions */

#define HB_NOLINE -1l

/* the following #define-s just for clarity when calling Error: */
#define HB_SYN 1
#define HB_LEM 2
#define HB_BASE 3
#define HB_CPD 4
#define HH_SD 5
#define HB_ARG 6
#define HB_HASH 7
#define HB_APPLICATION 8
#define HB_MAIN 9
#define HB_IOOR 10
#define HB_IOOW 11
#define HB_IOC 13
#define HB_MALLOC 12
#define HB_REALLOC 33
#define HB_IOR 14
#define HB_IOW 15
#define HB_IOSEEK 16
#define HB_LANG 17
#define HB_CBINIT 18
#define HB_LAST_ERROR 37

#ifdef HB_ERROR_REPORTING
static char *hb_szMsg[] = {
		 /* MODULE names must be from 1 to 9 */
		 /* 0 */ "Unknown",
		 /* 1 */ "hb_syn",
		 /* 2 */ "hb_lem",
		 /* 3 */ "hb_base",
		 /* 4 */ "hb_cpd",
		 /* 5 */ "hh_sd",
		 /* 6 */ "hb_arg",
		 /* 7 */ "hb_hash",
		 /* 8 */ "App",
		 /* 9 */ "main",
		 /* 10*/ "Cannot open input file: ",
		 /* 11*/ "Cannot open output file: ",
		 /* 12*/ "Cannot allocate memory; bytes == ",
		 /* 13*/ "Cannot close file with handle: ",
		 /* 14*/ "Cannot read from file with handle: ",
		 /* 15*/ "Cannot write to file with handle: ",
		 /* 16*/ "Cannot seek in file with handle: ",
		 /* 17*/ "Language/environment wrong abbreviation: ",
		 /* 18*/ "To large round buffer alloc request: ",
		 /* 19*/ "Reinitialized; call hb_ArgClose first",
		 /* 20*/ "Cannot allocate ptr range: size requested ",
		 /* 21*/ "Too many open arg files when opening ",
		 /* 22*/ "Allocation error opening ",
		 /* 23*/ "String to be copied too long: ",
		 /* 24*/ "Allocation error -- ",
		 /* 25*/ "Parse error at line ",
		 /* 26*/ "Too few arguments passed to ",
		 /* 27*/ "Too many arguments passed to ",
		 /* 28*/ "Getting file/attr: ",
		 /* 29*/ "Field index out of bounds: ",
                 /* 30*/ "n4001: Cannot write features to ",
                 /* 31*/ "n4001: Wrong iClosestConditionElem: ",
                 /* 32*/ "tag: ",
		 /* 33*/ "Cannot reallocate memory; bytes == ",
                 /* 34*/ "Hash table over 80% full; inserting key ",
                 /* 35*/ "#: ",
                 /* 36*/ "Data too long: ",
		 /* last */ ""
		 };
#endif

#define HB_EXIT(rc) { fprintf(stderr,"exit code: %d\n",rc); \
                      hb_CloseLog(); exit(rc); }

#if defined(__cplusplus)
extern "C" {
#endif
PROTO(void hb_OpenLog, (char *));
PROTO(char *hb_GetLogFileName, (void));
PROTO(void hb_CloseLog, (void));
PROTO(void hb_Error, (char *, char *, char *, int));
PROTO(void hb_FatalError, (int, int, char *));
PROTO(void hb_LnFatalError, (int, int, longint, char *));
PROTO(void hb_Warning, (int, int, char *));
PROTO(void hb_LnWarning, (int, int, longint, char *));
PROTO(void hb_Log, (int, int, char *));
PROTO(void hb_LnLog, (int, int, longint, char *));
PROTO(void *hb_Alloc, (Shortint));
PROTO(void *hb_ReAlloc, (void *, Shortint));
#ifdef unix
PROTO(void *hb_LongAlloc, (longint));
PROTO(void *hb_LongReAlloc, (void *, longint));
#endif
PROTO(void hb_Free, (void *));
PROTO(void hb_SetLangEnv, (plangEnvRec, char *));
PROTO(int hb_Open, (char *, char *));
PROTO(int hb_Close, (int));
PROTO(longint hb_Read, (int, void *, longint));
PROTO(longint hb_ReadMax, (int, void *, longint));
PROTO(longint hb_ReadLn, (int, void *, longint));
PROTO(longint hb_Write, (int, void *, longint));
PROTO(longint hb_Seek, (int, longint, int));

PROTO(int hb_cbInit, (phb_cbRec *, Shortint));
PROTO(int hb_cbInitVal, (phb_cbRec, longint));
PROTO(int hb_cbFree, (phb_cbRec));
PROTO(int hb_cbPut, (phb_cbRec, longint));
PROTO(int hb_cbGet, (phb_cbRec, longint *));

#define HB_LISTSZ_ASC 1
#define HB_LISTSZ_DESC 2
#define HB_LISTSZ_END 3

PROTO(hb_listszRecType *hb_listszNew, (char *));
PROTO(hb_listszRecType *hb_listszNewCopy, (char *));
PROTO(hb_listszRecType *hb_listszFree, (hb_listszRecType *));
PROTO(hb_listszRecType *hb_listszFreeCopy, (hb_listszRecType *));
PROTO(hb_listszRecType *hb_listszFreeList, (hb_listszRecType *));
PROTO(hb_listszRecType *hb_listszFreeListCopy, (hb_listszRecType *));
PROTO(hb_listszRecType *hb_listszPush, (hb_listszRecType *,
              hb_listszRecType *));
PROTO(hb_listszRecType *hb_listszInList, (hb_listszRecType *, char *));
PROTO(hb_listszRecType *hb_listszSortIn, (hb_listszRecType **, char *,
                                          int, int));
PROTO(hb_listszRecType *hb_listszSortInCopy, (hb_listszRecType **, char *,
                                          int, int));

PROTO(hb_llszRecType *hb_llszNew, (hb_listszRecType *));
PROTO(hb_llszRecType *hb_llszFree, (hb_llszRecType *));
PROTO(hb_llszRecType *hb_llszPush, (hb_llszRecType *,
              hb_llszRecType *));

#if defined(__cplusplus)
}
#endif

#define hb_toupper_ascii(c) ((c <= 'z' && c >= 'a') ? (c-32) : c)
#define hb_cbNext(p,iIndex) ((p != NULL) ? \
			     ((iIndex+1 >= p->crgMax) ? 0 : iIndex+1) : -1)
#define hb_cbPrev(p,iIndex) ((p != NULL) ? \
			     ((iIndex <= 0) ? p->crgMax-1 : iIndex-1) : -1)
#define hb_cbFull(p) ((p != NULL) ? (p->fFull) : -1)
#define hb_cbEmpty(p) ((p != NULL) ? (p->iStart == p->iFree && !(p->fFull))\
		       : -2)
#define hb_cbGetStart(p) ((p != NULL) ? p->iStart : -1)
#define hb_cbGetFree(p) ((p != NULL) ? p->iFree : -1)
#define hb_cbFill(p) (p->iFree = hb_cbNext(p,p->iFree))
#define hb_cbGetAct(p) ((p != NULL) ? p->iAct : -1)
#define hb_cbIncAct(p) ((p != NULL) ? p->iAct = hb_cbNext(p,p->iAct) : -1)
#define hb_cbGetFrame(p) ((p != NULL) ? p->iFrame : -1)
#define hb_cbSetFrame(p,i) ((p != NULL) ? p->iFrame = i : -1)
#define hb_cbIncFrame(p) ((p != NULL) ? \
			  p->iFrame = hb_cbNext(p,p->iFrame) : -1)
#define hb_cbValue(p,iIndex) ((p != NULL && iIndex < p->crgMax && iIndex >= 0)\
			      ? p->rgValue[iIndex] : 0l)
#define hb_cbStore(p,iIndex,ilVal) if (p != NULL && iIndex < p->crgMax && \
                     iIndex >= 0) { p->rgValue[iIndex] = (longint) ilVal; }
#define hb_cbDiscardFirst(p) ((p != NULL) ? \
			      (p->iStart = \
			       hb_cbNext(p,p->iStart), p->fFull = 0)\
			      : 0)
#define hb_cbDiff(p,i1,i2) ((p != NULL) ? \
			    ((i1 < i2) ? \
			     i2-i1 : \
					 ((i1 == i2) ? \
			      ((p->fFull) ? p->crgMax : 0) \
			      : p->crgMax+i2-i1) ) \
			    : -1)
#define hb_cbDump(p) fprintf(stderr,\
 "cbDump: size %d iStart %d iFree %d fFull %d iAct %d iFrame %d &Value %lx\n",\
         p->crgMax,p->iStart,p->iFree,p->fFull,p->iAct,p->iFrame,p->rgValue)
#endif


/****************************************************************/
/* lingv. moduly - spol. cast kod                               */
/*                                                              */
/* Changelog:                                                   */
/* v0.1 JH  94/02/16                                            */
/*          pocatecni verze                                     */
/****************************************************************/


#include <malloc.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#if defined(unix) && !defined(win)
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#else
#if defined(win)
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#else
#include <io.h>
#include <sys\stat.h>
#include <share.h>
#include <stdlib.h>
#include <process.h>
#endif
#endif

#include "hb_proto.h"
#include "hb_base.h"

/* the following constant should be [1-9]0000, [1-9]000, or [1-9]00 */
#define CONST_ERROR_RG_START 10000

/* the following constant should be 1000 or 100 or 10: */
#define CONST_ERROR_MOD_SHIFT 1000

/* local definitions: */
hb_listszRecType *hb_listszFreeHead = NULL;
hb_llszRecType *hb_llszFreeHead = NULL;

/* log file definitions: */
char hb_szLogFileName[HB_STRINGMAX] = { HB_EOS };
FILE *hb_fLogFile = NULL;

FUNDEF1(void
hb_OpenLog,
char *, szFileName)
{
FILE *fNewLog;
int fStdLog;
  fStdLog = 0;
  if (szFileName == NULL) fStdLog = 1;
  else if (*szFileName == HB_EOS) fStdLog = 1;
  if (!fStdLog) {
    fNewLog = freopen(szFileName,"at",stderr);
  }
  if (fStdLog || fNewLog == NULL) {
    strcpy(hb_szLogFileName,HB_STDERRLOG);
    fNewLog = freopen(hb_szLogFileName,"at",stderr);
    if (fNewLog == NULL) {
      fNewLog = stderr;
      strcpy(hb_szLogFileName,"<stderr>");
    }
  }
  else {
    strcpy(hb_szLogFileName,szFileName);
  }
  hb_fLogFile = fNewLog;
} /* of hb_OpenLog */

FUNDEF0(char *
hb_GetLogFileName)
{
  return hb_szLogFileName;
} /* of hb_GetLogFileName */

FUNDEF0(void
hb_CloseLog)
{
  fclose(hb_fLogFile);
  hb_fLogFile = NULL;
  strcpy(hb_szLogFileName,"");
} /* of hb_CloseLog */

FUNDEF1(hb_listszRecType *
hb_listszNew,
char *, pszStr)
{
           /* allocate new record, do not copy string -- save pointer */
Shortint cAlloc;
hb_listszRecType *listNew;

  if (hb_listszFreeHead != NULL) {
    listNew = hb_listszFreeHead;
    hb_listszFreeHead = hb_listszFreeHead->plistszNext;
  }
  else {
    cAlloc = sizeof(hb_listszRecType);
    if (NULL == (listNew = (hb_listszRecType *) hb_LongAlloc(cAlloc))) {
      return NULL;
    }
  }
  listNew->szStr = pszStr;
  listNew->plistszNext = NULL;
  return listNew;
} /* of hb_listszNew */

FUNDEF1(hb_listszRecType *
hb_listszNewCopy,
char *, pszStr)
{
           /* allocate new record, allocate & copy string */
Shortint cAlloc;
char *szStrCopy;

  if (strlen(pszStr) > 32000) return NULL;
  cAlloc = sizeof(char) * (strlen(pszStr)+1);
  if (NULL == (szStrCopy = (char *) hb_LongAlloc(cAlloc))) {
    return NULL;
  }
  strcpy(szStrCopy,pszStr);
  return hb_listszNew(szStrCopy);
} /* of hb_listszNewCopy */

FUNDEF1(hb_listszRecType *
hb_listszFree,
hb_listszRecType *, plistszToFree)
{
           /* delete record (--> free list), do not delete string */
  plistszToFree->plistszNext = hb_listszFreeHead;
  hb_listszFreeHead = plistszToFree;
  return hb_listszFreeHead;
} /* of hb_listszFree */

FUNDEF1(hb_listszRecType *
hb_listszFreeCopy,
hb_listszRecType *, plistszToFree)
{
           /* delete record (--> free list), delete string */
  hb_Free(plistszToFree->szStr);
  return hb_listszFree(plistszToFree);
} /* of hb_listszFreeCopy */

FUNDEF1(hb_listszRecType *
hb_listszFreeList,
hb_listszRecType *, plistszToFree)
{
hb_listszRecType *plistsz;
hb_listszRecType *plistszTF;
hb_listszRecType *plistszLastFreed;
           /* delete list elements of strings, do NOT delete the strings */
  plistsz = plistszToFree;
  while (plistsz != NULL) {
    plistszTF = plistsz;
    plistsz = plistsz->plistszNext;
    plistszLastFreed = hb_listszFree(plistszTF);
  }
  return plistszLastFreed;
} /* of hb_listszFreeList */

FUNDEF1(hb_listszRecType *
hb_listszFreeListCopy,
hb_listszRecType *, plistszToFree)
{
hb_listszRecType *plistsz;
hb_listszRecType *plistszTF;
hb_listszRecType *plistszLastFreed;

           /* delete list of strings, delete list elements */
  plistsz = plistszToFree;
  while (plistsz != NULL) {
    plistszTF = plistsz;
    plistsz = plistsz->plistszNext;
    plistszLastFreed = hb_listszFreeCopy(plistszTF);
  }
  return plistszLastFreed;
} /* of hb_listszFreeListCopy */

FUNDEF2(hb_listszRecType *
hb_listszPush,
hb_listszRecType *, plistszHead,
hb_listszRecType *, plistszNew)
{
  plistszNew->plistszNext = plistszHead;
  return plistszNew ;
} /* of hb_listszPush */

FUNDEF2(hb_listszRecType *
hb_listszInList,
hb_listszRecType *, plistszHead,
char *, pszStr)
{
hb_listszRecType *plistszAct;
  plistszAct = plistszHead;
  while (plistszAct != NULL) {
    if (!strcmp(pszStr,plistszAct->szStr)) {
      return plistszAct; /* string found! */
    }
    plistszAct = plistszAct->plistszNext;
  }
  return NULL; /* not found */
} /* of hb_listszInList */

FUNDEF4(hb_listszRecType *
hb_listszSortIn,
hb_listszRecType **, pplistszHead,
char *, pszIn,
int, fOrder,
int, fUnique)
{
  /* creates record with pszIn, and sorts it in the list *pplistszHead
     according to fOrder (asc, desc, end of list) */
hb_listszRecType *plistszAct;
hb_listszRecType *plistszPrev;
hb_listszRecType *plistszNew;
int fCmp;

  plistszPrev = NULL;
  plistszAct = *pplistszHead;
  while (plistszAct != NULL) {
    if (fOrder != HB_LISTSZ_END) {
      fCmp = strcmp(pszIn,plistszAct->szStr);
      if (fOrder == HB_LISTSZ_ASC) {
        if (fCmp < 0) { /* input string smaller, i.e. goes before Act (ASC) */
          break;
        }
        else if (fUnique && fCmp == 0) { /* do not allocate new record */
          return plistszAct;
        }
      }
      else if (fOrder == HB_LISTSZ_DESC) {
        if (fCmp > 0) { /* input string greater, i.e. *before* Act (DESC) */
          break;
        }
        else if (fUnique && fCmp == 0) { /* do not allocate new record */
          return plistszAct;
        }
      }
      else { /* wrong fOrder */
        return NULL; 
      }
    }
    plistszPrev = plistszAct;
    plistszAct = plistszAct->plistszNext;
  } /* of loop through the list */
  if ((plistszNew = hb_listszNewCopy(pszIn)) == NULL) {
    return NULL; /* cannot allocate new record */
  }
  /* link from previous, or link from head: */
  if (plistszPrev == NULL) {
    *pplistszHead = plistszNew;
  }
  else {
    plistszPrev->plistszNext = plistszNew;
  }
  /* link new element forward: */
  plistszNew->plistszNext = plistszAct;

  return plistszNew;
} /* of hb_listszSortIn */

FUNDEF4(hb_listszRecType *
hb_listszSortInCopy,
hb_listszRecType **, pplistszHead,
char *, pszIn,
int, fOrder,
int, fUnique)
{
  /* creates record with pszIn, and sorts it in the list *pplistszHead
     according to fOrder (asc, desc, end of list) */
           /* allocate new record, allocate & copy string */
Shortint cAlloc;
char *szStrCopy;

  if (strlen(pszIn) > 32000) return NULL;
  cAlloc = sizeof(char) * (strlen(pszIn)+1);
  if (NULL == (szStrCopy = (char *) hb_LongAlloc(cAlloc))) {
    return NULL;
  }
  strcpy(szStrCopy,pszIn);
  return hb_listszSortIn(pplistszHead,szStrCopy,fOrder,fUnique);
} /* of hb_listszSortInCopy */

FUNDEF1(hb_llszRecType *
hb_llszNew,
hb_listszRecType *, plistszList)
{
           /* allocate new record, do not copy string -- save pointer */
Shortint cAlloc;
hb_llszRecType *llNew;

  if (hb_llszFreeHead != NULL) {
    llNew = hb_llszFreeHead;
    hb_llszFreeHead = hb_llszFreeHead->pllszNext;
  }
  else {
    cAlloc = sizeof(hb_llszRecType);
    if (NULL == (llNew = (hb_llszRecType *) hb_LongAlloc(cAlloc))) {
      return NULL;
    }
  }
  llNew->plistszHead = plistszList;
  llNew->pllszNext = NULL;
  return llNew;
} /* of hb_llszNew */

FUNDEF1(hb_llszRecType *
hb_llszFree,
hb_llszRecType *, pllszToFree)
{
           /* delete record (--> free ll), do not delete string */
  pllszToFree->pllszNext = hb_llszFreeHead;
  hb_llszFreeHead = pllszToFree;
  return hb_llszFreeHead;
} /* of hb_llszFree */

FUNDEF2(hb_llszRecType *
hb_llszPush,
hb_llszRecType *, pllszHead,
hb_llszRecType *, pllszNew)
{
  pllszNew->pllszNext = pllszHead;
  return pllszNew ;
} /* of hb_llszPush */

/********************************************* END OF LIST SZ FUNCTIONS ****/
FUNDEF3(void
hb_FatalError,
int, iModule, int, iMsg, char *, szSuppString)
{

int iError;

   iError = CONST_ERROR_RG_START + (CONST_ERROR_MOD_SHIFT*iModule)
            + iMsg;
   if (iMsg > HB_LAST_ERROR) {
#ifdef HB_ERROR_REPORTING
     fprintf(stderr,"%s: Fatal(%05d): undefined error/%s\n",
           hb_szMsg[iModule],iError,szSuppString);
#else
     fprintf(stderr,"%d: Fatal(%05d): undefined error/%s\n",
           iModule,iError,szSuppString);
#endif
     exit(iError);
   }
#ifdef HB_ERROR_REPORTING
   fprintf(stderr,"%s: Fatal(%05d): %s%s\n",
           hb_szMsg[iModule],iError,hb_szMsg[iMsg],szSuppString);
#ifndef HB_DONOTEXIT
   exit(iError);
#endif
#else
   fprintf(stderr,"HB Fatal(%05d)/%s\n",
           iError,szSuppString);
#ifndef HB_DONOTEXIT
   exit(iError);
#endif
#endif
} /* of hb_FatalError */

FUNDEF3(void
hb_Warning,
int, iModule, int, iMsg, char *, szSuppString)
{
int iError;

   iError = CONST_ERROR_RG_START + (CONST_ERROR_MOD_SHIFT*iModule)
            + iMsg;
   if (iMsg > HB_LAST_ERROR) {
#ifdef HB_ERROR_REPORTING
     fprintf(stderr,"%s: Warn(%05d): undefined code/%s\n",
           hb_szMsg[iModule],iError,szSuppString);
#else
     fprintf(stderr,"%d: Warn(%05d): undefined code/%s\n",
           iModule,iError,szSuppString);
#endif
     /* exit(iError); */
   }
#ifdef HB_ERROR_REPORTING
   fprintf(stderr,"%s: Warn(%05d): %s%s\n",
           hb_szMsg[iModule],iError,hb_szMsg[iMsg],szSuppString);
#else
   fprintf(stderr,"HB Warn(%05d)/%s\n",
           iError,szSuppString);
#endif
} /* of hb_Warning */

FUNDEF3(void
hb_Log,
int, iModule, int, iMsg, char *, szSuppString)
{
int iError;

   iError = CONST_ERROR_RG_START + (CONST_ERROR_MOD_SHIFT*iModule)
            + iMsg;
   if (iMsg > HB_LAST_ERROR) {
#ifdef HB_ERROR_REPORTING
     fprintf(stderr,"%s: Log(%05d): undefined code/%s\n",
           hb_szMsg[iModule],iError,szSuppString);
#else
     fprintf(stderr,"%d: Log(%05d): undefined code/%s\n",
           iModule,iError,szSuppString);
#endif
     /* exit(iError); */
   }
#ifdef HB_ERROR_REPORTING
   fprintf(stderr,"%s",hb_szMsg[iModule]);
   fprintf(stderr," %s%s\n",hb_szMsg[iMsg],szSuppString);
#else
   fprintf(stderr,"HB(%05d)",iError);
   fprintf(stderr," %s\n",szSuppString);
#endif
} /* of hb_Log */

FUNDEF5(void
hb_LnMsgAdd,
int, iModule, int, iMsg, longint, ilLine, 
void, MsgFun(int, int, char *), char *, szSuppString)
{
char *szLnSuppString;
  if (ilLine != HB_NOLINE) {
    szLnSuppString = (char *) hb_LongAlloc((strlen(szSuppString)+28)*sizeof(char));
    if (szLnSuppString == NULL) {
      fprintf(stderr,
        "hb_base: fatal - no mem in hb_LnWarning/%s\n",szSuppString);
      exit(9991);
    }
    sprintf(szLnSuppString,"[ln %ld] %s",ilLine,szSuppString);
  }
  else {
    szLnSuppString = szSuppString;
  }
  MsgFun(iModule,iMsg,szLnSuppString);
  if (ilLine != HB_NOLINE) {
    hb_Free(szLnSuppString);
  }
} /* of hb_LnMsgAdd */

FUNDEF4(void
hb_LnLog,
int, iModule, int, iMsg, longint, ilLine, char *, szSuppString)
{
  hb_LnMsgAdd(iModule,iMsg,ilLine,hb_Log,szSuppString);
} /* of hb_LnLog */

FUNDEF4(void
hb_LnWarning,
int, iModule, int, iMsg, longint, ilLine, char *, szSuppString)
{
  hb_LnMsgAdd(iModule,iMsg,ilLine,hb_Warning,szSuppString);
} /* of hb_LnLog */

FUNDEF4(void
hb_LnFatalError,
int, iModule, int, iMsg, longint, ilLine, char *, szSuppString)
{
  hb_LnMsgAdd(iModule,iMsg,ilLine,hb_FatalError,szSuppString);
} /* of hb_LnLog */

FUNDEF4(void
hb_Error,
char *, szPN,
char *, szText,
char *, szText2,
int, iExit)
{
   fprintf(stderr,"%s: %s%s\n",szPN,szText,szText2);
   if (iExit > 0) {
     exit(iExit);
   }
} /* of hb_Error */

FUNDEF1(void *
hb_Alloc,
Shortint, cSize)
{
void *
pResult;
char szBytes[12];
   pResult = malloc(cSize);
   if (pResult == NULL) {
     sprintf(szBytes,"%d",cSize);
     hb_FatalError(HB_BASE,HB_MALLOC,szBytes);
   }
   return pResult;
} /* of hb_Alloc */

FUNDEF2(void *
hb_ReAlloc,
void *, ptr,
Shortint, cSize)
{
void *
pResult;
char szBytes[12];
   pResult = realloc(ptr,cSize);
   if (pResult == NULL) {
     sprintf(szBytes,"%d",cSize);
     hb_FatalError(HB_BASE,HB_REALLOC,szBytes);
   }
   return pResult;
} /* of hb_ReAlloc */

#ifdef unix
FUNDEF1(void *
hb_LongAlloc,
longint, clSize)
{
void *
pResult;
char szBytes[12];
   pResult = malloc((unsigned int)clSize);
   if (pResult == NULL) {
     sprintf(szBytes,"%ld",clSize);
     hb_FatalError(HB_BASE,HB_MALLOC,szBytes);
   }
   return pResult;
} /* of hb_LongAlloc */

FUNDEF2(void *
hb_LongReAlloc,
void *, ptr,
longint, clSize)
{
void *
pResult;
char szBytes[12];
   pResult = realloc(ptr,(unsigned int)clSize);
   if (pResult == NULL) {
     sprintf(szBytes,"%ld",clSize);
     hb_FatalError(HB_BASE,HB_REALLOC,szBytes);
   }
   return pResult;
} /* of hb_LongReAlloc */
#endif

FUNDEF1(void
hb_Free,
void *, p)
{
  /**/ /* fprintf(stderr,"freeing ptr %x: %c%c...\n",p,((char *)p)[0],
	       ((char *)p)[1]); /**/
   if (p != NULL)
     free(p);
} /* of hb_Free */

FUNDEF2(void
hb_SetLangEnv,
plangEnvRec, plang, 
char *, szLangEnvPar)
{
int i;
   for (i = 0; i < strlen(szLangEnvPar); i++)
     plang->szLangEnv[i] = hb_toupper_ascii(szLangEnvPar[i]);
   plang->szLangEnv[i] = '\0';

   plang->fCZ = 0;
   if (!strcmp(plang->szLangEnv,"CZ")) {
     strcpy(plang->NEG_PREFIX,"ne");
     strcpy(plang->GRAD_PREFIX,"nej");
     plang->fNegFirst = 0;
     plang->fCZ = 1;
   }
   else if (!strcmp(plang->szLangEnv,"SK")) {
     strcpy(plang->NEG_PREFIX,"ne");
     strcpy(plang->GRAD_PREFIX,"naj");
     plang->fNegFirst = 0;
   }
   else if (!strcmp(plang->szLangEnv,"PL")) {
     strcpy(plang->NEG_PREFIX,"nie");
     strcpy(plang->GRAD_PREFIX,"naj");
     plang->fNegFirst = 1;
   }
   else if (!strcmp(plang->szLangEnv,"EN")) {
     strcpy(plang->NEG_PREFIX,"");
     strcpy(plang->GRAD_PREFIX,"");
     plang->fNegFirst = 0;
   }
   else if (!strcmp(plang->szLangEnv,"GE")) {
     strcpy(plang->NEG_PREFIX,"");
     strcpy(plang->GRAD_PREFIX,"");
     plang->fNegFirst = 0;
   }
   else {
     hb_FatalError(HB_BASE,HB_LANG,plang->szLangEnv);
   }
   /* set prefix lengths: */
   plang->cNEG_PREFIX = strlen(plang->NEG_PREFIX);
   plang->cGRAD_PREFIX = strlen(plang->GRAD_PREFIX);
} /* of hb_SetLangEnv */

FUNDEF2(int
hb_Open, 
char *, szFileName, 
char *, szMode)
{
int fModes;
int iResult;
int iError;
int fModes2;
   fModes = 0;
   if (*szMode == 'r') {
     iError = HB_IOOR;
#if defined(unix) && !defined(win)
     fModes |= O_RDONLY;
     fModes2 = 0777; /* as in chmod, & ~umask, !! ignored for 'r' access */
#else
#if defined(win)
     fModes |= O_RDONLY|O_BINARY /*|SH_DENYNONE*/ ;
     fModes2 = S_IREAD;
#else
     fModes |= O_RDONLY|O_BINARY|SH_DENYNONE;
     fModes2 = S_IREAD;
#endif
#endif
   }
   else if (*szMode == 'w') {
     iError = HB_IOOW;
#if defined(unix) && !defined(win)
     fModes |= O_WRONLY|O_CREAT|O_TRUNC;
     fModes2 = 0777; /* as in chmod, & ~umask */
#else
     if (szMode[1] == 't') {
       fModes |= O_WRONLY|O_CREAT|O_TRUNC|O_TEXT;
     }
     else {
       fModes |= O_WRONLY|O_CREAT|O_TRUNC|O_BINARY;
     }
     fModes2 = S_IWRITE;
#endif
   }
#if defined(unix) && !defined(win)
   iResult = open(szFileName,fModes,fModes2);
#else
   iResult = open(szFileName,fModes,fModes2);
#endif
   if (iResult < 0) {
     hb_FatalError(HB_BASE,iError,szFileName);
   }
   return(iResult);
} /* of hb_Open */

FUNDEF1(int
hb_Close,
int, iFileHandle)
{
char sz[10];
   if (close(iFileHandle) < 0) {
     sprintf(sz,"%d",iFileHandle);
     hb_FatalError(HB_BASE,HB_IOC,sz);
   }
   return 0;
} /* of hb_Close */

FUNDEF3(longint
hb_Read,
int, iFileHandle, 
void *, pTo, 
longint, cBytes)
{

char sz[100];
int iiResult;
size_t cReadBytes;
longint iResult;
longint cBytesRemaining;
byte8 *pToAct;

   pToAct = pTo;
   cBytesRemaining = cBytes;
   while (cBytesRemaining > 0) {
     if (cBytesRemaining > HB_MAX_READ_WRITE) {
       cReadBytes = HB_MAX_READ_WRITE;
     }
     else cReadBytes = cBytesRemaining;
     cBytesRemaining -= cReadBytes;
     if ((iiResult = read(iFileHandle, (void *) pToAct, cReadBytes)) != 
       cReadBytes) {
       if (iiResult == 0) return 0/* EOF */;
       /* else error: */
       sprintf(sz,"%d, bytes %u, remaining %ld, read %d",
	       iFileHandle,cReadBytes,cBytesRemaining,iiResult);
       hb_FatalError(HB_BASE,HB_IOR,sz);
       sprintf(sz,"%d, offset %ld",
	       iFileHandle,lseek(iFileHandle,0l,SEEK_CUR));
       hb_FatalError(HB_BASE,HB_IOR,sz);
       return -1l;
     }
     pToAct += cReadBytes;
   } /* of while cBytesRemaining > 0 */

   iResult = cBytes;
   return iResult;
} /* of hb_Read */

FUNDEF3(longint
hb_ReadMax,
int, iFileHandle, 
void *, pTo, 
longint, cBytes)
{

char sz[100];
int iiResult;
size_t cReadBytes;
longint iResult;
longint cBytesRemaining;
byte8 *pToAct;

   pToAct = pTo;
   iResult = 0l;
   cBytesRemaining = cBytes;
   while (cBytesRemaining > 0) {
     if (cBytesRemaining > HB_MAX_READ_WRITE) {
       cReadBytes = HB_MAX_READ_WRITE;
     }
     else cReadBytes = cBytesRemaining;
     cBytesRemaining -= cReadBytes;
     if ((iiResult = read(iFileHandle, (void *) pToAct, cReadBytes)) != 
       cReadBytes) {
       if (iiResult < 0) { /* error */
         sprintf(sz,"%d, bytes %u, remaining %ld, read %d",
	         iFileHandle,cReadBytes,cBytesRemaining,iiResult);
         hb_FatalError(HB_BASE,HB_IOR,sz);
         sprintf(sz,"%d, offset %ld",
	         iFileHandle,lseek(iFileHandle,0l,SEEK_CUR));
         hb_FatalError(HB_BASE,HB_IOR,sz);
         return -1l;
       }
       iResult += iiResult; /* truncated number of bytes read
                               (less than specified max) */
       return iResult; /* EOF (0) if first read */
     }
     iResult += cReadBytes;
     pToAct += cReadBytes;
   } /* of while cBytesRemaining > 0 */
   return iResult; /* full number of bytes read */
} /* of hb_ReadMax */

FUNDEF3(longint
hb_ReadLn,
int, iFileHandle, 
void *, pTo, 
longint, cBytes)
{

char sz[100];
int iiResult;
size_t cReadBytes;
longint iResult;
longint cBytesRemaining;
byte8 *pToAct;

   iResult = 0l;
   pToAct = pTo;
   while (iResult < cBytes) {
     if ((iiResult = read(iFileHandle, (void *) pToAct, 1)) != 1) {
       if (iiResult == 0) { /* eof */
         *pToAct = HB_EOS;
         return iResult; /* returns 0 if eof at the beginning */
       }
       if (iiResult < 0) { /* error */
         sprintf(sz,"%d, bytes %lu",
	         iFileHandle,iResult);
         hb_FatalError(HB_BASE,HB_IOR,sz);
         sprintf(sz,"%d, offset %ld",
	         iFileHandle,lseek(iFileHandle,0l,SEEK_CUR));
         hb_FatalError(HB_BASE,HB_IOR,sz);
         *pToAct = HB_EOS;
         return -1l;
       } /* of error */
     } /* eof or error */
     if (*pToAct == '\n') {
       *pToAct = HB_EOS;
       return iResult; /* returns length of line read, without \n! */
     }
     iResult++;
     pToAct++;
   } /* of iResult < cBytes */
   *pToAct = HB_EOS;
   return iResult; /* returns length of line read (truncated, not at end of
                      line) */
} /* of hb_ReadLn */

FUNDEF3(longint
hb_Write,
int, iFileHandle, 
void *, pFrom, 
longint, cBytes)
{
char sz[100];
int iiResult;
size_t cWriteBytes;
longint iResult;
longint cBytesRemaining;
byte8 *pFromAct;

   pFromAct = pFrom;
   cBytesRemaining = cBytes;
   while (cBytesRemaining > 0) {
     if (cBytesRemaining > HB_MAX_READ_WRITE) {
       cWriteBytes = HB_MAX_READ_WRITE;
     }
     else cWriteBytes = cBytesRemaining;
     cBytesRemaining -= cWriteBytes;
     if ((iiResult = write(iFileHandle, (void *) pFromAct, cWriteBytes)) != 
       cWriteBytes) {
       /* else error: */
       sprintf(sz,"%d, bytes to write %u, remaining %ld, written %d",
	       iFileHandle,cWriteBytes,cBytesRemaining,iiResult);
       hb_FatalError(HB_BASE,HB_IOW,sz);
       return -1l;
     }
     pFromAct += cWriteBytes;
   } /* of while cBytesRemaining > 0 */
   iResult = cBytes;
   return iResult;
} /* hb_Write */

FUNDEF3(longint
hb_Seek,
int, iFileHandle, 
longint, cOffset, 
int, fType)
{
longint ilResult;
char sz[10];
   /**/ /* fprintf(stderr,"seeking to %ld at %d\n",cOffset,fType); */
   ilResult = lseek(iFileHandle, cOffset, fType);
   /**/ /* fprintf(stderr,"returned: %ld\n",ilResult); */
   if (ilResult < 0) {
     sprintf(sz,"%d",iFileHandle);
     hb_FatalError(HB_BASE,HB_IOSEEK,sz);
   }
   return ilResult;
} /* of hb_Seek */

FUNDEF2(int 
hb_cbInit, 
phb_cbRec *, ppcb,
Shortint, cSize)
{
char sz[12];

  if (cSize > 8000) {
    sprintf(sz,"%d",cSize);
    hb_FatalError(HB_BASE,HB_CBINIT,sz);
    cSize = 1000;
  }
  if ((*ppcb = (thb_cbRec *) hb_LongAlloc(sizeof(thb_cbRec))) == NULL) return 1;
  if (((*ppcb)->rgValue = 
         (longint *) hb_LongAlloc(sizeof(longint)*cSize)) == NULL) return 2;
  (*ppcb)->crgMax = cSize;
  (*ppcb)->iFree = 0;
  (*ppcb)->iStart = 0;
  (*ppcb)->fFull = 0;
  (*ppcb)->iAct = 0;
  return 0;
} /* of hb_cbInit */

FUNDEF2(int 
hb_cbInitVal, 
phb_cbRec, pcb,
longint, cInit)
{
shortint i;

  for (i = 0; i < pcb->crgMax; i++) {
    pcb->rgValue[i] = cInit;
  }
  pcb->iFree = 0;
  pcb->iStart = 0;
  pcb->iAct = 0;
  pcb->fFull = 0;
  return 0;
} /* of hb_cbInitVal */

FUNDEF1(int
hb_cbFree, 
phb_cbRec, pcb)
{
  hb_Free((void *) (pcb->rgValue));
  hb_Free((void *) pcb);
  return 0;
} /* of hb_cbFree */

FUNDEF2(int
hb_cbPut, 
phb_cbRec, pcb, 
longint, iValue)
{
  pcb->rgValue[pcb->iFree] = iValue;
  pcb->iFree++;
  if (pcb->iFree >= pcb->crgMax) pcb->iFree = 0;
  pcb->fFull = (pcb->iStart == pcb->iFree);
  return 0;
} /* of hb_cbPut */

FUNDEF2(int
hb_cbGet, 
phb_cbRec, pcb, 
longint *, piValue)
{
  *piValue = pcb->rgValue[pcb->iStart];
  pcb->iStart++;
  if (pcb->iStart >= pcb->crgMax) pcb->iStart = 0;
  pcb->fFull = 0;
  return 0;
} /* of hb_cbGet */

/* end of hb_base.c */

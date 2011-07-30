/****************************************************************/
/* lingv. moduly - compiled dictionary retrieval (code)         */
/*                                                              */
/* Changelog:                                                   */
/* v0.1 JH  94/02/13                                            */
/*          initial version (adapted from hh_)                  */
/****************************************************************/

#include "hb_proto.h"
#include "hb_base.h"
#include "hh_cp.h"
#include "hb_cpd.h"

#include <stdio.h>
#include <string.h>
#ifdef unix
#else
#include <alloc.h>
#endif

/* 2001.07.09 version 4: added verison info, long keys possible */
#define VERSION 4

/* private functions dcl: */
int
hb_CpdGetDataBlock(hb_cpdRecType *pcpd, char *szKey, int *pfFound);

int
hb_CpdReadDataBlock(hb_cpdRecType *pcpd, int iXtable);

int
hb_CpdGetData(hb_cpdRecType *pcpd,
              Char *pchData,
              char *szcpdData, int cMaxDataLen,
              int *piPattern);

/**********************************************************/
/*                                                        */
/* konverzni tabulka - kam                                */
/*                                                        */
/* (C) Jan Haji‡ 1994                                     */
/*                                                        */
/**********************************************************/

unsigned char *hb_cpdTab;

Char hb_cpd895[256] = {
	00,01,02,03,04,05,06,07, 8, 9,10,11,12,13,14,15,
	16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,
	32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,
	48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,
	250, 66, 74, 92,110,118,126,134, 172,180,188,226,244,252,254,248,
	240,222,184,176,138,130,122,114,  96, 78, 70,231,233,235,237,239,
	100, 67, 75, 93,111,119,127,135, 173,181,189,227,245,253,255,249,
	241,223,185,177,139,131,123,115,  97, 79, 71,247,251,141,143,145,

	 68,171,121, 73,113, 72, 76, 69,  91, 90, 94, 98,179, 95,112,116,
	120,125,124,229,137,128,225,132, 183,136,170,174,178,182,186, 77,
	117, 99,129,133,221,220,224,228, 175,187,243,242,160,246,161,162,
	64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
	64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
	64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
	64,65,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
	64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64
};

Char hb_cpdInvTab[256];

int
hb_CpdFillInvTab(void);

static int hb_fCpdInvTabFilled = 0;


/* public functions def: */

FUNDEF1(hb_cpdRecType *
hb_CpdOpen,
char *, szCpdFileName)
{
hb_cpdRecType *pcpdNew;
char rgchBuffer[9];
Longint iOffset;
Shortint i;
Shortint cBufLen;
Shortint iTmp;
longint cBytesToRead;
Shortint cPatMaxShort; /* pattern space, short int */
int i1;
char szT[2000];

   memset(rgchBuffer, 0, 9);
   if ((pcpdNew = (hb_cpdRecType *) hb_LongAlloc(sizeof(hb_cpdRecType))) ==
     NULL) {
     return NULL;
   }
   if ((pcpdNew->iFileHandle = hb_Open(szCpdFileName,"r")) < 0) {
     free(pcpdNew);
     return NULL;
   }
   strcpy(pcpdNew->szFileName,szCpdFileName);
   cBytesToRead = 4; hb_Read(pcpdNew->iFileHandle, rgchBuffer, cBytesToRead);
   sscanf(rgchBuffer,"%04hx",&(iTmp));
   if (iTmp != VERSION) {
     /* wrong dictionary version! 2001.07.09 */
     i1 = iTmp;
     sprintf(szT,"file %s, has ver %d, need ver %d",szCpdFileName,i1,VERSION);
     hb_FatalError(HB_CPD,956,szT);
     return NULL;
   }
   cBytesToRead = 4; hb_Read(pcpdNew->iFileHandle, rgchBuffer, cBytesToRead);
   sscanf(rgchBuffer,"%04hx",&(iTmp));
   cBytesToRead = 8; hb_Read(pcpdNew->iFileHandle, rgchBuffer, cBytesToRead);
#ifdef LONG_IS_64
   sscanf(rgchBuffer,"%08x",&(pcpdNew->cMaxData)); /* cMaxData is int */
#else
   sscanf(rgchBuffer,"%08lx",&(pcpdNew->cMaxData));
#endif
   cBytesToRead = 4; hb_Read(pcpdNew->iFileHandle, rgchBuffer, cBytesToRead);
   sscanf(rgchBuffer,"%04hx",&(pcpdNew->cMaxBuffer));
   cBytesToRead = 4; hb_Read(pcpdNew->iFileHandle, rgchBuffer, cBytesToRead);
   sscanf(rgchBuffer,"%04hx",&(pcpdNew->cMaxDxSpace));
   cBytesToRead = 4; hb_Read(pcpdNew->iFileHandle, rgchBuffer, cBytesToRead);
   sscanf(rgchBuffer,"%04hx",&(pcpdNew->cLastDxIndex));
   cBytesToRead = 4; hb_Read(pcpdNew->iFileHandle, rgchBuffer, cBytesToRead);
   sscanf(rgchBuffer,"%04hx",&(pcpdNew->cMaxKeyLen));
   cBytesToRead = 4; hb_Read(pcpdNew->iFileHandle, rgchBuffer, cBytesToRead);
   sscanf(rgchBuffer,"%04hx",&(pcpdNew->cMaxBufNeed));

   /* memory allocation based on actual values from the compiled file */
   if ((pcpdNew->pchBuffer = (unsigned char *)
     hb_LongAlloc(pcpdNew->cMaxBuffer+100)) == NULL) {
     hb_FatalError(HB_CPD,901,szCpdFileName);
     return NULL;
   }
   if ((pcpdNew->plDxIndex = (longint *)
     hb_LongAlloc(sizeof(Longint)*
                 (pcpdNew->cLastDxIndex+4))) == NULL) {
     hb_FatalError(HB_CPD,902,szCpdFileName);
     return NULL;
   }
   if ((pcpdNew->pchDxSpace = (unsigned char *)
                     hb_LongAlloc(pcpdNew->cMaxDxSpace+100)) == NULL) {
     hb_FatalError(HB_CPD,903,szCpdFileName);
     return NULL;
   }


   if (hb_Seek(pcpdNew->iFileHandle,
             pcpdNew->cMaxData,
             SEEK_SET) < 0) {
    hb_FatalError(HB_CPD,904,szCpdFileName);
    return NULL;
   }

   cBytesToRead = pcpdNew->cMaxDxSpace;
   if (hb_Read(pcpdNew->iFileHandle,
              pcpdNew->pchDxSpace,
              cBytesToRead) != cBytesToRead) {
     hb_FatalError(HB_CPD,905,szCpdFileName);
     return NULL;
   }
   pcpdNew->pchDxSpace[pcpdNew->cMaxDxSpace++] = 0;

   iOffset = 0;
     /**/ /* fprintf(stderr,"%d ",pcpdNew->cLastDxIndex); */
   for (i = 0; i < pcpdNew->cLastDxIndex; i++) {
     /**/ /* fprintf(stderr,"%d ",i); */
     cBytesToRead = 4;
     if (hb_Read(pcpdNew->iFileHandle, rgchBuffer, cBytesToRead) != 
	 cBytesToRead) {
       hb_FatalError(HB_CPD,906,szCpdFileName);
       return NULL;
     }
     sscanf(rgchBuffer,"%04hx",&cBufLen);
     pcpdNew->plDxIndex[i] = (iOffset += cBufLen);
   }

   pcpdNew->iBufferOffset = 0;
   pcpdNew->iDxIndexAct = 0;

   cBytesToRead = 2;
   if (hb_Read(pcpdNew->iFileHandle, rgchBuffer, cBytesToRead) != 
       cBytesToRead) {
     hb_FatalError(HB_CPD,907,szCpdFileName);
     return NULL;
   }
   sscanf(rgchBuffer,"%02hx",&(pcpdNew->cRwr));

   for (i = 0; i < pcpdNew->cRwr; i++) {
     cBytesToRead = 2;
     if (hb_Read(pcpdNew->iFileHandle, rgchBuffer, cBytesToRead) != 
	 cBytesToRead) {
       hb_FatalError(HB_CPD,908,szCpdFileName);
       return NULL;
     }
     sscanf(rgchBuffer,"%02hx",&iTmp);
     pcpdNew->iRwr[i] = iTmp;
   }

   /* get language and pattern list: */
   cBytesToRead = 2;
   if (hb_Read(pcpdNew->iFileHandle, rgchBuffer, cBytesToRead) != 
       cBytesToRead) {
     /* language not present (cpd old version) */
     hb_FatalError(HB_CPD,916,szCpdFileName);
     return NULL;
   }
   else {
     /* '94 version dict file: */
     rgchBuffer[2] = '\0';
     hb_SetLangEnv(&(pcpdNew->langEnv),rgchBuffer);
     cBytesToRead = 8;
     if (hb_Read(pcpdNew->iFileHandle, rgchBuffer, cBytesToRead) != 
	 cBytesToRead) {
       hb_FatalError(HB_CPD,915,szCpdFileName);
       return NULL;
     }
#ifdef LONG_IS_64
     sscanf(rgchBuffer,"%08x",&(pcpdNew->cPatNameSpaceMax)); /* is int here */
#else
     sscanf(rgchBuffer,"%08lx",&(pcpdNew->cPatNameSpaceMax));
#endif
     pcpdNew->rgchPatNameSpace = NULL;
     cPatMaxShort = pcpdNew->cPatNameSpaceMax+1;
     if (cPatMaxShort != pcpdNew->cPatNameSpaceMax+1) {
       hb_FatalError(HB_CPD,917,szCpdFileName);
       /* but continue if Fatal does not exit */
       /* Pat funcs will not work */
     }
     else {
       /* allocate here */
       if ((pcpdNew->rgchPatNameSpace = hb_LongAlloc(cPatMaxShort)) == NULL) {
         hb_FatalError(HB_CPD,918,szCpdFileName);
         return NULL; /* will crash soon anyway */
       }
       /* and read patterns */
       if (hb_Read(pcpdNew->iFileHandle, (void *) (pcpdNew->rgchPatNameSpace),
                   pcpdNew->cPatNameSpaceMax) != pcpdNew->cPatNameSpaceMax) {
         hb_FatalError(HB_CPD,919,szCpdFileName);
         return NULL;
       }       
       /* ok, and now mark end of list by (double) tab: */
       pcpdNew->rgchPatNameSpace[pcpdNew->cPatNameSpaceMax] = HB_CPD_PATSEP;
     } /* allocate & read pattern strings */
   } /* of dict 'new' version */

   *(pcpdNew->szKeyLast) = '\0';
   pcpdNew->cKeyLastLen = 0;
   pcpdNew->cKeyCallLen = 0;
   pcpdNew->pcpdBufferTop = NULL;

#ifdef HB_CPD_DEBUG
/**/   fprintf(stderr,
   "%d %s\nMDS %d\nLDI %d\nMB %d\nRwr %d\nMD %ld\nMKL %d\nMBN %d\nL %s\n",
   pcpdNew->iFileHandle,
   pcpdNew->szFileName,
   pcpdNew->cMaxDxSpace,
   pcpdNew->cLastDxIndex,
   pcpdNew->cMaxBuffer,
   pcpdNew->cRwr,
   pcpdNew->cMaxData,
   pcpdNew->cMaxKeyLen,
   pcpdNew->cMaxBufNeed,
   pcpdNew->langEnv.szLangEnv);
#endif
   return pcpdNew;
} /* of hb_CpdOpen */

int
hb_CpdClose(hb_cpdRecType *cpdFile)
{
hb_cpdBufferRecType *pcpdBufferRec;
hb_cpdBufferRecType *pcpdBufferToFree;

   hb_Close(cpdFile->iFileHandle);
   /* free buffers: */
   hb_Free((void *) cpdFile->pchDxSpace);
   hb_Free((void *) cpdFile->plDxIndex);
   hb_Free((void *) cpdFile->pchBuffer);
   hb_Free((void *) cpdFile->rgchPatNameSpace);
   pcpdBufferRec = cpdFile->pcpdBufferTop;
   while (pcpdBufferRec != NULL) {
     hb_Free((void *) pcpdBufferRec->pchBufferR);
     pcpdBufferToFree = pcpdBufferRec;
     pcpdBufferRec = pcpdBufferRec->pcpdBufferNext;
     hb_Free((void *) pcpdBufferToFree);
   }
   /* free the file descriptor: */
   hb_Free((void *) cpdFile);
   return 0;
} /* of hb_CpdClose */

int
hb_CpdPush(hb_cpdRecType *cpdFile)
{
hb_cpdBufferRecType *pcpdBufferRec;
hb_cpdBufferRecType *pcpdBufferToFree;

  if (NULL == (pcpdBufferRec = (hb_cpdBufferRecType *) 
                   hb_LongAlloc(sizeof(hb_cpdBufferRecType)))) {
    return 1;
  }
  if (NULL == (pcpdBufferRec->pchBufferR = (Char *) 
                   hb_LongAlloc(sizeof(Char)*(cpdFile->cMaxBuffer+100)))) {
    return 2;
  }
  memcpy(pcpdBufferRec->pchBufferR,cpdFile->pchBuffer,cpdFile->cMaxBuffer+99);
  pcpdBufferRec->iBufferOffsetR = cpdFile->iBufferOffset;
  pcpdBufferRec->iDxIndexActR = cpdFile->iDxIndexAct;
  pcpdBufferRec->pchNextKeyR = cpdFile->pchNextKey;
  pcpdBufferRec->pchBufferEndR = cpdFile->pchBufferEnd;
  strcpy(pcpdBufferRec->szKeyLastR,cpdFile->szKeyLast);
  pcpdBufferRec->cKeyCallLenR = cpdFile->cKeyCallLen;
  pcpdBufferRec->cKeyLastLenR = cpdFile->cKeyLastLen;
  pcpdBufferRec->pcpdBufferNext = cpdFile->pcpdBufferTop;
  cpdFile->pcpdBufferTop = pcpdBufferRec;
  return 0;
} /* of hb_CpdPush */

int
hb_CpdPop(hb_cpdRecType *cpdFile)
{
hb_cpdBufferRecType *pcpdBufferRec;
hb_cpdBufferRecType *pcpdBufferToFree;

  pcpdBufferRec = cpdFile->pcpdBufferTop;
  cpdFile->pcpdBufferTop = pcpdBufferRec->pcpdBufferNext;

  memcpy(cpdFile->pchBuffer,pcpdBufferRec->pchBufferR,cpdFile->cMaxBuffer+99);
  cpdFile->iBufferOffset = pcpdBufferRec->iBufferOffsetR;
  cpdFile->iDxIndexAct = pcpdBufferRec->iDxIndexActR;
  cpdFile->pchNextKey = pcpdBufferRec->pchNextKeyR;
  cpdFile->pchBufferEnd = pcpdBufferRec->pchBufferEndR;
  strcpy(cpdFile->szKeyLast,pcpdBufferRec->szKeyLastR);
  cpdFile->cKeyLastLen = pcpdBufferRec->cKeyLastLenR;
  cpdFile->cKeyCallLen = pcpdBufferRec->cKeyCallLenR;
  hb_Free(pcpdBufferRec->pchBufferR);
  hb_Free(pcpdBufferRec);
  return 0;
} /* of hb_CpdPop */

int
hb_CpdFindFirst(hb_cpdRecType *pcpd, char *szKey,
                char *szcpdData, int cMaxDataLen,
                int *piPattern,
                int *fCannotFindLonger)
{
int fFound;
int iError;
Char *pch;
Char *pchData;
int iKeyIndex;

/**/ /*** fprintf(stderr,"looking for key %s\n",szKey); /**/
   pcpd->pchNextKey = NULL; /* will stay if not found */
   if ((iError = hb_CpdGetDataBlock(pcpd,szKey,&fFound)) != 0) {
     hb_FatalError(HB_CPD,910,pcpd->szFileName);
     *szcpdData = '\0';
     *fCannotFindLonger = 1;
     return(iError);
   }
   if (!fFound) {
     *fCannotFindLonger = 1;
     goto not_found;
   }
   /* found in index, buffer read in... (in hb_CpdGetDataBlock) */
   pchData = pcpd->pchBuffer + 1; /* first byte always 0 */
   pch = szKey;
   /* pch ends by zero, pchData by strip count (always < HB_CPD_FIRST_CHAR) */
   while (*pch) {
#ifdef HB_CPD_DEBUG
/**/ fprintf(stderr,
       "hb_cpd: comparing key(%x): '%c' (%d) -- cpd data(%x): '%c' (%d), nxt '%c' (%d)\n",
       pch-(Char *)szKey,*pch,*pch,
       pchData-(pcpd->pchBuffer),*pchData,*pchData,*(pchData+1),*(pchData+1));
#endif
     if (*pch < *pchData) {
       *fCannotFindLonger = 1;
       goto not_found; /* not found, data greater */
     }
     if (*pch > *pchData) { /* data smaller, find next data */
         /* (goes here also if *pchData ends, ie. < 32)!! */
       iKeyIndex = pch - (Char *) szKey;
       do {
         while (*pchData++ >= HB_CPD_FIRST_CHAR)
#ifdef HB_CPD_DEBUG
/**/ fprintf(stderr,
       "hb_cpd: ignoring tail cpd data(%x): '%c' (%d)\n",
       pchData-1-(pcpd->pchBuffer),*(pchData-1),*(pchData-1));
#endif
         ; /* skip rest of tail of key */
         /* skip strip cnt + pattern */
         while (*(++pchData) >= HB_CPD_FIRST_CHAR)
#ifdef HB_CPD_DEBUG
/**/ fprintf(stderr,
       "hb_cpd: ignoring replace cpd data(%x): '%c' (%d)\n",
       pchData-(pcpd->pchBuffer),*pchData,*pchData);
#endif
         ; /* skip tail of replace */
#ifdef HB_CPD_DEBUG
/**/ fprintf(stderr,
       "hb_cpd: repeat len cpd data(%x): '%c' (%d) >(loop) %d\n",
       pchData-(pcpd->pchBuffer),*pchData,*pchData,iKeyIndex);
#endif
         if (*pchData == HB_CPD_FIRST_CHAR - 2) {
           pchData++; /* points to real same key length */
         }
#ifdef HB_CPD_DEBUG
/**/ fprintf(stderr,
       "hb_cpd: true repeat len cpd data(%x): '%c' (%d) >(loop) %d\n",
       pchData-(pcpd->pchBuffer),*pchData,*pchData,iKeyIndex);
#endif
       } while (*pchData++ > iKeyIndex); /* no need to check data with */
         /* longer same prefix - cannot equal, too */
       pch = szKey + *(pchData-1);/* lower beginning of compare index */
     } /* of shift to next candidate data */
     else { /* szKey eq data - do next char */
       pch++;
       pchData++;
     } /* of shift to next char */
   } /* of while *pch */
   /* here: *pch zero */
   if (*pchData >= HB_CPD_FIRST_CHAR) {
     /* data longer (greater) ... not found */
     *fCannotFindLonger = 0; /* but longer data does exist */
     goto not_found;
   }
   /* *pchData ends, too --> same lgt, found! */
   *fCannotFindLonger = 0; /* longer data *possibly* exists */

   pcpd->cKeyCallLen = strlen(szKey);
   pcpd->cKeyLastLen = pcpd->cKeyCallLen;
   if (pcpd->cKeyLastLen < HB_STRINGMAX) {
     strcpy(pcpd->szKeyLast,szKey);
   }
   else {
     hb_FatalError(HB_CPD,912,pcpd->szFileName);
     return(9);
   }

   /**/ /****/ /* fprintf(stderr,"cpd find first DATA FOUND!!!! \n"); /**/
   if (hb_CpdGetData(pcpd,pchData,szcpdData,cMaxDataLen,piPattern) > 0) {
     hb_FatalError(HB_CPD,913,pcpd->szFileName);
     return(10);
   }

   hb_CpdInvCvs(szcpdData,szcpdData);
   return 0;

not_found:
   *szcpdData = '\0';
   return 0;

} /* of hb_CpdFindFirst */

FUNDEF8(int
hb_CpdFindFirstLonger,
                hb_cpdRecType *, pcpd, char *, szKey,
                char *, szcpdKey, int, cMaxKeyLen,
                char *, szcpdData, int, cMaxDataLen,
                int *, piPattern,
                int *, fCannotFindLonger)
{
     /* finds first key which is equal or longer to szKey */
     /* same as findfirstlonger, but returns also the key (may be
          different from szKey (i.e. longer) */
int fFound;
int iError;
Char *pch;
Char *pchData;
int iKeyIndex;
int cKeyLen;

   pcpd->pchNextKey = NULL; /* will stay if not found */
   if ((iError = hb_CpdGetDataBlock(pcpd,szKey,&fFound)) != 0) {
     hb_FatalError(HB_CPD,915,pcpd->szFileName);
     *szcpdData = '\0';
     *fCannotFindLonger = 1;
     return(iError);
   }
   if (!fFound) {
     *fCannotFindLonger = 1;
     goto not_found;
   }
   /* found in index, buffer read in... (in hb_CpdGetDataBlock) */
   pchData = pcpd->pchBuffer + 1; /* first byte always 0 */
   pch = szKey;
   /* pch ends by zero, pchData by strip count (always < HB_CPD_FIRST_CHAR) */
   while (*pch) {
#ifdef HB_CPD_DEBUG
/**/ fprintf(stderr,
       "hb_cpd: comparing key(%x): '%c' (%d) -- cpd data(%x): '%c' (%d)\n",
       pch-(Char *)szKey,*pch,*pch,
       pchData-(pcpd->pchBuffer),*pchData,*pchData);
#endif

     if (*pch < *pchData) {
       *fCannotFindLonger = 1;
       goto not_found; /* not found, data greater */
            /* this means really not found -- not even data with longer key
               exist! */
     }
     if (*pch > *pchData) { /* data smaller, find next data */
         /* (goes here also if *pchData ends, ie. < 32)!! */
       iKeyIndex = pch - (Char *) szKey;
       do {
         while (*pchData++ >= HB_CPD_FIRST_CHAR)
#ifdef HB_CPD_DEBUG
/**/ fprintf(stderr,
       "hb_cpd: ignoring tail cpd data(%x): '%c' (%d)\n",
       pchData-1-(pcpd->pchBuffer),*(pchData-1),*(pchData-1));
#endif
         ; /* skip rest of tail of key */
         /* skip strip cnt + pattern */
         while (*(++pchData) >= HB_CPD_FIRST_CHAR)
#ifdef HB_CPD_DEBUG
/**/ fprintf(stderr,
       "hb_cpd: ignoring replace cpd data(%x): '%c' (%d)\n",
       pchData-(pcpd->pchBuffer),*pchData,*pchData);
#endif
         ; /* skip tail of replace */
#ifdef HB_CPD_DEBUG
/**/ fprintf(stderr,
       "hb_cpd: repeat len cpd data(%x): '%c' (%d) >(loop) %d\n",
       pchData-(pcpd->pchBuffer),*pchData,*pchData,iKeyIndex);
#endif
       } while (*pchData++ > iKeyIndex); /* no need to check data with */
         /* longer same prefix - cannot equal, too */
       pch = szKey + *(pchData-1);/* lower beginning of compare index */
     } /* of shift to next candidate data */
     else { /* szKey eq data - do next char */
       pch++;
       pchData++;
     } /* of shift to next char */
   } /* of while *pch */
   /* here: *pch zero */
   cKeyLen = strlen(szKey) + 1;
   if (cKeyLen < cMaxKeyLen) {
     strcpy(szcpdKey,szKey);
   }
   else {
     hb_FatalError(HB_CPD,914,pcpd->szFileName);
     return(12);
   }
   if (*pchData >= HB_CPD_FIRST_CHAR) {
     /* data longer (greater) ... not found */
     *fCannotFindLonger = 0; /* but longer data does exist */
     /* goto not_found; */ /* THIS IS DIFFERENT FROM USUAL FINDFIRST */
     cKeyLen = strlen(szKey);
     while (*pchData >= HB_CPD_FIRST_CHAR) {
       if (cKeyLen + 1 < cMaxKeyLen) {
         szcpdKey[cKeyLen++] = *pchData;
        }
       else {
         hb_FatalError(HB_CPD,918,pcpd->szFileName);
         return(13);
       }
       pchData++;
     }
     szcpdKey[cKeyLen] = HB_EOS;
   }
   /* *pchData ends, too --> same lgt, found! */
   *fCannotFindLonger = 0; /* longer data *possibly* exists */

   pcpd->cKeyCallLen = strlen(szKey);
   pcpd->cKeyLastLen = strlen(szcpdKey);
   if (pcpd->cKeyLastLen < HB_STRINGMAX) {
     strcpy(pcpd->szKeyLast,szcpdKey); /* remember *full* key */
   }
   else {
     hb_FatalError(HB_CPD,916,pcpd->szFileName);
     return(9);
   }

   if (hb_CpdGetData(pcpd,pchData,szcpdData,cMaxDataLen,piPattern) > 0) {
     hb_FatalError(HB_CPD,917,pcpd->szFileName);
     return(10);
   }

   hb_CpdInvCvs(szcpdData,szcpdData);
   return 0;

not_found:
   *szcpdData = '\0';
   return 0;

} /* of hb_CpdFindFirstLonger */

int
hb_CpdGetNext(hb_cpdRecType *pcpd,
                char *szcpdData, int cMaxDataLen,
                int *piPattern,
                int *fCannotFindLonger)
{
Char *pchData;

   pchData = pcpd->pchNextKey;
   *szcpdData = '\0';
   *piPattern = 0;
   *fCannotFindLonger = 1;
   pcpd->pchNextKey = NULL;

   /**/ /*** fprintf(stderr,
            "looking for next data, key len %d\n",pcpd->cKeyLastLen); /**/
   if (pchData != NULL) {
   /**/ /*** fprintf(stderr,
                 "pchData: %d, next %d\n",*pchData,*(pchData+1)); /**/
     if ((*pchData == pcpd->cKeyLastLen) ||
         ((*pchData == HB_CPD_FIRST_CHAR - 2) && 
          (*(pchData+1) == pcpd->cKeyLastLen))) {

       /* same length */
       if (*pchData == HB_CPD_FIRST_CHAR - 2) {
         pchData++; /* skip over real length of key */
       }
       pchData++; /* to-add-length */
       if (*pchData < HB_CPD_FIRST_CHAR) {
              /* nothing to add */
              /* --> it is the same key! */
         *fCannotFindLonger = 0; /* longer data *possibly* exists */
  
         if (hb_CpdGetData(pcpd,pchData,szcpdData,cMaxDataLen,piPattern) > 0) {
           hb_FatalError(HB_CPD,914,pcpd->szFileName);
           return(10);
         }
         hb_CpdInvCvs(szcpdData,szcpdData);
       }
     }
   }

   return 0;
} /* of hb_CpdGetNext */

FUNDEF7(int
hb_CpdGetNextLonger, 
                hb_cpdRecType *, pcpd,
                char *, szcpdKey, int, cMaxKeyLen,
                char *, szcpdData, int, cMaxDataLen,
                int *, piPattern,
                int *, fCannotFindLonger)
{
int fFound;
int iError;
Char *pch;
Char *pchData;
Char *pchRealData;
int iKeyIndex;
int iResult;
int cKeyLen;
int iRetainCount;
char szCallKey[HB_STRINGMAX];
Char *pchCallKey;

   pchData = pcpd->pchNextKey; /* start here */
   pcpd->pchNextKey = NULL; /* will stay if not found */
   *fCannotFindLonger = 1;   /* suppose last possible data */
   *szcpdData = '\0'; /* suppose not found at all */
   *piPattern = 0; /* just to behave cleanly */

   while (1) {

     while (pcpd->pchBufferEnd - pchData > 0) {
       iRetainCount = *pchData++;
   
       /**/ /* fprintf(stderr,"retain count for next data item: %d\n",
                iRetainCount); /**/
       /**/ /* fprintf(stderr,"from end of buffer: %d\n",
                    pcpd->pchBufferEnd - pchData); /**/

       if (iRetainCount >= pcpd->cKeyCallLen) { /* ok, key still longer */
                  /**/ /* NB: may not work for "CUT ALL" !
                          but currently it is used only for
                          data (HB_CPD_FIRST_CHAR - 2), not for
                          keys */
         pcpd->szKeyLast[iRetainCount] = HB_EOS;
         iKeyIndex = iRetainCount; /* add rest of key to szKeyLast */
         while (*pchData >= HB_CPD_FIRST_CHAR) {
           if (iKeyIndex + 1 < HB_STRINGMAX)
           pcpd->szKeyLast[iKeyIndex++] = *pchData++;
         }
         pcpd->szKeyLast[iKeyIndex] = HB_EOS;
         pcpd->cKeyLastLen = iKeyIndex;
         if (pcpd->cKeyLastLen < cMaxKeyLen) {
           strcpy(szcpdKey,pcpd->szKeyLast); /* NB key is always coded */
         } 
         else { /* cannot output new key to caller's space */
           hb_FatalError(HB_CPD,922,pcpd->szFileName);
           return(14);
         }
       }
       else { /* retain count smaller, but must check if really
              key has different prefix than caller's key! */
              /* this happens always when crossing block boundaries */
         strncpy(szCallKey,pcpd->szKeyLast,pcpd->cKeyCallLen);
         szCallKey[pcpd->cKeyCallLen] = HB_EOS; /* now we have reconstructed
                               the original caller's key */
         iKeyIndex = iRetainCount; /* add rest of key to szKeyLast */
         while (*pchData >= HB_CPD_FIRST_CHAR) {
           if (iKeyIndex + 1 < HB_STRINGMAX)
           pcpd->szKeyLast[iKeyIndex++] = *pchData++;
         }
         pcpd->szKeyLast[iKeyIndex] = HB_EOS;
         pcpd->cKeyLastLen = iKeyIndex;
         /* now new key fully in pcpd->szKeyLast */
         /* so we can test if caller's key is a prefix: */
         iKeyIndex = 0;
         pchCallKey = (Char *) szCallKey;
         while (*pchCallKey != HB_EOS) {
           if (*pchCallKey != pcpd->szKeyLast[iKeyIndex]) {
             break; /* difference before caller's key length reached */
           }
           pchCallKey++;
           iKeyIndex++;
         }
         if (*pchCallKey == HB_EOS) { /* test ok, now behave as if
                              retain count longer than caller's key! */
           if (pcpd->cKeyLastLen < cMaxKeyLen) {
             strcpy(szcpdKey,pcpd->szKeyLast); /* NB key is always coded */
           } 
           else { /* cannot output new key to caller's space */
             hb_FatalError(HB_CPD,923,pcpd->szFileName);
             return(15);
           }
           /* and go get data... */
         }
         else { 
           return 0; /* no error, but next data not found (signalled to caller
                      by empty szcpdData) */
         }
       }
       if (hb_CpdGetData(pcpd,pchData,szcpdData,cMaxDataLen,piPattern) > 0) {
         hb_FatalError(HB_CPD,921,pcpd->szFileName);
         return(10);
       }
       hb_CpdInvCvs(szcpdData,szcpdData);
       return 0; /* found, return ok indication */
     } /* end of while till end of buffer */

     if (pcpd->iDxIndexAct < pcpd->cLastDxIndex-2) {
       hb_CpdReadDataBlock(pcpd, pcpd->iDxIndexAct + 1);
       pchData = pcpd->pchBuffer; /* first byte always 0 (= retain count) */
     }
     else {
       break; /* last block read in already, exit */
     }
   } /* of while more blocks in data file to search through */
   return 0;
} /* hb_CpdGetNextLonger */

int
hb_CpdCvs(Char *szFrom, Char *szTo)
{
   if (hb_fCpdInvTabFilled == 0) hb_CpdFillInvTab();
   while (*szFrom) {
     *szTo++ = hb_cpdTab[*szFrom++];
   }
   *szTo = 0;
   return 0;
} /* of hb_CpdCvs */


int
hb_CpdInvCvs(Char *szFrom, Char *szTo)
{
   if (hb_fCpdInvTabFilled == 0) hb_CpdFillInvTab();
   while (*szFrom) {
     *szTo++ = hb_cpdInvTab[*szFrom++];
   }
   *szTo = 0;
   return 0;
} /* of hb_CpdInvCvs */




/* private functions def: */
int
hb_CpdGetData(hb_cpdRecType *pcpd,
              Char *pchData,
              char *szcpdData, int cMaxDataLen,
              int *piPattern)
{

Char *pchRealData;
int iResult;
int cKeyLen;
int cSameLen;

   if (*pchData == HB_CPD_FIRST_CHAR - 1) {
      /* data space contains pointer to iRwr */
     pchRealData = pcpd->iRwr + pchData[1];
#ifdef HB_CPD_DEBUG
     fprintf(stderr,"going to rwr (flag %d), offset [%d]\n",
	     *pchData,pchData[1]);
#endif
     pcpd->pchNextKey = pchData + 2; /* next data follows iRwr pointer */
        /* 'same chars' len ptr */
   }
   else
     pchRealData = pchData;
   /* in pchRealData now: address of data string, be it */
   /* directly in data space, or in common iRwr space */

   cKeyLen = /* strlen(pcpd->szKeyLast); */
             pcpd->cKeyLastLen;
   /* fill in part of key which is the same as data: */
   if (*pchRealData == HB_CPD_FIRST_CHAR - 2)
     cSameLen = 0;
   else
     cSameLen = cKeyLen - (*pchRealData);
#ifdef HB_CPD_DEBUG
     fprintf(stderr,"*pchReal: '%c' (%d), cSameLen computed: %d, cKeyLen %d\n",
	    *pchRealData,*pchRealData,cSameLen,cKeyLen);
#endif
   for (iResult = 0; iResult < cSameLen; iResult++) {
     if (iResult < cMaxDataLen-1) {
       szcpdData[iResult] = pcpd->szKeyLast[iResult];
     }
   }
   *piPattern = pchRealData[1];
   pchRealData += 2;
   while (*pchRealData >= HB_CPD_FIRST_CHAR) {
     if (iResult < cMaxDataLen-1) {
#ifdef HB_CPD_DEBUG
     fprintf(stderr,"filling data: iResult %d, char '%c' (%d), nx '%c,%c' (%d,%d)\n",
	    iResult,*pchRealData,*pchRealData,
	     *(pchRealData+1),*(pchRealData+2),
            *(pchRealData+1),*(pchRealData+2));
#endif
       szcpdData[iResult++] = *pchRealData++;
     }
   }
   if (pcpd->pchNextKey == NULL) {
     /* might have been already set (if Rwr used -> pchRealData */
     /* does not point to data buffer */
     pcpd->pchNextKey = pchRealData; /* 'same chars' count ptr */
   }
#ifdef HB_CPD_DEBUG
     fprintf(stderr,"nextkey data dic start: char '%c' (%d)\n",
	    *(pcpd->pchNextKey),*(pcpd->pchNextKey));
#endif
   szcpdData[iResult] = '\0';
   return 0;
} /* of hb_CpdGetData */


int
hb_CpdReadDataBlock(hb_cpdRecType *pcpd, int iXtable)
{
longint iReadOffset;
Shortint cReadBytes;
#ifdef HB_CPD_DEBUG
/**/ 
Shortint i;
#endif        

   iReadOffset = pcpd->plDxIndex[iXtable];
   if (pcpd->iFileHandle == -1) {
     /* no need to read endings, they */
     /* are in memory all the time */
     pcpd->iBufferOffset = iReadOffset;
     pcpd->iDxIndexAct = iXtable;
     /* pcpd->pchBufferEnd not set !! */
   }
   else {
     if (iReadOffset != pcpd->iBufferOffset ||
         iXtable != pcpd->iDxIndexAct) {
       cReadBytes = pcpd->plDxIndex[iXtable+1] - iReadOffset;
#ifdef HB_CPD_DEBUG
/**/ fprintf(stderr,
       "hb_cpd: will read %d bytes\n",
       cReadBytes);
#endif
       if (hb_Seek(pcpd->iFileHandle,iReadOffset,SEEK_SET) < 0) {
         hb_FatalError(HB_CPD,909,pcpd->szFileName);
         return 1;
       }
       if (hb_Read(pcpd->iFileHandle,
                   pcpd->pchBuffer,cReadBytes) != cReadBytes) {
         hb_FatalError(HB_CPD,910,pcpd->szFileName);
         return 2;
       }
       pcpd->iDxIndexAct = iXtable;
       pcpd->iBufferOffset = iReadOffset;
       pcpd->pchBuffer[cReadBytes] = 0; /* curr buffer closing zero */
       pcpd->pchBufferEnd = pcpd->pchBuffer + cReadBytes;
#ifdef HB_CPD_DEBUG
     /**/ 
     for (i = 0; i < cReadBytes; i++) {
       fprintf(stderr,
       "hb_cpd: data read: (%x): '%c' (%d)\n",
       i,pcpd->pchBuffer[i],pcpd->pchBuffer[i]);
     }
#endif
       /* ptr to the zero byte just inserted */
     }
   }  /* of dict reading */

   return 0;
}


int
hb_CpdGetDataBlock(hb_cpdRecType *pcpd, char *szKey, int *pfFound)
{
Shortint iXtable;
Shortint iEndtable;
Char *pch;
longint iReadOffset;
Shortint cReadBytes;
Char *pchIndexString;

   pchIndexString = pcpd->pchDxSpace;
   iEndtable = pcpd->cLastDxIndex - 1;
   for (iXtable = 0; iXtable < iEndtable; iXtable++) {
     pch = szKey;
     while (1) {
       if (*pch < *pchIndexString) {
         /* found here */
         goto found;
       }
       else if (*pch > *pchIndexString) {
         while (*pchIndexString++); /* next index string */
         break; /* here not found */
       }
       else /* chars equal */ if (*pch == 0) {
         /* both strings equal, it's here, too */
         goto found;
       }
       /* strings continue, try next char */
       pch++;
       pchIndexString++;
     } /* of while */
   } /* of for */
   /* not found at all */
   *pfFound = 0;
   return(0); /* but no fatal error */

found:
#ifdef HB_CPD_DEBUG
/**/ fprintf(stderr,"hb_cpd: found in table cell: iXtable == %d;\n",iXtable);
#endif
   if (hb_CpdReadDataBlock(pcpd,iXtable) > 0) {
     hb_FatalError(HB_CPD,911,pcpd->szFileName);
     return 1;
   }
   *pfFound = 1;
   return 0;

} /* of hb_CpdGetDataBlock */


int
hb_CpdFillInvTab(void)
{
int i;
  if (hh_tab != NULL) {
    hb_cpdTab = hh_tab;
  }
  else {
    hb_cpdTab = hb_cpd895;
  }
  for (i = 0; i < 256; i++) hb_cpdInvTab[i] = 'z';
  for (i = 0; i < 256; i++) hb_cpdInvTab[hb_cpdTab[i]] = i;
  hb_fCpdInvTabFilled = 1;
  return 0;
} /* of hb_CpdFillInvTab */

FUNDEF4(int
hb_CpdPatternOfIndex, 
hb_cpdRecType *, pcpd,
Char *, szPattern,
Shortint, cMaxLen,
Shortint, iIndex)
{
Char *pchPatStart;
Char *pch;
Shortint iAct;

  pchPatStart = pcpd->rgchPatNameSpace;
  iAct = 0;
  while (*pchPatStart != HB_CPD_PATSEP) {
    if (iAct == iIndex) break;
    pch = pchPatStart + 1;
    while (*pch++ != HB_CPD_PATSEP);
    pchPatStart = pch;
    iAct++;
  } /* of search loop */
  *szPattern = HB_EOS;
  if (iAct != iIndex || *pchPatStart == HB_CPD_PATSEP) { /* not found */
    return 1;
  }
  else {
    /* return the pattern string */
    pch = szPattern;
    while (cMaxLen-- > 0) {
      *pch++ = *pchPatStart++;
      if (*pchPatStart == HB_CPD_PATSEP) break;
    }
    *pch = HB_EOS;
    return 0;
  }
} /* of hb_CpdPatternOfIndex */

FUNDEF3(int
hb_CpdIndexOfPattern,
hb_cpdRecType *, pcpd,
Shortint *, piIndex,
Char *,szPattern)
{
  /**/
  return 0;
} /* of hb_CpdIndexOfPattern */


/*
   string hashing module 
 (C) Jan Hajic 11.03.97
   */

#include <stdio.h>
#include <string.h>

#if defined(unix)
#include <unistd.h>
#include <sys/stat.h>
#else
#include <io.h>
#include <sys\stat.h>
#include <share.h>
#include <stdlib.h>
#include <process.h>
#endif

#include "hb_proto.h"
#include "hb_base.h"
#include "hb_hash.h"

/**/ /*
#include <mem.h>
/**/
/**/ /*
#include <alloc.h>
/**/

/* private functions */
longint
FUNDEF1(hbp_HashCode, char *, sz)
{
longint lHashCode;
char *pch;
longint lHashChar;
int iShift;
int iIncr;

  lHashCode = 0l;
  pch = sz;
  iShift = 22;
  iIncr = 7;
  while (*pch != HB_EOS) {
    while (iShift >= 24) {
      iShift -= 24;
    } /* of while decr. iShift */
    lHashChar = (unsigned char) *pch;
    lHashChar <<= iShift;
    lHashCode += lHashChar;
    pch++;
    iShift += iIncr + (((unsigned char) *pch) & 0x1F);
  } /* of while pch not NULL */
  if (lHashCode < 0) lHashCode++; /* hajic 01/02/04
                                     to ensure that lHashCode is not
                                     equal to MIN_INT32; if it were, it
                                     crashes when -lHashCode computed */
  return lHashCode;
} /* of hbp_HashCode */

/* end of private functions */

hb_hashRecType * 
FUNDEF1(hb_HashOpen, char *, szHashFileName)
{
return NULL;
}

int
FUNDEF1(hb_HashClose, hb_hashRecType *,hashFile)
{
return 0;
}

FUNDEF6(int
hb_HashFirstData, hb_hashRecType *, phash,
                char *, szhashKey, int, cMaxKeyLen,
                char *, szhashData, int, cMaxDataLen,
                longint *, plPattern)
{
longint *plIndex;
  phash->lCurrHashIndex = 0l; /* reset current seq. position to 0 */
  plIndex = HB_plHASHTAB(phash,phash->lCurrHashIndex);
  phash->lNextStrOut = *plIndex; /* 0 if empty, otherwise ptr to Str */
  return hb_HashNextData(phash,szhashKey,cMaxKeyLen,szhashData,cMaxDataLen,
                         plPattern);
} /* of hb_HashFirstData */

FUNDEF6(int
hb_HashNextData, hb_hashRecType *, phash,
                char *, szhashKey, int, cMaxKeyLen,
                char *, szhashData, int, cMaxDataLen,
                longint *, plPattern)
{
    /* returns -1 on error or end of data, 0 if all ok, 1 if key trunc'd,
                        2 if data trunc'd, 3 if both key and data truncated */
longint lHashIndex;
longint *plIndex;
char *pszStrNext; /* Str data area ptr */
char *pszStrKey; /* Str data area ptr */
char *pszStrData; /* Str data area ptr */
longint lNextKeyData; /* ptr to Str; Next + Key + Data */
longint lPattern;
int iRC;

  lHashIndex = phash->lCurrHashIndex;
  if (lHashIndex < 0l || lHashIndex >= phash->lSize) {
    /* error or eof: signal eof */
    phash->lCurrHashIndex = -1l;
    phash->lNextStrOut = -1l;
    return -1;
  }
  lNextKeyData = phash->lNextStrOut;
  while (lNextKeyData == 0l) { /* no more data for current hash index */
    lHashIndex++;
    if (lHashIndex >= phash->lSize) {
      phash->lCurrHashIndex = -1l;
      phash->lNextStrOut = -1l;
      return -1; /* end of data */
    }
    plIndex = HB_plHASHTAB(phash,lHashIndex);
    lNextKeyData = *plIndex; /* 0 if empty, otherwise ptr to Str */
  }
  /* something to return... */
  /* but first, compute next key/data ptr and save it: */
  pszStrNext = HB_pszHASHSTR(phash,lNextKeyData); /* ptr to key/data rec */
  HB_HASHS2L(pszStrNext,lNextKeyData);
  /* ok, now ptr to next data with same hash index stored in lNextKeyData */

  pszStrKey = pszStrNext + 8; /* key is after next and pat longints */
  pszStrData = pszStrKey + strlen(pszStrKey) + 1;
  HB_HASHS2L(pszStrNext+4,lPattern);
  iRC = 0;
  if (strlen(pszStrKey) >= cMaxKeyLen) {
    iRC += 1; /* key truncated */
    strncpy(szhashKey,pszStrKey,cMaxKeyLen-1);
    szhashKey[cMaxKeyLen-1] = HB_EOS;
  }
  else {
    strcpy(szhashKey,pszStrKey);
  }
  if (strlen(pszStrData) >= cMaxDataLen) {
    iRC += 2; /* data truncated */
    strncpy(szhashData,pszStrData,cMaxDataLen-1);
    szhashKey[cMaxDataLen-1] = HB_EOS;
  }
  else {
    strcpy(szhashData,pszStrData);
  }
  *plPattern = lPattern;
  /* now save current position: */
  phash->lCurrHashIndex = lHashIndex;
  phash->lNextStrOut = lNextKeyData;
  return iRC;
} /* of hb_HashNextData */

FUNDEF4(int
hb_HashFirstDataPtr, hb_hashRecType *, phash,
	char **, ppszhashKey,
	char **, ppszhashData,
	longint *, plPattern)
{
longint *plIndex;
  phash->lCurrHashIndex = 0l; /* reset current seq. position to 0 */
  plIndex = HB_plHASHTAB(phash,phash->lCurrHashIndex);
  phash->lNextStrOut = *plIndex; /* 0 if empty, otherwise ptr to Str */
  return hb_HashNextDataPtr(phash,ppszhashKey,ppszhashData,
                         plPattern);
} /* of hb_HashFirstDataPtr */

FUNDEF4(int
hb_HashNextDataPtr, hb_hashRecType *, phash,
	char **, ppszhashKey,
	char **, ppszhashData,
	longint *, plPattern)
{
  /* returns -1 on error or end of data, 0 if all ok */
longint lHashIndex;
longint *plIndex;
char *pszStrNext; /* Str data area ptr */
char *pszStrKey; /* Str data area ptr */
longint lNextKeyData; /* ptr to Str; Next + Key + Data */
longint lPattern;
int iRC;

  lHashIndex = phash->lCurrHashIndex;
  if (lHashIndex < 0l || lHashIndex >= phash->lSize) {
    /* error or eof: signal eof */
    phash->lCurrHashIndex = -1l;
    phash->lNextStrOut = -1l;
    return -1;
  }
  lNextKeyData = phash->lNextStrOut;
  /****/ /* fprintf(stderr,"lHashIndex ok %ld, phash->lSize %ld, lNextKeyData %ld.\n",
		 lHashIndex,phash->lSize,lNextKeyData); /**/
  while (lNextKeyData == 0l) { /* no more data for current hash index */
    lHashIndex++;
    if (lHashIndex >= phash->lSize) {
      phash->lCurrHashIndex = -1l;
      phash->lNextStrOut = -1l;
      return -1; /* end of data */
    }
    plIndex = HB_plHASHTAB(phash,lHashIndex);
    lNextKeyData = *plIndex; /* 0 if empty, otherwise ptr to Str */
  }
  /****/ /* fprintf(stderr,"something to return....\n"); /**/
  /* something to return... */
  /* but first, compute next key/data ptr and save it: */
  pszStrNext = HB_pszHASHSTR(phash,lNextKeyData); /* ptr to key/data rec */
  HB_HASHS2L(pszStrNext,lNextKeyData);
  /* ok, now ptr to next data with same hash index stored in lNextKeyData */

  phash->pchPatAtFound = pszStrNext + 4; /* save ptr to pat longint */

  /* fill in return values: */
  *ppszhashKey = pszStrKey = pszStrNext + 8; 
                          /* key is after next and pat longints */
  *ppszhashData = pszStrKey + strlen(pszStrKey) + 1;
  HB_HASHS2L(pszStrNext+4,lPattern);
  *plPattern = lPattern;

  iRC = 0;
  /* now save current position: */
  phash->lCurrHashIndex = lHashIndex;
  phash->lNextStrOut = lNextKeyData;
  return iRC;
} /* of hb_HashNextDataPtr */

int
FUNDEF6(hb_HashFindFirst, hb_hashRecType *,phash, char *,szKey,
                 char *,szhashData, int, cMaxDataLen,
                 longint *,plPattern,
                 int *,pfCannotFindLonger)
{
  /* returns < 0 if error, 0 if found, 1 if not found */
longint lHashCode;
longint lHashIndex;
longint *plIndex;
longint lKeyLen; /* total no. of chars in Key */
char *pszStrNext; /* Str data area ptr */
char *pszStrKey; /* Str data area ptr */
char *pszStrData; /* Str data area ptr */
longint lNextKeyData; /* ptr to Str; Next + Key + Data */

  /**//* fprintf(stdout,"to add: <%s> <%s>\n",pszAdd,pszDataAdd); */
  lHashCode = hbp_HashCode(szKey);
  if (lHashCode < 0) lHashCode = -lHashCode;
  lHashIndex = lHashCode % (phash->lSize - 1);
  /**/ /* fprintf(stderr,"key hash code: %ld, index %ld\n",
    lHashCode,lHashIndex);                                   /**/
  plIndex = HB_plHASHTAB(phash,lHashIndex);
  lNextKeyData = *plIndex; /* 0 if empty, otherwise ptr to Str */
  phash->lNextStrOut = lNextKeyData;

  if (phash->szLastKey != NULL) {
    hb_Free(phash->szLastKey);
  }
  if ((phash->szLastKey = (char *) hb_LongAlloc((strlen(szKey)+1)*sizeof(char)))
      == NULL) {
    hb_FatalError(HB_HASH,712,szKey);
    return -1;
  }
  strcpy(phash->szLastKey,szKey);

  return (hb_HashGetNext(phash,szhashData,cMaxDataLen,
                          plPattern,pfCannotFindLonger));
} /* of hb_HashFindFirst */

int
FUNDEF5(hb_HashGetNext, hb_hashRecType *,phash,
                char *,szhashData, int, cMaxDataLen,
                longint *,plPattern,
                int *,pfCannotFindLonger)
{
char *pszStrNext; /* Str data area ptr */
char *pszStrKey; /* Str data area ptr */
char *pszStrData; /* Str data area ptr */
longint lNextKeyData; /* ptr to Str; Next + Key + Data */
char *pszKey;

  lNextKeyData = phash->lNextStrOut;
  pszKey = phash->szLastKey;
  while (lNextKeyData != 0l) {
    pszStrNext = HB_pszHASHSTR(phash,lNextKeyData); /* ptr to next 'record' */
    pszStrKey = pszStrNext + 8; /* key is after next and pat longints */
    /**/ /* fprintf(stderr,"cmp: <%s> <%s>\n",pszKey,pszStrKey); /**/
    if (!strcmp(pszKey,pszStrKey)) {
      /* same, return data */
      pszStrData = pszStrKey + strlen(pszStrKey) + 1;
      if (strlen(pszStrData) + 1 > cMaxDataLen) {
        strncpy(szhashData,pszStrData,cMaxDataLen-1);
        szhashData[cMaxDataLen-1] = HB_EOS;
      }
      else {
        strcpy(szhashData,pszStrData);
      }
      /* longint Pat offset == 4, just after next ptr: */
      phash->pchPatAtFound = pszStrNext+4; /* save ptr to pat longint */
      HB_HASHS2L(pszStrNext+4,
                   *plPattern);
      *pfCannotFindLonger = 1; /**/
      HB_HASHS2L(pszStrNext,lNextKeyData);
      phash->lNextStrOut = lNextKeyData;
      return 0; /* found */
    }
    HB_HASHS2L(pszStrNext,lNextKeyData);

    /**/ /* fprintf(stderr,"lNextKeyData: %ld\n",lNextKeyData); /**/    

  } /* of while lNextKeyData */
  
  return 1; /* not found any more */
} /* of hb_HashGetNext */

int
FUNDEF6(hb_HashFindFirstPtr, hb_hashRecType *, phash, char *, szKey,
                 char **, ppszhashKey, char **, ppszhashData,
                 longint *, plPattern,
                 int *, pfCannotFindLonger)
{
  /* returns < 0 if error, 0 if found, 1 if not found */
longint lHashCode;
longint lHashIndex;
longint *plIndex;
longint lKeyLen; /* total no. of chars in Key */
char *pszStrNext; /* Str data area ptr */
char *pszStrKey; /* Str data area ptr */
char *pszStrData; /* Str data area ptr */
longint lNextKeyData; /* ptr to Str; Next + Key + Data */

  /**//* fprintf(stdout,"to add: <%s> <%s>\n",pszAdd,pszDataAdd); */
  lHashCode = hbp_HashCode(szKey);
  if (lHashCode < 0) lHashCode = -lHashCode;
  lHashIndex = lHashCode % (phash->lSize - 1);
  /**/ /* fprintf(stderr,"key hash code: %ld, index %ld\n",
    lHashCode,lHashIndex);                                   /**/
  plIndex = HB_plHASHTAB(phash,lHashIndex);
  lNextKeyData = *plIndex; /* 0 if empty, otherwise ptr to Str */
  phash->lNextStrOut = lNextKeyData;

  if (phash->szLastKey != NULL) {
    hb_Free(phash->szLastKey);
  }
  if ((phash->szLastKey = (char *) hb_LongAlloc((strlen(szKey)+1)*sizeof(char)))
      == NULL) {
    hb_FatalError(HB_HASH,712,szKey);
    return -1;
  }
  strcpy(phash->szLastKey,szKey);

  return (hb_HashGetNextPtr(phash,ppszhashKey,ppszhashData,
                          plPattern,pfCannotFindLonger));
} /* of hb_HashFindFirstPtr */

int
FUNDEF5(hb_HashGetNextPtr, hb_hashRecType *, phash,
                 char **, ppszhashKey, char **, ppszhashData,
                 longint *, plPattern,
                 int *, pfCannotFindLonger)
{
char *pszStrNext; /* Str data area ptr */
char *pszStrKey; /* Str data area ptr */
char *pszStrData; /* Str data area ptr */
longint lNextKeyData; /* ptr to Str; Next + Key + Data */
char *pszKey;

  lNextKeyData = phash->lNextStrOut;
  pszKey = phash->szLastKey;
  while (lNextKeyData != 0l) {
    pszStrNext = HB_pszHASHSTR(phash,lNextKeyData); /* ptr to next 'record' */
    pszStrKey = pszStrNext + 8; /* key is after next and pat longints */
    /**/ /* fprintf(stderr,"cmp: <%s> <%s>\n",pszKey,pszStrKey); /**/
    if (!strcmp(pszKey,pszStrKey)) {
      /* same, return data */
      pszStrData = pszStrKey + strlen(pszStrKey) + 1;
      *ppszhashKey = pszStrKey;
      *ppszhashData = pszStrData;
      /* longint Pat offset == 4, just after next ptr: */
      phash->pchPatAtFound = pszStrNext+4; /* save ptr to pat longint */
      HB_HASHS2L(pszStrNext+4,
                   *plPattern);
      *pfCannotFindLonger = 1; /**/
      HB_HASHS2L(pszStrNext,lNextKeyData);
      phash->lNextStrOut = lNextKeyData;
      return 0; /* found */
    }
    HB_HASHS2L(pszStrNext,lNextKeyData);

    /**/ /* fprintf(stderr,"lNextKeyData: %ld\n",lNextKeyData); /**/    

  } /* of while lNextKeyData */
  
  return 1; /* not found any more */
} /* of hb_HashGetNextPtr */

int
FUNDEF2(hb_HashReplacePat, hb_hashRecType *, phash,
                longint, lPattern)
{ 
  HB_HASHL2S(lPattern,phash->pchPatAtFound);
  return 0;
} /* of hb_HashReplacePat */

int
FUNDEF4(hb_HashIncr, hb_hashRecType *, phash,
                     char *, pszKeyAdd,
                     longint, lPatAdd, 
                     char *, pszDataAdd)
{
char szData[HB_STRINGMAX];
longint lCount;
int fT;

  if (0 == hb_HashFindFirst(
                 phash,pszKeyAdd,szData,HB_STRINGMAX-2,
                 &lCount,&fT)) { /* found, increase number */
    do {
      /* found "next" record, data (y) in szData */
      if (!strcmp(pszDataAdd,szData)) {
        /* found! */
        lCount++; /* count incremented */
        if (0 != hb_HashReplacePat(phash,lCount)) {
          hb_FatalError(HB_HASH,714,pszKeyAdd);
          return 1; /* fatal internal error */
        }
/**/ /* 
    fprintf(stderr,"--%s//%s-- incr to %ld\n",pszKeyAdd,pszDataAdd,lCount);
     */
        return 0; /* incremented ok */
      } /* of y-value found */
      /* get next value: */
    } while (hb_HashGetNext(phash,szData,HB_STRINGMAX-2,
                 &lCount,&fT) == 0);
  }
  /* not found: either key not found, or no y-value found: */
  if (hb_HashAdd(phash,pszKeyAdd,lPatAdd+1l,pszDataAdd) > 0) {
    hb_FatalError(HB_HASH,713,pszKeyAdd);
    return 2;
  }
/**/ /* fprintf(stderr,"added: --%s//%s--\n",pszKeyAdd,pszDataAdd); */
  return 0; /* count 1 + add key + y-value ok */
} /* of hb_HashIncr */
                             
hb_hashRecType *
FUNDEF3(hb_HashNew, longint, lTableSize, longint, lStrMaxSize, 
             longint, lMaxData)
{
hb_hashRecType *phash;
Shortint cAlloc; /* for hb_alloc calls */
longint l; /* for loop index */

  if (lTableSize <= 1) {
    hb_FatalError(HB_HASH,701,"Too small table size in New");
    return NULL;
  }
  phash = (hb_hashRecType *) hb_LongAlloc(sizeof (hb_hashRecType));
  if (phash == NULL) return NULL;
  
  phash->lTabMax = (lTableSize - 1) / HB_HASHTABMAX + 1; 
  phash->lSize = phash->lTabMax * HB_HASHTABMAX;
  phash->lWarnEntries = phash->lSize - (phash->lSize / 5); /* warn
                            at 80% fill ratio */
  phash->lEntries = 0l;
  phash->lTabFilled = 0l;
  phash->lStrFree = 8;
  phash->lStrMax = (lStrMaxSize - 1) / HB_HASHSTRMAX + 1;
  /* phash->lStrSize = phash->lStrMax * HB_HASHSTRMAX; JH 23.1.98 */
  phash->lBufferMax = lMaxData;
  phash->lNextStrOut = -1l; /* (no/null) next data output ptr to Str */
  phash->lCurrHashIndex = -1l; /* no seq. access yet */
  phash->szLastKey = NULL; /* last key when FindFirst called */
  strcpy(phash->szFileName,"");
  phash->iFileHandle = -1;
  /**/ /* printf("lSize %ld\n",phash->lSize); /**/
  /**/ /* printf("lTabMax %ld\n",phash->lTabMax);  /**/
  /**/ /* printf("lStrMax %ld\n",phash->lStrMax);        /**/
  /* allocations: */
  cAlloc = phash->lBufferMax * sizeof(char);
  phash->pszBuffer = (char *) hb_LongAlloc(cAlloc);
  if (phash->pszBuffer == NULL) {
    return NULL;
  }
  cAlloc = phash->lTabMax * sizeof(longint *);
  if ((phash->prgprgTab = (longint **) hb_LongAlloc(cAlloc)) == NULL) {
    return NULL;
  }
  cAlloc = phash->lStrMax * sizeof(char *);
  if ((phash->prgpchStr = (char **) hb_LongAlloc(cAlloc)) == NULL) {
    return NULL;
  }

  for (l = 0; l < phash->lTabMax; l++) {
    if ((phash->prgprgTab[l] = (longint *) 
                hb_LongAlloc((Shortint)HB_HASHTABMAX * sizeof(longint))) == NULL) {
      return NULL;
    }
    memset((void *)(phash->prgprgTab[l]),0,
          (Shortint)HB_HASHTABMAX * sizeof(longint));
  } 
  for (l = 0; l < phash->lStrMax; l++) {
    if ((phash->prgpchStr[l] = (char *) 
                hb_LongAlloc(HB_HASHSTRMAX * sizeof(char))) == NULL) {
      return NULL;
    }
  } 
  return phash;
} /* of hb_HashNew */

FUNDEF1(int hb_HashFree, 
hb_hashRecType **, pphash)
{
hb_hashRecType *phash;
longint l;

  phash = *pphash;
  for (l = 0; l < phash->lStrMax; l++) {
    hb_Free(phash->prgpchStr[l]);
  } 
  for (l = 0; l < phash->lTabMax; l++) {
    hb_Free(phash->prgprgTab[l]);
  } 
  hb_Free(phash->prgpchStr);
  hb_Free(phash->prgprgTab);
  hb_Free(phash->pszBuffer);
  if (phash->szLastKey != NULL) {
    hb_Free(phash->szLastKey);
  }
  hb_Free(phash);
  *pphash = NULL;
  return 0;
} /* of hb_HashFree */

int
FUNDEF4(hb_HashAdd, hb_hashRecType *,phash,
             char *,pszAdd, longint, lPatAdd, char *,pszDataAdd)
{
longint lHashCode;
longint lHashIndex;
longint *plIndex;
longint lStrSubArray; /* free string subarray */
longint lStrSubOffset; /* free string offset within subarray */
longint lLenAdd; /* total no. of chars to add inc. z */
longint lStrDest; /* new string (Key + Data) destination */
longint lNextSameHash; /* ptr to Str; next Key + Data with same hash */
longint lT; /**/
/**/ char szOutTag[100]; /**/
/**/ int fT;
Shortint cAlloc; /* for hb_ReAlloc call */
char szT[HB_STRINGMAX];

  /**/ /* fprintf(stderr,"to add: <%s> <%s>\n",pszAdd,pszDataAdd); /**/
  lHashCode = hbp_HashCode(pszAdd);
  if (lHashCode < 0) lHashCode = -lHashCode;
  lHashIndex = lHashCode % (phash->lSize - 1);
  /**/ /* fprintf(stderr,"key hash code: %ld, index %ld\n",
    lHashCode,lHashIndex); /**/
  plIndex = HB_plHASHTAB(phash,lHashIndex);
  /**/ /* fprintf(stderr,"PHASH->RG[0] %lx\n",
    phash->prgprgTab[0]); /**/
  /**/ /* fprintf(stderr,"key hash code addr: %lx\n",
    plIndex); /**/
  lStrDest = phash->lStrFree;
  lStrSubOffset = lStrDest & HB_HASHSTRMASKL;
  lStrSubArray = (lStrDest & HB_HASHSTRMASKU) >> HB_HASHSTRBITS;
  lLenAdd = strlen(pszAdd) + strlen(pszDataAdd) + 4 + 7;
  if (lLenAdd > HB_HASHSTRMAX) {
    /* cannot add such a long thing... */
    return 1;
  }
  lNextSameHash = *plIndex; /* 0 if empty, otherwise ptr to Str */
  if (*plIndex == 0l) {
    /* will become nonempty, thus increase number of nonempty table cells: */
    phash->lTabFilled++;
  }
  if (++phash->lEntries == phash->lWarnEntries) {
    hb_Warning(HB_HASH,34,pszAdd);
    hb_HashStat(phash,szT);
    hb_Warning(HB_HASH,35,szT);
  }
  if (lStrSubOffset + lLenAdd > HB_HASHSTRMAX) {
    /* must move to another string subarray; no space left in curr */
    lStrSubArray++;
    if (lStrSubArray >= phash->lStrMax) {
      if (lStrSubArray >= (longint) HB_MAXUSHORT / sizeof(char *)) {
        hb_FatalError(HB_HASH,711,pszAdd);
        return 2;
      }
      lStrSubArray = phash->lStrMax;
      phash->lStrMax++;
      cAlloc = phash->lStrMax * sizeof(char *);
      if ((phash->prgpchStr = 
        (char **) hb_ReAlloc(phash->prgpchStr,cAlloc)) == NULL) {
        hb_FatalError(HB_HASH,721,pszAdd);
        return 3;
      }
      if ((phash->prgpchStr[lStrSubArray] = (char *) 
                hb_LongAlloc(HB_HASHSTRMAX * sizeof(char))) == NULL) {
        hb_FatalError(HB_HASH,722,pszAdd);
        return 4;
      }
/**/ /* fprintf(stderr,
          "hb_hash: REALLOCATED string area ptrs: %ld\n",phash->lStrMax); /**/

    }
    lStrSubOffset = 0;
    lStrDest = (lStrSubArray << HB_HASHSTRBITS) + lStrSubOffset;
  }
  /**/ /* fprintf(stderr,"ST lStrSubOffset: %ld (lStrSubArray: %ld)\n", 
      lStrSubOffset,lStrSubArray); /**/                     
  /**/ /* fprintf(stderr,"ST lStrFree: %ld (old Next ptr: %ld)\n", 
      phash->lStrFree,lNextSameHash); /**/
  /* add ptr to next: */
  HB_HASHL2S(lNextSameHash,phash->prgpchStr[lStrSubArray]+lStrSubOffset);
  /**/ /* HB_HASHS2L(phash->prgpchStr[lStrSubArray]+lStrSubOffset,lT); /**/
  /**/ /* if (lT != lNextSameHash) {
        fprintf(stderr,"HB_HASHS2L wrong: %ld x %ld\n",lNextSameHash,lT);
       } /**/
  lStrSubOffset += 4;
  /* add Pat number (4 bytes, longint): */
  HB_HASHL2S(lPatAdd,phash->prgpchStr[lStrSubArray]+lStrSubOffset);
  lStrSubOffset += 4;
  /* add Key string: */
  strcpy(phash->prgpchStr[lStrSubArray]+lStrSubOffset,pszAdd);
  lStrSubOffset += strlen(pszAdd) + 1;
  /* add data: */
  strcpy(phash->prgpchStr[lStrSubArray]+lStrSubOffset,pszDataAdd);
  lStrSubOffset += strlen(pszDataAdd) + 1;
  *(phash->prgpchStr[lStrSubArray]+lStrSubOffset) = HB_EOS;
  phash->lStrFree = (lStrSubArray << HB_HASHSTRBITS) + lStrSubOffset; 
  /**/ /* fprintf(stderr,"lStrSubOffset: %ld (lStrSubArray: %ld)\n", 
      lStrSubOffset,lStrSubArray); /**/                     
  /**/ /* fprintf(stderr,"decoded Next: %ld\n",lT); /**/
  /**/ /* fprintf(stderr,"new lStrFree: %ld (old Next ptr: %ld)\n", 
      phash->lStrFree,lNextSameHash); /**/
  *plIndex = lStrDest;
  /**/ /**/ /* fprintf(stdout,"### %ld\n",*(phash->prgprgTab[0]+6245)); */

  return 0; /* added o.k. */
} /* of hb_HashAdd */

int
FUNDEF3(hb_HashMemberPtr,
       hb_hashRecType *, phash, char *, pszKey, char **, ppszKeyPtr)
{
char *pszKeyPtr;
char *pszDummyData;
longint lDummyCount;
int fCFL;
int iRC;

  /* add pszKey if not there; alwasy return key *address* in internal
     hash structure (in *ppszKeyPtr) */
  /* return 0 if everything ok, return > 0 if error, return -1
     if just added */
  *ppszKeyPtr = NULL;
  if (0 > (iRC = hb_HashFindFirstPtr(phash,pszKey,
                    &pszKeyPtr,&pszDummyData,&lDummyCount,&fCFL))) {
    return (100-iRC); /* error when searching for pszKey */
  }
  if (iRC == 1) { /* not found */
    if (0 != (iRC = hb_HashAdd(phash,pszKey,1l,""))) {
      return (200+iRC); /* cannot add to hash table */
    }
    /* added, now get ptr back: */
    if (0 > (iRC = hb_HashFindFirstPtr(phash,pszKey,
                    &pszKeyPtr,&pszDummyData,&lDummyCount,&fCFL))) {
      return (300-iRC); /* error when searching back for pszKey */
    }
    *ppszKeyPtr = pszKeyPtr;
    return -1; /* ok, but was not there before it was added this call */
  }
  else { /* found -- already there */
    *ppszKeyPtr = pszKeyPtr;
    return 0;
  }  
} /* of hb_HashMemberPtr */

/* next function is again private: */                             
int
FUNDEF4(hb_HashFileInInternal, hb_hashRecType **,pphash, char *,szFileName, int, fInv, int, fNumbering)
{
hb_hashRecType *phash;
FILE *fIn;
char c, cLast; /* for reading from input file char by char */
longint lSize; /* file size --> string size */
longint lLines;               /* no of lines read from input */
                 /* numbered from 0 !! */
longint lCols; /* columns on analyzed line */
longint lMaxData; /* max data size */
longint lLineSize; /* current line length */
longint lT; /* temp */
longint lPattern; /* 0l or line number, for HashAdd */
char *szLine; /* current Line */
char *pszKey; /* ptr to current Key from current line */
char *pszData; /* ptr to Data on current Line */
char *pch; /* ptr to current line */
Shortint cAlloc; /* for allocations */
char szT[20]; /* for longint conversion */
char *pszSaved;

  fIn = fopen(szFileName,"rt");
  if (!fIn) {
    return 2;
  }
  lSize = 0;
  lLines = 0;
  cLast = 0;
  lCols = 1;
  lMaxData = 0;
  lLineSize = 0;
  while (fread((void*)&c, 1, 1, fIn) == 1) {
    lSize++;
    if (c == 10) { /* line feed */
      if (lLineSize > lMaxData) lMaxData = lLineSize;
      lLineSize = 0;
      lLines++;
    }
    else if (c == 13) { /* CR on DOS */
      lSize--;
    }
    else { /* other char */
      lLineSize++;
      if (lLines == 0) { /* first line -- determine type */
        if ((cLast == ' ' || cLast == '\t') && c != ' ' && c != '\t') {
          lCols++;
        }
      }
    }
    cLast = c;
  }
  /**/ /* fprintf(stderr,
        "hb_hash: %s: size %ld lines %ld cols %ld maxdata %ld\n",
        szFileName,lSize,lLines,lCols,lMaxData); /**/

  if (lCols <= 1) { /* add space for numbering */
    lSize += lLines * 9;  /* up to 100 mil. items */
  }     
  lSize += lLines * 10; /* next same key ptr + pat# (8), EOS chars etc. */
  if (lLines < 100) lLines = 120;
  else {
    lLines += ((lLines / 10) * 2);  /* add 20% to lines */
  }
  if (lMaxData < 100) lMaxData = 150;
  else {
    lMaxData /= 10; lMaxData *= 15; /* add 50% to max data size */
  }
  fclose(fIn);
  
  /**/ /* fprintf(stderr,
        "hb_hash: %s: bef HashNew: size %ld lines %ld cols %ld maxdata %ld\n",
        szFileName,lSize,lLines,lCols,lMaxData); /**/
  phash = hb_HashNew(lLines,lSize,lMaxData);
  if (phash == NULL) return 1;

  fIn = fopen(szFileName,"rt");
  if (!fIn) {
    return 2;
  }
  cAlloc = phash->lBufferMax * sizeof(char);
  if ((szLine = (char *) hb_LongAlloc(cAlloc)) == NULL) {
    return 3;
  }

  lLines = 0;
  cLast = 0;
  lLineSize = 0;
  *szLine = HB_EOS;
  pch = NULL;
  pszKey = NULL;
  pszData = NULL;
  while (fread((void *)&c, 1, 1, fIn) == 1) {
    if (c == 10) { /* line feed */
      if (pch == NULL) {
        /* ignore line -- null */
      }
      else { /* nonempty line */
        if (pszData == NULL) {
          /* must generate data (as line number) */
          sprintf(szT,"%ld",lLines); /* from 0: lLines not yet incr. */
          pszData = szT;
        }
        /* now pszData not NULL for sure */
        *pch++ = HB_EOS;
        if (fInv) { /* exchange key and data */
          pszSaved = pszData;
          pszData = pszKey;
          pszKey = pszSaved;
        }
        /* set "pattern" 32-bit number: */
        if (fNumbering) {
          lPattern = lLines; /* goes from 0! lLines incremented only below */
        }
        else {
          lPattern = 0;
        }
        if (hb_HashAdd(phash,pszKey,lPattern,pszData) > 0) {
          hb_FatalError(HB_HASH,702,pszKey);
          hb_Free(szLine);
          return 4;
        }
        pch = NULL;
        pszKey = NULL;
        pszData = NULL;
      } 
      lLines++;
    } /* of linefeed */
    else if (c == 13) { /* CR -- ignore on DOS */
    }
    else { /* other char */
      if (c == ' ' || c == '\t') {
        if (pszKey == NULL) {
          /* ignore spaces before beg. of key */
        }
        else { /* pszKey not null */
          if (pszData == NULL) {
            /* ignore spaces before data */
            /* but set end of key if needed: */
            if (cLast != ' ' && cLast != '\t') {
              *pch++ = HB_EOS;
            }
          }
          else {            
            /* space within data, just copy it, no problem... */
            *pch++ = c;
          }
        } /* of pszKey not NULL */
      } /* of c is space */
      else { /* c not space */
        if (pszKey == NULL) {
          /* first nonspace on line: */
          pszKey = pch = szLine;
          *pch++ = c;
        }
        else { /* pszKey not null */
          if (pszData == NULL) {
            /* we are within or after Key now, no data yet: */
            if (cLast == ' ' || cLast == '\t') {
              pszData = pch;
            }
            *pch++ = c;
          }
          else {            
            /* space within data, just copy it, no problem... */
            *pch++ = c;
          }
        } /* of pszKey not NULL */
      } /* of c not space */
      lLineSize++;
    } /* of c not CR/LF */
    cLast = c;
  } /* of reading file fIn */

  hb_Free(szLine);
  fclose(fIn);
  *pphash = phash;
  return 0;
} /* of hb_HashFileInInternal */

int
FUNDEF3(hb_HashFileIn, hb_hashRecType **,pphash, char *,szFileName, int, fInv)
{
  /* no auto numbering */
  return hb_HashFileInInternal(pphash,szFileName,fInv,0); 
} /* of hb_HashFileIn */

int
FUNDEF3(hb_HashFileInSeq, hb_hashRecType **,pphash,char *,szFileName,int, fInv)
{
  /* auto numbering in pat */
  return hb_HashFileInInternal(pphash,szFileName,fInv,1);
} /* of hb_HashFileIn */

int
FUNDEF2(hb_HashFileOut, hb_hashRecType *,phash, char *,szFileName)
{
int fOut;
longint lHashIndex;
longint *plIndex;
longint lKeyLen; /* total no. of chars in Key */
char *pszStrNext; /* Str data area ptr */
char *pszStrKey; /* Str data area ptr */
char *pszStrData; /* Str data area ptr */
longint lNextKeyData; /* ptr to Str; Next + Key + Data */
longint lPattern;
longint lLineLen;
Shortint cAlloc;
char szT[20];

  if (0 > (fOut = hb_Open(szFileName,"wt"))) {
    hb_FatalError(HB_HASH,715,szFileName);
    return 1;
  }
  for (lHashIndex = 0; lHashIndex < phash->lSize; lHashIndex++) {

    plIndex = HB_plHASHTAB(phash,lHashIndex);
    lNextKeyData = *plIndex; /* 0 if empty, otherwise ptr to Str */
    while (lNextKeyData != 0l) {
      pszStrNext = HB_pszHASHSTR(phash,lNextKeyData); /* ptr to key/data rec */
      pszStrKey = pszStrNext + 8; /* key is after next and pat longints */
      pszStrData = pszStrKey + strlen(pszStrKey) + 1;
      HB_HASHS2L(pszStrNext+4,lPattern);
      HB_HASHS2L(pszStrNext,lNextKeyData);
      lLineLen = strlen(pszStrKey)+12+strlen(pszStrData)+4;

      if (lLineLen > phash->lBufferMax) {
        if (lLineLen >= HB_MAXSSHORT) {
          sprintf(szT,"%ld",lLineLen);
          hb_FatalError(HB_HASH,36,szT);
        }
        /* must realloc buffer */
        cAlloc = lLineLen;
        if (NULL == (phash->pszBuffer = 
                     (char *) hb_ReAlloc(phash->pszBuffer,cAlloc))) {
          hb_FatalError(HB_HASH,723,pszStrKey);
          return 2;
        }
      }
      sprintf(phash->pszBuffer,"%s %ld %s\n",pszStrKey,lPattern,pszStrData);
      hb_Write(fOut,phash->pszBuffer,strlen(phash->pszBuffer));
    }
  } /* of for lHashCode */

  hb_Close(fOut);
  return 0;
} /* of hb_HashFileOut */

int
FUNDEF2(hb_HashFileDump, hb_hashRecType *,phash, char *,szFileName)
{
return 0;
}

int
FUNDEF2(hb_HashStat, hb_hashRecType *, phash,
             char *, pszStat)
{
  sprintf(pszStat,"tab %ld, full %ld, recs %ld, sz %ld",
         phash->lSize,phash->lTabFilled,phash->lEntries,phash->lStrFree);
  return 0;
} /* of hb_HashStat */
                             
   


#ifndef __HB_HASHH
#define __HB_HASHH

/*                               
  string hashing
 (C) Jan Hajic 11.03.97
  */

#include "proto.h"
#include "hb_base.h"

#define HB_HASHCHARSIZE (sizeof(char))
#define HB_HASHSHORTSIZE (sizeof(shortint))
#define HB_HASHINTSIZE (sizeof(int))
#define HB_HASHLONGSIZE (sizeof(longint))
#define HB_HASHPTRSIZE (sizeof(int *))

#define HB_HASHSTRMAX 32768
/* following 3 values must correspond to HB_HASHxxxMAX !! */
#define HB_HASHSTRMASKL 0x00007FFF
#define HB_HASHSTRMASKU 0xFFFF8000
#define HB_HASHSTRBITS 15

#define HB_HASHTABMAX 8192
/* following 3 values must correspond to HB_HASHxxxMAX !! */
#define HB_HASHTABMASKL 0x00001FFF
#define HB_HASHTABMASKU 0xFFFFE000
#define HB_HASHTABBITS 13

#define HB_HASHOVFMAX 4096
/* following 3 values must correspond to HB_HASHxxxMAX !! */
#define HB_HASHOVFMASKL 0x00000FFF
#define HB_HASHOVFMASKU 0xFFFFF000
#define HB_HASHOVFBITS 12

#define HB_plHASHTAB(phash,lIndex) (phash->\
 prgprgTab[((lIndex)&HB_HASHTABMASKU)>>HB_HASHTABBITS]+\
 ((lIndex)&HB_HASHTABMASKL))
#define HB_poHASHOVF(phash,lIndex) (phash->\
 prgprgOvf[((lIndex)&HB_HASHOVFMASKU)>>HB_HASHOVFBITS]+\
 ((lIndex)&HB_HASHOVFMASKL))
#define HB_pszHASHSTR(phash,lIndex) (phash->\
 prgpchStr[((lIndex)&HB_HASHSTRMASKU)>>HB_HASHSTRBITS]+\
 ((lIndex)&HB_HASHSTRMASKL))
#define HB_HASHL2S(l,sz) { *((unsigned char *)(sz)) = (l) & 0x000000FFul;\
                   *(((unsigned char *)(sz))+1) = ((l) & 0x0000FF00ul) >> 8;\
                   *(((unsigned char *)(sz))+2) = ((l) & 0x00FF0000ul) >> 16;\
                   *(((unsigned char *)(sz))+3) = ((l) & 0xFF000000ul) >> 24; }
#define HB_HASHS2L(sz,l) { l = *((unsigned char *)(sz)) +\
                              (*(((unsigned char *)(sz))+1) << 8) +\
                              (*(((unsigned char *)(sz))+2) << 16) +\
                              (*(((unsigned char *)(sz))+3) << 24); }
                           
typedef struct hb_hashRec {
  longint lSize;            /* hash table size; must be fixed from beg. */
  longint lTabMax;          /* no of tables = lSize/HB_HASHTABMAX */
  longint lStrFree;         /* String area curr size, or free index */
  longint lStrMax;          /* String area no of subarea = lStrSize/HB_STRMAX*/
  longint **prgprgTab;      /* pointer to pointer array to hash table seg. */
  char **prgpchStr;         /* pointer to pointer array to string subareas */
  longint lTabFilled;       /* no of nonempty cells in the hash table */
  longint lEntries;         /* total data entries in table (can be > lSize!) */
  longint lWarnEntries;     /* warn if so many entries */
  char szFileName[HB_STRINGMAX]; /* filename if any */
  int iFileHandle;          /* file handle from hb_Open or -1 if no file */
  char *pszBuffer;          /* buffer for result if needed */
  longint lBufferMax;       /* buffer size for output data building */
  longint lNextStrOut;      /* ptr to Str to next str output in Find/GetNext */
                            /* and for NextData */
  longint lCurrHashIndex;   /* current hash index for sequential access */
  char *szLastKey ;         /* last Key (for GetNext) */
  char *pchPatAtFound;      /* prt to pat longint in latest found data */
} hb_hashRecType;

PROTO(hb_hashRecType *
hb_HashOpen, (char *szHashFileName));

PROTO(int
hb_HashClose, (hb_hashRecType *hashFile));

PROTO(int
hb_HashFindFirst, (hb_hashRecType *phash, char *szKey,
                 char *szhashData, int cMaxDataLen,
                 longint *plPattern,
                 int *pfCannotFindLonger));

PROTO(int
hb_HashGetNext, (hb_hashRecType *phash,
                char *szhashData, int cMaxDataLen,
                longint *plPattern,
                int *pfCannotFindLonger));

PROTO(int
hb_HashFindFirstPtr, (hb_hashRecType *phash, char *szKey,
                 char **ppszhashKey, char **ppszhashData,
                 longint *plPattern,
                 int *pfCannotFindLonger));

PROTO(int
hb_HashGetNextPtr, (hb_hashRecType *phash,
                 char **ppszhashKey, char **ppszhashData,
                 longint *plPattern,
                 int *pfCannotFindLonger));

PROTO(int
hb_HashFirstData, (hb_hashRecType *phash,
                char *szhashKey, int cMaxKeyLen,
                char *szhashData, int cMaxDataLen,
                longint *plPattern));

PROTO(int
hb_HashNextData, (hb_hashRecType *phash,
                char *szhashKey, int cMaxKeyLen,
                char *szhashData, int cMaxDataLen,
                longint *plPattern));

PROTO(hb_hashRecType *
hb_HashNew, (longint lTableSize, longint lStringMaxSize, 
             longint lMaxData));

PROTO(int
hb_HashFree, (hb_hashRecType **));

PROTO(int
hb_HashAdd, (hb_hashRecType *phash,
             char *pszAdd, longint lPatAdd, char *pszDataAdd));
                             
PROTO(int
hb_HashMemberPtr, (hb_hashRecType *, char *, char **));

PROTO(int
hb_HashReplacePat, (hb_hashRecType *phash,
             longint lPat));
                             
PROTO(int
hb_HashIncr, (hb_hashRecType *phash,
             char *pszAdd, longint lPatAdd, char *pszDataAdd));
                             
PROTO(int
hb_HashFileIn, (hb_hashRecType **pphash, char *szFileName, int fInv));

PROTO(int
hb_HashFileInSeq, (hb_hashRecType **pphash, char *szFileName, int fInv));

PROTO(int
hb_HashFileOut, (hb_hashRecType *phash, char *szFileName));

PROTO(int
hb_HashFileDump, (hb_hashRecType *phash, char *szFileName));

PROTO(int
hb_HashStat, (hb_hashRecType *phash,
             char *pszStat));
                             
PROTO(int
hb_HashFirstDataPtr, (hb_hashRecType * phash,
		      char ** ppszhashKey,
		      char ** ppszhashData,
		      longint * plPattern));

PROTO(int
hb_HashNextDataPtr, (hb_hashRecType * phash,
		     char ** ppszhashKey,
		     char ** ppszhashData,
		     longint * plPattern));

#endif


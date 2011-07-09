/*************************************************************/
/*                                                           */
/* hf functions and internal declarations                    */
/*                                                           */
/*                                                           */
/* Changelog:                                                */
/* v3.6 30.10.2000 hajic correct multiple tag handling (bug  */
/*                       fix only)                           */
/* v3.5 24.09.1999 hajic merge from O.C.code                 */
/*                 remains: open -> hb_Open!                 */
/* v3.1            O.C.: Char, ifdef, etc.                   */
/* v3.0 28.08.1997 hajic  on-the-fly derivations             */
/*                        NB: major ver. 2 SW works with     */
/*                        major ver. 3 ending cpd file!      */
/* v2.2.1 01.08.1997 hajic                                   */
/* v2.2 14.05.1996 hajic                                     */
/* v2.1 30.07.1995 hajic patterns + patt --> expl map added  */
/* v2.0 04.04.1995 JH first attempt: morphology additions    */
/* v1.0 Jan Hajic 26.05.91  lemmatizace                      */
/*                                                           */
/* (C) Jan Hajic 1991-2000                                   */
/* (C) Jan Hric 1994-1996                                    */
/* (C) Ondrej Cikhart 1996-1999                              */
/*                                                           */
/*                                                           */
/*************************************************************/
#ifndef HF_DEFC

#include <stdio.h>
#include <string.h>
#include <fcntl.h>   /* JK 17.01.1993 */
#if !defined(unix)
#ifdef __DJGPP__
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#else
#include <io.h>    /* JK 17.01.1993 */
#include <share.h>   /* JK 17.01.1993 */
#include <sys\stat.h> /* JK 17.01.1993 */
#include <stdlib.h>
#endif
#else
#include <unistd.h>
#endif


/* JK 17.01.1993 malloc -> alloc */
static char *__File__ = __FILE__;
/*#define alloc malloc*/

/* JH 26.11.1994 */
/* 05.06.95 hajic all changed to .h only */
#include "hb_proto.h"
#include "hb_base.h"
#include "hf_codex.h"
#include "hf_isif.h"
#include "hf_modd.h"
/* 28.04.96 JH 
   hf_cp.h changed to hh_cp895.h (used for compilation in dict51.x) */
/* 22.11.97 JH changed hh_cp895 to hh_cp */
#include "hh_cp.h"

static int DICVERSION = 4; /* hajic 09.07.2001 */
static int iMajor = 3;  /* hajic 28.08.97 */
/* static int iMajor = 2; */
/* static int iMinor = 0; */
/* static int iMinor = 1; *//* hajic 30.07.95 */
/* static int iMinor = 2; *//* hajic 14.05.96 */
static int iMinor = 6; /* hajic 28.08.97 */

static int iMajorEnd;
static int iMinorEnd;

#define HEADER_SIZE 80

static int hf_cpd_dict; /* JK 17.01.1993 */
static int hf_cpd_ending; /* JK 17.01.1993 */

/*static Char hf_dx_space[HF_DX_SP];*//* JH 03.02.1994 */
static Char *hf_dx_space;
static Char **hf_dx_space_index; /* OC 22.5.1997 */
static unsigned int hf_dsx; /* size of hf_dx_space */
/*static Longint hf_dx_index[HF_DX_MAX];*//* JH 03.02.1994 */
static Longint *hf_dx_index;
static unsigned int hf_dxx; /* last element + 1, to hf_dx_index */
/*static Char hf_dbuffer[HF_DMAX_BUF];*//* JK 20.01.1994 */
static Char *hf_dbuffer;
static Longint hf_dboff; /* current buffer file offset */
static Char hf_rwr_space[256]; /* space for frequent class+rewrite info */
static unsigned int hf_sz_rwr_space; /* size of hf_rwr_space */
static Longint cMaxDataForKey; /* JH 30.08.97 */ /* max data
                           item for the same key */
static unsigned int hf_ddc; /* hf_dbuffer size, HF_DMAX_BUF replacement */
                            /* JH 03.02.1994 */

/*static Char hf_ex_space[HF_EX_SP];*//* JH 03.02.1994 */
static Char *hf_ex_space;
static Char **hf_ex_space_index; /* OC 22.5.1997 */
static unsigned int hf_esx; /* size of hf_ex_space */
/*static Longint hf_ex_index[HF_EX_MAX];*//* JH 03.02.1994 */
static Longint *hf_ex_index;
static unsigned int hf_exx; /* last element + 1, to hf_ex_index */
/*static Char hf_ebuffer[HF_ESIZE];*//* XX_HR */
static Char *hf_ebuffer; /* endings in memory */
static Longint hf_eboff; /* current buffer offset */
/* static Char *hf_elist[HF_EKEYMAX];*/ /* JH 03.02.1994 */
static Char **hf_elist; /* JH 03.02.1994 */
static unsigned int hf_ekeymax; /* JH 03.02.1994 */
static unsigned int hf_xelist;
static unsigned int hf_edc; /* actually it is irrelevant */
                            /* JH 03.02.1994 */

/* tags-related */

char **pszTag;
char *pszStrSpace;
char *pszStrFree;
longint cStrSize;
int cTags;
longint iTagsOffset;

/* added JH 25.11.1994 */
Char cNegFirst;
Char cAdjFirst;
Char cNejNo;

/* next block of decls added 30.07.1995 */
longint iPatExOffset;
char *rgchPat2Expl; /* map from pattern numbers to expl. letters
                       (= indexes to rgszExpl) */
char *rgpszPat[256]; /* array of pointers to sz patterns
                        index is the pat. number */
                      /* NB: patterns not (never) coded */
char *rgpszExpl[256]; /* array of pointers to sz explanation;
                        index is the expl. letter/char directly:
                        NB: only low 7 bits used in orig code! */
                      /* but it mey be coded (?) */
char *szPatSpace; /* space for pattern strings */
char *szExplSpace; /* space for explanation strings */


longint cszPatSpace;  /* actual pattern string space size */
longint cszExplSpace; /* actual explanation string space size */

longint crgPat; /* number of  patern strings */
longint crgExpl; /* number of explanation strings */

#define crgchPat2Expl crgPat

char chExplType; /* type of explanation: space, -, ? */
char iActMap; /* map index, active */
longint iActPat; /* pattern number, active */

/**************************************************** 1997 **************/
/* another new block of decl: added JH 29.08.97 (dyn. distributions) beg */

Char cCoded0;

/* all read from endings file, see /hf/a1113_9.fl for format */
longint iDistOffset; /* offset of following data in endings file */
longint cDistRules;   /* no. of distribution rules 
                       = size of rgDictPat, rgDistData */
int *rgDictPat; /* two bytes enough - array of dict pat numbers,
                   pointed to by rgDIstDataIndex[i] - ...[i+1] */
int *rgDistData; /* two bytes enough - array of dist data 
                    ptrs into szDistDataSpace,
                    pointed to by rgDistDataIndex[i] - ...[i+1] */
char *szDistDataSpace; /* dist rules: root,0,lemma+syn rules,0 */
longint cszDistDataSpace; /* size of the above */
int *rgDistDataIndex; /* master index, pointing to rgDictPat and
                         rgDistData tables to the first data for indexed
                         (ending) pattern number, size: crgPat+1 */
/* crgPat: already declared within pattern name handling, chg 30.07.95 */
/* next number only for easier handling, it must be equal crgPat+1 --
   it is checked in hf_init during read of endings file */
longint crgDistDataIndex; /* size of rgDistDataIndex */

/* dictionary patterns: read from dictionary! */
longint crgDictPat; /* number of DictPat strings */
char **rgpszDictPat; /* array of pointers to sz type dict patterns,
                         size: crgDictPat */
char *szDictPatSpace; /* space for DictPat strings */
longint cszDictPatSpace; /* actual DictPat string space size */

/* language from dictionary: */
char szLangDict[3];

typedef struct resultRec {
  int fNe; /* 1 if (-)ne- separated */
  int fNej;
  Char *pszcData;
  Char szcLemmaDerived[HF_MAX_ROOT_DER]; /* derived lemma 
                                         after hf_dist distribution */
  Char szcLemmaDerivedID[HF_IDMAX]; /* base form homonym id */
  Char szcLemmaDerivedInfo[HF_MAXLEMMA]; /* rest of info from rule */
  Char szcLemmaDict[HF_MAX_ROOT_DER]; /* source lemma from dict */
  Char szcLemmaDictID[HF_IDMAX]; /* base form homonym id */
  int iEndPattern; /* pattern from ending data (after deriv.) */
  int iDictPattern; /* pattern from dict (source pat before der.) */
  longint iEndTag; /* tag number from ending file */
  int iEndingLen; /* length of ending separated before root analysis */
  longint rgiEndTag[HF_MAXNO_TAGS]; /* tag number from ending file */
  int cEndTag; /* no. of tags */
} resultRecType;

resultRecType rrAct[HF_MAX_RESULT_REC]; /* global: parsed result(s) */

/* another new block of decl: added JH 29.08.97 (dyn. distributions) end */
/**************************************************** 1997 **************/



/**/ /* must be somehow dynamic... */
/* del JH 27.08.95 #define HF_NOUN_FIRST 5 */
/* next line deleted JH 25.11.94 */
/*#define HF_ADJ_FIRST HF_NEG_FIRST*/
/**/ /* must be somehow dynamic... */
/* del JH 27.08.95 #define HF_VERB_FIRST 134 */
static Char hf_results[HF_MAXRESULT];
static unsigned hf_xresult;
static unsigned hf_res_count;

/* JH 20.8.97 next 4 lines for checking lowercasing of NE, NEJ */
/* -- in order not to repeat morphology in such cases as NEJkratsi */
static Char szPrevRootS[32];
static Char szPrevRootNs[32];
static Char szPrevRootJs[32];
static Char szPrevRootJns[32];

/* JH 30.08.97 dict buffer caching decl BEG */
typedef struct dictbufcacheRec {
  char szcKeyCached[HB_STRINGMAX]; /* cached root (dict file key) */ 
  Char *szBuffer; /* size: hf_ddc (bufmax) */
  Char *pszcData; /* ptr to szBuffer, where data for key starts */
  int fFail; /* fail indication */
  char *rgfDicPatOk; /* flag == 1:
             (dict) pattern for szKeyCached in dict data; size: crgDictPat */
} dictbufcacheRecType;

#define CRGDICTCACHE 10
static int crgDictCache = CRGDICTCACHE;
           /* size of dictionary cache; fixed for now (prev define) */
static int irgDictCache; /* active (free bottom) of cache */
dictbufcacheRecType **rgpdbcAct; /* size: crgDictCache */
/* JH 30.08.97 dict buffer caching decl END */

  /* XX_HR 4/  28.6.1992 obsluha ne- */
static Char hf_ne_results[HF_MAXNO_RESULTS]; /* (bool) iff odtrhnute ne- */
static unsigned hf_no_res;   /* poradove cislo vysledku */
unsigned hf_is_ne;    /* (bool) nastavi hf_get_res, iff odtrhnute ne- */

int
hf_err(s)
char *s;
{
#ifdef HF_ERR
  fprintf(stderr,"hf: error: %s\n",s);
#endif
  return(0);
} /* of hf_err */

int
hf_cvs(s,t)
Char *s;
Char *t;
{
  while (*s) {
    *t++ = hh_tab[*s++];
  }
  *t = 0;
  return(0);
} /* of hf_cvs */

int
hf_inv_cvs(s,t)
Char *s;
Char *t;
{
  while (*s) {
    *t++ = hh_inv_tab[*s++];
  }
  *t = 0;
  return(0);
} /* of hf_inv_cvs */

int
hf_reverse(res,src)
Char *res;
Char *src;
{
int xres;
  xres = strlen((char *)src);
  res[xres] = 0;
  while (*src)
    res[--xres] = *src++;
  return(0);
} /* of hf_reverse */

/* OC 22.5.1997 */

int hf_get_expl(c,s)
int c;
char *s;
{
 return hf_inv_cvs((Char *) rgpszExpl[hh_tab[c]]+1,(Char *) s);
}

/* OC 22.5.1997 */
int
hf_find_index(s,table,index_table,table_size,buffer,pcurrent_offset,
              f,pres)
Char *s;
Longint *table;
Char **index_table;
unsigned int table_size;
Char *buffer;
Longint *pcurrent_offset;
/*FILE *f;*/
int f; /* JK 17.01.1993 */
int *pres;
{
Char *ps;
Char *str_item;
Longint read_offset;
unsigned int read_bytes;
unsigned int l_bound, u_bound, pilot;
  l_bound=0; u_bound=table_size-1;
  while (1) {
    pilot=(l_bound+u_bound)/2;
    ps = s;
    str_item = index_table[pilot];
    while (1) {
      if (*ps < *str_item) {
        /* is lower */
        u_bound = pilot;
        break;
      }
      else if (*ps > *str_item) {
        /* is greater */
        l_bound = pilot+1;
        break;
      }
      else /* chars equal */ if (*ps == 0) {
        /* both strings equal, it's here, too */
        l_bound = pilot; u_bound = pilot;
        goto found;
      }
      /* strings continue, try next char */
      ps++;
      str_item++;
    } /* of while */
    if (l_bound == u_bound) break;
  } /* of while */
  if (l_bound < table_size-1) goto found;
  /* not found at all */
  *pres = 0;
  return(0);
found:
  read_offset = table[l_bound];
  if (f == -1) { /* no need to read endings, they */ /* JK 14.02.1993 */
    /* are in memory all the time */
    *pcurrent_offset = read_offset;
  }
  else {
    if (read_offset != *pcurrent_offset) {
      read_bytes = table[l_bound+1] - read_offset;
      if (lseek(f,read_offset,SEEK_SET) < 0) { /* JK 17.01.1993 */
        hf_err("seek");
        return(1);
      }
      if (read(f,buffer,read_bytes) != read_bytes) { /* JK 17.01.1993 */
        hf_err("indx");
        return(2);
      }
      *pcurrent_offset = read_offset;
      buffer[read_bytes] = 0; /* curr buffer closing zero */
    }
  } /* of dict reading */
  *pres = 1;
  return(0);
} /* of hf_find_index */

int
hf_dsearch(s,r,pfail_type)
Char *s; /* string to find, internal coding! */
Char **r; /* result or NULL; ptr to buffer! */
int *pfail_type;
{
int found;
int error;
Char *ps_char;
Char *pdata;
int ps_index;
/*** if (strlen(s) > 30) fprintf(stderr,"searching for %s\n",s); /**/
  if ((error = hf_find_index(s,hf_dx_index,hf_dx_space_index,hf_dxx,
    hf_dbuffer,&hf_dboff,hf_cpd_dict,&found)) != 0) {
    hf_err("dsea");
    *pfail_type = 1;
    *r = NULL;
    return(error);
  }
  if (!found) {
    *pfail_type = 2;
    goto not_found;
  }
  /* found in index, buffer read in... (in hf_find_index) */
  pdata = hf_dbuffer + 1; /* first byte always 0 */
  ps_char = s;
  /* ps_char ends by zero, pdata by strip count (always < HF_FIRST_CHAR) */
  while (*ps_char) {
    if (*ps_char < *pdata) {
      *pfail_type = 3;
      goto not_found; /* not found, data greater */
    }
    if (*ps_char > *pdata) { /* data smaller, find next data */
        /* (goes here also if *pdata ends, ie. < 32)!! */
      ps_index = ps_char - s;
      do {
        while (*pdata++ >= HF_FIRST_CHAR); /* skip rest of tail of key */
        /* skip strip cnt + pattern */
        while (*(++pdata) >= HF_FIRST_CHAR); /* skip tail of replace */
        if (*pdata == HF_FIRST_CHAR - 2) {
          pdata++; /* points to real same key length */
	  /*** if (strlen(s) > 30) fprintf(stderr,"long data at key %s\n",s); /**/
        }
      } while (*pdata++ > ps_index); /* no need to check data with */
        /* longer same prefix - cannot equal, too */
      ps_char = s + *(pdata-1);/* lower beginning of compare index */
    } /* of shift to next candidate data */
    else { /* s eq data - do next char */
      ps_char++;
      pdata++;
    } /* of shift to next char */
  } /* of while *ps_char */
  /* here: *ps_char zero */
  if (*pdata >= HF_FIRST_CHAR) {
    /* data longer (greater) ... not found */
    *pfail_type = 0;
    goto not_found;
  }
  /* *pdata ends, too --> same lgt, found! */
  *pfail_type = 0;
  *r = pdata; /* return value, ptr to buffer (to strip count) */
  return(0);

not_found:
  *r = NULL;
  return(0);

} /* of hf_dsearch */

int
hf_esearch(s,r,pfail_type)
Char *s; /* string to find, internal coding! */
Char **r; /* result or NULL; ptr to buffer! */
int *pfail_type;
{
int found;
int error;
Char *ps_char;
Char *pdata;
int ps_index;
  if ((error = hf_find_index(s,hf_ex_index,hf_ex_space_index,hf_exx,
    hf_ebuffer,&hf_eboff,-1,&found)) != 0) { /* JK 14.02.1993 */
    hf_err("esea");
    *pfail_type = 1;
    *r = NULL;
    return(error);
  }
  if (!found) {
    *pfail_type = 2;
    goto not_found;
  }
  /* found in index, buffer read in... (in hf_find_index) */
  pdata = hf_ebuffer + hf_eboff + 1; /* first byte always 0 */
  ps_char = s;
  /* ps_char ends by zero, pdata by strip count (always < HF_FIRST_CHAR) */
  while (*ps_char) {
    if (*ps_char < *pdata) {
      *pfail_type = 3;
      goto not_found; /* not found, data greater */
    }
    if (*ps_char > *pdata) { /* data smaller, find next data */
        /* (goes here also if *pdata ends, ie. < 32)!! */
      ps_index = ps_char - s;
      do { /* the following differs slightly from dict... */
        while (*pdata++); /* skip rest of tail of key, end == 0 */
        /* skip 0, and suppose at least 1 class there */
        while (*(++pdata) >= HF_FIRST_CHAR); /* classes */
      } while (*pdata++ > ps_index); /* no need to check data with */
        /* longer same prefix - cannot equal, too */
      ps_char = s + *(pdata-1);/* lower beginning of compare index */
    } /* of shift to next candidate data */
    else { /* s eq data - do next char */
      ps_char++;
      pdata++;
    } /* of shift to next char */
  } /* of while *ps_char */
  /* here: *ps_char zero */
  if (*pdata >= HF_FIRST_CHAR) {
    /* data longer (greater) ... not found */
    *pfail_type = 0; /* longer, extended key may have success */
    goto not_found;
  }
  /* *pdata ends, too --> same lgt, found! */
  *pfail_type = 0; /* longer key may have success, too */
  *r = pdata+1; /* return value, ptr to buffer (to 1st class) */
  return(0);

not_found:
  *r = NULL;
  return(0);

} /* of hf_esearch */

FUNDEF3(int
ReverseRootChange,
Char *, pszcRootOrig,
Char *, pszcRootChg,
Char *, pszcRootOut)
{
int iLenEnd;
int iLenRootOut;
int iLenRootIn;

/**/ /* fprintf(stderr,"ReverseRootChange: <%s> x <%s>\n",
             pszcRootOrig,pszcRootChg); /**/
  /* NB: '0' change must be checked out of this function!! */
  if (*pszcRootChg == HF_EOS) { /* zero change always ok */
    strcpy((char *) pszcRootOut,(char *) pszcRootOrig);
    return 1;
  }
  /**/ /* no ~ in pattern now allowed!! */
       /* does not even check for it */
  iLenRootIn = strlen((char *) pszcRootOrig);
  iLenEnd = strlen((char *) pszcRootChg);
  iLenRootOut = iLenRootIn - iLenEnd;
  if (/*pszcRootOrig[iLenRootIn-1] == pszcRootChg[iLenEnd-1] &&
        moved before this function call */
      iLenRootOut >= 1 &&
      !strcmp((char *) pszcRootChg,(char *) pszcRootOrig+iLenRootOut)) {
    /* ok, just strip it */
    strcpy((char *) pszcRootOut,(char *) pszcRootOrig);
    pszcRootOut[iLenRootOut] = HF_EOS;
    return 1;
  }
  return 0;
} /* of ReverseRootChange */

FUNDEF3(int GetDictLemma,
Char *, pszcDictRoot,
Char **, ppszRealData,
resultRecType *, prrAct)
{
Char *pchcAppend;
Char *pchcDictLemma;
Char *pchcDictLemmaID;

  strcpy((char *) prrAct->szcLemmaDict, (char *) pszcDictRoot);
  pchcDictLemma = prrAct->szcLemmaDict 
                    + strlen((char *) prrAct->szcLemmaDict) - *(*ppszRealData);
  pchcAppend = (*ppszRealData) + 2;
  while (*pchcAppend >= HF_FIRST_CHAR && 
         *pchcAppend != hh_tab[HF_ID_SEP] &&
         *pchcAppend != hh_tab[HF_MULTILEMMA_SEP] &&
         *pchcAppend != hh_tab[HF_TAG_SEP] &&
         *pchcAppend != hh_tab[HF_EXPL_SPACE]) {
    *pchcDictLemma++ = *pchcAppend++;
  }
  *pchcDictLemma = HF_EOS;
  pchcDictLemmaID = prrAct->szcLemmaDictID;
  while (*pchcAppend >= HF_FIRST_CHAR && 
         *pchcAppend != hh_tab[HF_TAG_SEP] &&
         *pchcAppend != hh_tab[HF_EXPL_SPACE]) {
    *pchcDictLemmaID++ = *pchcAppend++;
  }
  *pchcDictLemmaID = HF_EOS;
  *ppszRealData = pchcAppend;
  return 0;
} /* of GetDictLemma */

FUNDEF3(int ConstructLemma,
Char *, pszcDictRoot,
char *, pszcRule,
resultRecType *, prrAct)
{
Char *pchcAppend;
Char *pchcDictLemma;
Char *pchAction;
char chForLemma;
int cLemmaCut;
char pszRule[HF_MAXBUILD];
char szT[HB_STRINGMAX];

  /* use the dist rule to make the derivation lemma: */
  /**/ /* must also do uv, in hard changes! */
  hf_inv_cvs((Char *) pszcRule,(Char *) pszRule); /* JH 26.11.1997 */
  pchAction = (Char *) pszRule;
/**/ /* fprintf(stderr,"rule: %s\n",pszRule); /**/
  cLemmaCut = 0;
  chForLemma = 'l'; /* default: take orig lemma as base for new l. */
  if (*pchAction == 'R' || *pchAction == 'r') {
    chForLemma = 'r';
    pchAction++;
  }
  if (chForLemma == 'l') {
    /* start from lemma, otherwise from root */
    strcpy((char *) prrAct->szcLemmaDerived,(char *) prrAct->szcLemmaDict);
  }
  else {
    strcpy((char *) prrAct->szcLemmaDerived,(char *) pszcDictRoot);
  }
  while (*pchAction != HB_EOS && *pchAction != ',') {
    if (*pchAction >= '0' && *pchAction <= '9') {
      cLemmaCut = cLemmaCut*10 + (*pchAction - '0');
    }
    else {
      hf_err("wasn");
      /**/ hf_inv_cvs(prrAct->szcLemmaDerived,(Char *) szT); /**/
      /**/ hf_err(szT); /**/
      /**/ hf_err(pszRule); /**/
      return(1);
    }
    pchAction++;
  }
  if (*pchAction == HB_EOS) {
    hf_err("wass");
    return(2);
  }
  while (*pchAction != HB_EOS && (*pchAction == ' ' ||
                                  *pchAction == '\t')) pchAction++;
  /* start appending from cut position: */
  if (*pchAction++ != ',') {
    /* skip comma */
    hf_err("cmsd");
    return(6);
  }
  while (*pchAction != HB_EOS && (*pchAction == ' ' ||
                                  *pchAction == '\t')) pchAction++;
  pchcAppend = prrAct->szcLemmaDerived 
                 + strlen((char *) prrAct->szcLemmaDerived) - cLemmaCut;
  if (*pchAction == '0') pchAction++;
  while (*pchAction != HB_EOS && 
         *pchAction != HF_ID_SEP &&
         *pchAction != HF_MULTILEMMA_SEP &&
         *pchAction != HF_TAG_SEP &&
         *pchAction != HF_EXPL_SPACE &&
         *pchAction != ',') {
    *pchcAppend++ = hh_tab[*pchAction++];
  }
  *pchcAppend = HF_EOS; /* final output lemma zero char */
  /* lemma id: */
  pchcAppend = prrAct->szcLemmaDerivedID;
  while (*pchAction != HB_EOS && 
         *pchAction != HF_TAG_SEP &&
         *pchAction != HF_EXPL_SPACE &&
         *pchAction != ',') {
    *pchcAppend++ = hh_tab[*pchAction++];
  }
  *pchcAppend = HF_EOS; /* final output lemma ID zero char */
  /* rest of info: */
  pchcAppend = prrAct->szcLemmaDerivedInfo;
  while (*pchAction != HB_EOS && 
         *pchAction != ',') {
    *pchcAppend++ = hh_tab[*pchAction++];
  }
  *pchcAppend = HF_EOS; /* final output lemma info zero char */
  if (*pchAction == HB_EOS) { /* there should be synonym part in the rule! */
    hf_err("wasl");
    return(3);
  }
  return 0;
} /* of ConstructLemma */

FUNDEF3(int BuildSingleResult,
Char *, pszResult,
resultRecType *, prrAct,
int, fOutFlag)
{

  /* JH 30.08.97 from hf_Expl */
  /* adds synt explanation if not already there */
  /* expands expl abbreviations */
  /* sets ne/nej variables in tag */ /* JH 95 */
  /* appends tag */
  /* appends result */

int i;
char *szT; /* tag (End) ptr */
char *pszT; /* tag (End) ptr */
int fIrregTag; /* 1 if irreg tag in data present */
char *pchExpl; /* for expanding explanations all all kinds */
int fSyntDone; /* Synt/POS already found and expanded */
int iAppState; /* state during expl expansions/copy rest */
Char *puch; /* for expanded expl. copy */
int cSame; /* counting identical characters in orig and derived lemma */
int cCut; /* counting 'to-be-cut' characters from derived lemma */
char *pchOrig; /* orig lemma chars */
char *pchDerived; /* derived lemma chars */
char szLemOrigID[HB_STRINGMAX]; /* orig lemma, plus sem id */
char szLemDerivedID[HB_STRINGMAX]; /* derived lemma, plus sem id */
char szDerInfo[HB_STRINGMAX]; /* readable form of dervation info paren */
char szEndLen[3]; /* for ending length conversion */

Char *pchcData; /* data from dictionary (buffer) */
int iResult; /* index into pszResult */
int irgTag;
int iTagOut;

  /* start with derived lemma and its ID: */
  strcpy((char *) pszResult,(char *) prrAct->szcLemmaDerived);
  if (*(prrAct->szcLemmaDerivedID) == HB_EOS) { /* empty der. ID */
    strcpy((char *) prrAct->szcLemmaDerivedID, (char *) prrAct->szcLemmaDictID);
  }
  strcat((char *) pszResult,(char *) prrAct->szcLemmaDerivedID);
  iResult = strlen((char *) pszResult);

  /* JH 21.08.95 */
  /* find end of lemma (inc. tag if any; flag tag if present) */
  pchcData = prrAct->pszcData;
  fIrregTag = 0;
  if (*pchcData == hh_tab[HF_TAG_SEP]) {
    fIrregTag = 1;
  }

  /* JH 21.08.95/30.08.97 patterns */
  pszResult[iResult++] = hh_tab[HF_PATSEP_CHAR]; /* separator of 1st, 
                                                    dictionary pattern */

  sprintf(szEndLen,"%02d",prrAct->iEndingLen); /* length of ending separated */
  pszResult[iResult++] = hh_tab[*szEndLen]; /* first two chars after PATSEP */
  pszResult[iResult++] = hh_tab[szEndLen[1]]; /* are ending length in dec */
  
  puch = (Char *) rgpszDictPat[prrAct->iDictPattern];
  while (*puch != HF_EOS) { 
    pszResult[iResult++] = hh_tab[*puch++];
  }
  pszResult[iResult++] = hh_tab[HF_PAT2SEP_CHAR]; /* separator of 
                                                     2nd (ending) pattern */
  puch = (Char *) rgpszPat[prrAct->iEndPattern];
  while (*puch != HF_EOS) {
    pszResult[iResult++] = hh_tab[*puch++];
  }
    /**/ /* fprintf(stderr,"real data: %c%c%c%c...\n",hh_inv_tab[*pchcData]
               ,hh_inv_tab[pchcData[1]]
               ,hh_inv_tab[pchcData[2]]
               ,hh_inv_tab[pchcData[3]]); /**/
  /* JH 21.08.95 rest of tag and other info... if any */
  while (*pchcData >= HF_FIRST_CHAR && *pchcData != hh_tab[HF_EXPL_SPACE]) {
    pszResult[iResult++] = *pchcData++;
  }

  /* JH 21.08.95 add tag here */
  /* JH 26.08.96 commented out */
  /* 
  szT = prrAct->rgiEndTag[0];
  if (fIrregTag && *szT != HF_EOS) { COMMENT tag redefinition!? 
    pszResult[iResult++] = hh_tab['?'];
    pszResult[iResult++] = hh_tab['G'];
    pszResult[iResult++] = hh_tab['!'];
  } COMMENT of tag redefinition check 
  */

  iTagOut = 0;
  if (!fIrregTag) { /* JH 26.08.96 added left brace here to put while inside */
    /* JH 26.08.96 do *NOT* append input tag if irreg tag present in lemma */
    /* WAS: append input tag in any case: */

    /* get tag(s) string: JH 26.11.1994 */
      for (irgTag = 0; irgTag < prrAct->cEndTag; irgTag++) {
      i = prrAct->rgiEndTag[irgTag];
      pszT = szT = pszTag[i];
      if (*szT == HF_GRAD_TAGC) {
        szT++;
        pszT++;
      }
      if (*pszT != HF_EOS) {
        if (iTagOut++ == 0) {
          pszResult[iResult++] = hh_tab[HF_TAG_SEP];
        }
        else {
          pszResult[iResult++] = hh_tab[HF_TAG_ALT];
        }
        while (*pszT != HF_EOS) { /* appending tag from ending file */
          pszResult[iResult++] = hh_tab[*pszT++];
        } /* of adding input tag */
      }
    } /* of loop over tags from prrAct */
  } /* JH 26.08.96 added right brace here to match the left one above */
  /* JH 21.08.95 add tag: end */

  /* JH 21.08.95 add rest of append; adds nothing if no expl in dict  */
  /* expand expl! */
  /* NB: we're still in 'coded' state! */
  iAppState = 0;
  fSyntDone = 0;
  while (*pchcData >= HF_FIRST_CHAR) {
    pszResult[iResult] = *pchcData; /* always copy char; replaced if expansion
                                       occurs */
    switch (iAppState) {
     case 0: /* unmarked case: start, too */
      if (*pchcData == hh_tab[HF_EXPL_SPACE]) {
        if (pchcData[1] == hh_tab[HF_LEFT_COMMENT]) { /* JH 15.11.1996 */
          if (pchcData[2] == hh_tab[HF_EXPL_SPACE]) {
            /* comment _(_ -- to be ignored */
            iResult--;
            iAppState = 3;
          }
          else if (pchcData[2] == hh_tab[HF_SYN]) {
            /* comment _(* -- to be ignored as well for now;
               otherwise should remember for syn calls */
            iResult--;
            iAppState = 3;
          }
          else {
            /* left paren after space: always as left paren, has precedence
              over single-char-between-spaces principle */
            iResult++;
            pchcData++;
            pszResult[iResult] = *pchcData;
            iAppState = 2;
          }
        }
        else { /* underscore plus not left paren */
          iAppState = 1;
        }
      } /* of underscore */
      else if (*pchcData == hh_tab[HF_LEFT_COMMENT]) {
        iAppState = 2;
      }
      break;
     case 1: /* previous: HF_EXPL_SPACE */
      /* lookahead: */
      if (*(pchcData+1) == hh_tab[HF_EXPL_SPACE] ||
          *(pchcData+1) < HF_FIRST_CHAR) {
        /* single char after HF_EXPL_SPACE */
        /* expand it if flag does not prevent it: */
        puch = (Char *) rgpszExpl[*pchcData];
                               /* still everything coded... */
        if (*puch++ == hh_tab[HF_EXPL_SYNT])
          fSyntDone = 1;
        if (fOutFlag == 0) {
          /* fprintf(stderr,"expanding %c into...\n",hh_inv_tab[*pchcData]); */
          while (*puch != HF_EOS) {
            /* fprintf(stderr,"%c",hh_inv_tab[*puch]); */
            pszResult[iResult++] = *puch++;
          }
          /* fprintf(stderr,"\n"); */
          iResult--;
        }
      }
      iAppState = 0;
      break;
     case 2: /* inside parenthesis: copy only */
      if (*pchcData == hh_tab[HF_RIGHT_COMMENT]) {
        iAppState = 0;
      }
      break;
     case 3: /* ignoring _(_ comment */
      iResult--;
      if (*pchcData == hh_tab[HF_RIGHT_COMMENT]) {
        iAppState = 0;
      }
      break;
     default: /**/ /* JH should fatally exit... */ iAppState=0;
    } /* of switch iAppState */
    iResult++;
    pchcData++;
  }
  /* JH 21.08.95 end of add rest of pchcData */

  /* JH 30.08.97 add derivation info: BEG */
  cSame = 0;
  strcpy((char *) szLemOrigID,(char *) prrAct->szcLemmaDict);
  strcat((char *) szLemOrigID,(char *) prrAct->szcLemmaDictID);
  strcpy((char *) szLemDerivedID,(char *) prrAct->szcLemmaDerived);
  strcat((char *) szLemDerivedID,(char *) prrAct->szcLemmaDerivedID);

  if (strcmp((char *) szLemOrigID,(char *) szLemDerivedID)) { /* different, construct deriv.
             info */
    pchOrig = szLemOrigID;
    pchDerived = szLemDerivedID;
    while (*pchOrig != HB_EOS && *pchDerived != HB_EOS &&
           *pchOrig == *pchDerived) {
      pchOrig++;
      pchDerived++;
      cSame++;
    }
    cCut = strlen(szLemDerivedID) - cSame;
    /* derivation origin rule output: */
    sprintf(szDerInfo,"%c%c%c%d",
                 HF_EXPL_SPACE,HF_LEFT_COMMENT,HF_DERIV_INFO,cCut);
    pchDerived = szDerInfo;
    while (*pchDerived != HB_EOS) {
      pszResult[iResult++] = hh_tab[*pchDerived++];
    }
    while (*pchOrig != HB_EOS) {
      pszResult[iResult++] = *pchOrig++;
    }
    pszResult[iResult++] = hh_tab[HF_RIGHT_COMMENT];

  } /* of constructing and appending der. info */
  /**/ /* the following should also go through symbol expansion!! */ /**/
  pchDerived = (char *) prrAct->szcLemmaDerivedInfo;
  while (*pchDerived != HB_EOS) { /* add any info from derived lemma: */
    pszResult[iResult++] = *pchDerived++;
  }
  /**/ /* JH 30.08.97: should (?) also get synonym from rule */
  /* JH 30.08.97 add derivation info -- END */

  /* JH 21.08.95 add default synt/POS if no synt in dict */
  if (!fSyntDone) {
    pszResult[iResult++] = hh_tab[HF_EXPL_SPACE];
    if (fOutFlag == 0) { /* ok, expand 1.8.97 JH */
      puch = (Char *)
              rgpszExpl[(Char) rgchPat2Expl[prrAct->iEndPattern]];
                      /* still everything coded...; table too */
      puch++; /* skip type info */
      while (*puch != HF_EOS) {
        /* fprintf(stderr,"%c",hh_inv_tab[*puch]); */
        pszResult[iResult++] = *puch++;
      }
      /* fprintf(stderr,"\n"); */
    }
    else { /* do not expand */
      pszResult[iResult++] = (Char) rgchPat2Expl[prrAct->iEndPattern];
    }
  }
  /* JH 21.08.95 of add default synt/POS index letter */

  pszResult[iResult] = 0;
  /* now convert pszResult to readable form: DECODE HERE DECODE HERE DECODE! */
  hf_inv_cvs(pszResult,pszResult);

  /* JH 19.7.1995 beg */
  /* change @ and/or # in tags to reflect negation/gradation */
  /* NB: now we are already decoded to readable form! */
  pszT = (char *) pszResult;
  while (*pszT != HF_EOS && *pszT++ != HF_TAG_SEP);
  /* pszT either EOS or start of tag string */
  while (*pszT != HF_EOS && *pszT != HF_EXPL_SPACE) {
    if (*pszT == HF_NEG_VAR) { /* ne- replacement char */
      if (prrAct->fNe) {         /* ne- present */
        *pszT = HF_NEG_NEG;
      }
      else {
        *pszT = HF_NEG_AFF;
      }
    }
    else if (*pszT == HF_NEJ_VAR) { /* nej- replacement char */
      if (prrAct->fNej) {             /* nej- present */
        *pszT = HF_NEJ_SUP; /* superlative */
      }
      else {
        *pszT = HF_NEJ_CMP; /* comparative */
      }
    }
    /* do not break then/else branches; may be both */
    pszT++;
  }
  /* JH 19.7.1995 end */

  return 0;
} /* of BuildSingleResult */

FUNDEF2(int AppendToResults,
Char *, pszIn,
int, fNeSep)
{
Char *new_result;
Char *old_result;
Char *pnew_result;
Char *pold_result;
int fLemmaFound;
Char *pszLemmaFound;
int cLemmaLength;
int cAppLength;
int iResultsFreeBeg;
int cCharsToInsert;
int cCharsToMove;
int cOldResKeep;
int iCharMoving;
int iCharMovingFrom;
int iCharMovingTo;
char *pchCharMovingTo;
char *pchCharMovingFrom;
char *pchIn;
char *pchCheckOld;
char *pchCheckNew;
int fTagDiffOnly;
char *pchOldTagEnd;
char *pchTagOldBeg; /* beginning of (separator before) tags in 'old' result */
char *pchTagNewBeg; /* beginning of (separator before) tags in the result
                       being added; both for "merge" */
char *pchTagEnd;
char *pchNewResult;
int cInsertOffset;
char *pchTagNew;
char *pszTagNew;
char chSave;
char szNewTagsEtc[HB_LONGSTRINGMAX]; /* new 10/28/00: move part of new res. */

  /* !! strings already readable (in KAM) (decoded in BuildSingleResult, 
                  while in build) */
  /* JH 1.8.97: lemma checking, output factoring according to lemma */
  /*            purpose: each hf_get_res call gets another lemma */
  fLemmaFound = 0;
  new_result = hf_results + hf_xresult;
  if (*pszIn == 0) return(0);
/* XX_HR 4 b. */
  if (hf_res_count >= HF_MAXNO_RESULTS) {
    hf_err("nepr");
    return(1);
  }
/* XX_HR 4 e. */
  pchIn = (char *) pszIn;

/**/ /* fprintf(stderr,"new res: %s, old res: <%s>\n",pszIn,hf_results); /**/
  iResultsFreeBeg = hf_xresult;
  while (*pchIn) {
    if (hf_xresult >= HF_MAXRESULT) {
      hf_err("ress");
      hf_results[--hf_xresult] = 0;
      hf_results[--hf_xresult] = 0;
      hf_ne_results[hf_res_count] = fNeSep; /* XX_HR 4 */
      return(1);
    }
    hf_results[hf_xresult++] = *pchIn++;
  }
  cAppLength = hf_xresult - iResultsFreeBeg;
  hf_results[hf_xresult++] = 0;
  old_result = hf_results;
    /* test if result already there */
  while (new_result != old_result) {
    pnew_result = new_result;
    pold_result = old_result;
    while (*pnew_result && *pnew_result != HF_EXPL_SPACE
      && *pnew_result == *pold_result) {
      if (*pnew_result == HF_PATSEP_CHAR) {
        /* lemmas end here and thus are the same */
        /* this code assumes lemmas unified -- otherwise would
           remember the last of them */
        cLemmaLength = pnew_result - new_result;
        pszLemmaFound = old_result;
        fLemmaFound = 1;
      }
      pnew_result++;
      pold_result++;
    }
    if (!strcmp((char *) pnew_result,(char *) pold_result)) {
      /* JH 30.08.97 the info tail was not checked before! */
      /* yes, same result, delete current result */
      hf_xresult = new_result - hf_results;
      return(0);
    }
    old_result += strlen((char *)old_result) + 1;
  }
  /* no such result computed previously */
  /* JH 1.8.97: the whole section which follows */
  if (fLemmaFound) {
    /* result not found completely the same, but lemma found: */
    /* will move rest of result */
    /* to the lemma found */
    /* !! involves moving rest of hf_results to the right to make
       space for inserted string */ 
    /* first assume that all of data should be moved back to the
       old lemma; then check pattern & comment -- if same, move tag only */
    cCharsToInsert = cAppLength - cLemmaLength; /* insert 
                        everything from HF_PATSEP_CHAR inclusive */
    cOldResKeep = strlen((char *)pszLemmaFound);
    /**/ /* fprintf(stderr,
         "dupl!! <%s> at %d, ins full %d, hf_xresult %d, lemlen %d\n",
         pszLemmaFound,pszLemmaFound-hf_results,
         cCharsToInsert,hf_xresult,cLemmaLength); /**/
    /**/ /* fprintf(stderr,
         "dupl!! (cont) new_result: <%s>\n",new_result); /**/
    /* now parameters (cOldResKeep, cCharsToInsert) set to full info insert;
       check first if only tag differs --> insert only tag, otherwise
       stick to full info insert to old res lemma */
    fTagDiffOnly = 0;
    cInsertOffset = cLemmaLength;
    pchCheckOld = (char *) pszLemmaFound + cLemmaLength; /* ptr at HF_PATSEP_CHAR */
    while (*pchCheckOld == HF_PATSEP_CHAR) { /* go through all appended
                            *full* strings -- all begin with HF_PAT_SEP */
      pchCheckNew = (char *) new_result + cLemmaLength; /* dtto */
      /* check if patterns same (end by HF_TAG_SEP): */
      while (*pchCheckOld != '\0' && *pchCheckOld != HF_TAG_SEP &&
             *pchCheckOld == *pchCheckNew) {
        pchCheckOld++;
        pchCheckNew++;
      }
      if (*pchCheckOld == '\0' ||
          *pchCheckOld == HF_PATSEP_CHAR) {
        /* should not happen: tag should always be present! */
        hf_err("tagm");
        return(1);
      }
      if (*pchCheckOld == HF_TAG_SEP && *pchCheckNew == HF_TAG_SEP) {
        /* patterns same, check comment & flags: */
	/**/ /* fprintf(stderr,"patterns found same!\n"); /**/
        /* skip tags: */
        pchTagOldBeg = pchCheckOld;
        while (*pchCheckOld != '\0' 
            && *pchCheckOld != HF_EXPL_SPACE
            && *pchCheckOld != HF_PATSEP_CHAR) {
          pchCheckOld++;
        }
        pchTagNewBeg = pchCheckNew;
        while (*pchCheckNew != '\0' 
            && *pchCheckNew != HF_EXPL_SPACE
            && *pchCheckNew != HF_PATSEP_CHAR) {
          pchCheckNew++;
        }
        pchOldTagEnd = pchCheckOld; /* ptr to HB_EOS or HF_EXPL_SPACE */
        if ((*pchCheckOld == '\0' || *pchCheckOld == HF_PATSEP_CHAR) &&
            (*pchCheckNew == '\0' || *pchCheckNew == HF_PATSEP_CHAR)) {
          /* no comment at both places, ok, difference is in tags only */
          fTagDiffOnly = 1;
        }
        else {
          /* nonempty comment at one side, at least: */
          /* check equality of comments: */
          while (*pchCheckOld != '\0' &&
                 *pchCheckOld == *pchCheckNew) {
            pchCheckOld++;
            pchCheckNew++;
          }
          if ((*pchCheckOld == '\0' || *pchCheckOld == HF_PATSEP_CHAR) &&
              (*pchCheckNew == '\0' || *pchCheckNew == HF_PATSEP_CHAR)) {
            /* ok, both ended at the same place and equal: */
            fTagDiffOnly = 1;
          }
        } /* of nonempty comment at least at one of the strings */
      } /* of: patterns same, checking comments */
      /**/ /* fprintf(stderr,"fTagDiffOnly: %d\n",fTagDiffOnly); /**/
      if (fTagDiffOnly)
        break; /* first found string counts */
      else { /* move pchCheckOld to next HF_PATSEP_CHAR */
        while (*pchCheckOld != '\0' &&
               *pchCheckOld != HF_PATSEP_CHAR) {
          pchCheckOld++;
        } /* of while moving to next data in old result */
      } /* not found, moving to next data part of old result */
    } /* of while loop over all appended full data */

    /* now see if only tag should be inserted in old result;
       otherwise, full data except lemma will be added */
          /**/ /* fprintf(stderr,"Outside searching loop: fTagDiffOnly: %d\n",
               fTagDiffOnly); /**/
    if (fTagDiffOnly) {
      /* first, check if tag(s) not already there: */
      /* if yes, shorten new result; if no tags remain, exit */
      pchCheckNew = pchTagNewBeg;
      /**/ /* fprintf(stderr,"Checking new tag(s) from: %s\n",pchCheckNew); /**/
      while (*pchCheckNew != '\0' 
          && *pchCheckNew != HF_EXPL_SPACE
          && *pchCheckNew != HF_PATSEP_CHAR) {
        pchCheckNew++; /* skip separator */
        pszTagNew = pchCheckNew; /* start of new tag for cmp */
        while (*pchCheckNew != '\0' 
            && *pchCheckNew != HF_EXPL_SPACE
            && *pchCheckNew != HF_TAG_SEP
            && *pchCheckNew != HF_TAG_ALT
            && *pchCheckNew != HF_PATSEP_CHAR) {
          pchCheckNew++;
        }
        chSave = *pchCheckNew; /* save separator */
        *pchCheckNew = '\0'; /* now in pszTagNew sz-form of 'new' tag */
        /* check if it is in the old string of tags: */
	/**/ /* fprintf(stderr,"new tag to check: %s\n",pszTagNew); /**/
        pchCheckOld = pchTagOldBeg;
        while (*pchCheckOld != '\0' 
            && *pchCheckOld != HF_EXPL_SPACE
            && *pchCheckOld != HF_PATSEP_CHAR) {
          pchCheckOld++; /* skip separator */
          pchTagNew = pszTagNew;
             /**/ /* fprintf(stderr,"checking against: %s\n",pchCheckOld); /**/
          while (*pchCheckOld != '\0' 
              && *pchCheckOld != HF_EXPL_SPACE
              && *pchCheckOld != HF_TAG_SEP
              && *pchCheckOld != HF_TAG_ALT
              && *pchCheckOld != HF_PATSEP_CHAR) {
            if (*pchTagNew == '\0' || 
              *pchTagNew != *pchCheckOld) {
                   /**/ /* fprintf(stderr,"different :(...\n"); /**/
              goto GetNextOldTag; /* not found */
	    }
            pchTagNew++;
            pchCheckOld++;
          } /* of comparing tags (end of old tag) */
          if (*pchTagNew == '\0' /* must check, new could be longer! */) {
            /* found, delete it from new_result: */
                   /**/ /* fprintf(stderr,"Found!\n"); /**/
            if (chSave != '\0' &&
                chSave != HF_EXPL_SPACE &&
                chSave != HF_PATSEP_CHAR)
              pchTagNew++; /* skip separator ptr to start of next tag
                               or rest of info string to be moved */
            pchCheckNew = pszTagNew - 1; /* skip back to separator before
                                 the first of moved tag(s)  (+ or /) */
                /**/ /* fprintf(stderr,"Moving %s over to %s (chSave: %c)\n",
                     pchTagNew,pszTagNew, chSave); /**/
            while (*pchTagNew != '\0') {
              *pszTagNew++ = *pchTagNew++; /* move it */
            }
            *pszTagNew = HB_EOS; /* bug fix: missing line 10/28/00 hajic */
              /**/ /* fprintf(stderr,"new tags after move: %s\n",
                      pchCheckNew); /**/

            if (chSave == '\0' ||
                chSave == HF_EXPL_SPACE ||
                chSave == HF_PATSEP_CHAR) {
              /* 10/28/00: delete last tag separator: */
              pszTagNew--; /* bug fix, part of */
              *pszTagNew = HB_EOS; /* bug fix: missing line 10/28/00 hajic */
              /* it was the last tag - exit check loop */
              /**/ /* fprintf(stderr,"Last new tag done....\n"); /**/ 
              if (pchTagNewBeg == pchCheckNew) { /* No tags left, no move
                                                    of anything, hf_result
                                                    unchanged */
                hf_xresult = new_result - hf_results;
                return 0;
              }
              else /* some tags left: add to 'old' result anyway */
                goto AllTagsChecked;
            } /* of last new tag processed, deleted */
            else { /* more new tags: check against all old tags again */
              goto CheckAgainAllOldTags;
            }
          }
GetNextOldTag:;
          while (*pchCheckOld != '\0' 
              && *pchCheckOld != HF_EXPL_SPACE
              && *pchCheckOld != HF_TAG_SEP
              && *pchCheckOld != HF_TAG_ALT
              && *pchCheckOld != HF_PATSEP_CHAR) {
            pchCheckOld++; /* skip rest of old tag */
          }
          /**/ /* fprintf(stderr,"next old tag to check against: %s\n",
                   pchCheckOld); /**/
        } /* of outer loop over all 'old' tags for cmp */
        *pchCheckNew = chSave;
CheckAgainAllOldTags:; /* new tag already pointed to! */
      } /* end of outer loop over 'new' tags */
AllTagsChecked:;
      /* check if any tag remained to be inserted to the new lemma: */
      /**/ /* fprintf(stderr,"All tags checked\n"); /**/
      /* insert only new tag, not the whole info string */
      /* first, find end of tags at "old" location: */
      /* start at last old tag, possibly in the middle, after last match (
         or no match) with the "new" tag(s) (pchCheckOld): */
      while (*pchCheckOld != '\0' 
          && *pchCheckOld != HF_EXPL_SPACE
          && *pchCheckOld != HF_PATSEP_CHAR) {
        pchCheckOld++; /* skip rest of old tag */
      }
      pchOldTagEnd = pchCheckOld; 
      cOldResKeep = pchOldTagEnd - (char *)pszLemmaFound; /* orig. full
                    pszLemma length -- now only up to end of tag */
      pchNewResult = (char *) new_result + cLemmaLength;
      /**/ /* fprintf(stderr,"pchNewResult: %s\n",pchNewResult); /**/
      while (*pchNewResult != '\0' &&
             *pchNewResult != HF_TAG_SEP) {
        pchNewResult++;
      }
      /**/ /* fprintf(stderr,"pchNewResult after skip pat: %s\n",
               pchNewResult); /**/
      if (*pchNewResult == '\0') {
        /* should not happen: some new tag should always be present here! */
        hf_err("tgmn");
        return(1);
      }
      pchNewResult++;
      /* pchNewResult remains pointed to tag */
      pchTagEnd = pchNewResult;
      while (*pchTagEnd != '\0' &&
             *pchTagEnd != HF_EXPL_SPACE) {
        pchTagEnd++;
      }
      /* now compute the length of new tag(s), plus one (for new sep, /) */
      cCharsToInsert = pchTagEnd - pchNewResult + 1;
      /* move the chars to temp storage: */
      if (cCharsToInsert < HB_LONGSTRINGMAX - 2) {
        strncpy(szNewTagsEtc+1,pchNewResult,cCharsToInsert-1);
        szNewTagsEtc[0] = HF_TAG_ALT; /* add alternate symb. (/) */
        szNewTagsEtc[cCharsToInsert] = HB_EOS;
	/**/ /* fprintf(stderr,"szNewTagsEtc: %s\n",szNewTagsEtc); /**/
      }
      else {
        /* too long tag string to insert... (increase szNewTagsEtc size) */
        hf_err("ntes");
        return(1);
      }
      /* ob. cInsertOffset = pchNewResult - (char *)new_result; */
      /* ob. *(pchOldTagEnd) = HF_TAG_ALT; /* replace char after tag in old
            result by tag alt (/) -- NB: might replace null char! --
            but that should not matter now */
      /**/ /* fprintf(stderr,
  "tag only par: cOldResKeep %d, cCharsToInsert %d, (new) cInsertOffset %d\n",
             cOldResKeep,cCharsToInsert,cInsertOffset); /**/
    } /* of setting parameters of tag-only insertion to old lemma */
    else { /* insert everything (only lemma matches) */
      if (cCharsToInsert < HB_LONGSTRINGMAX - 2) {
        strncpy(szNewTagsEtc, (char *)pszIn + cInsertOffset, cCharsToInsert);
        szNewTagsEtc[cCharsToInsert] = HB_EOS;
      }
      else {
        /* too long string to insert... (increase szNewTagsEtc size) */
        hf_err("ntsf");
        return(1);
      }
    }
    cCharsToMove = (new_result - pszLemmaFound) - cOldResKeep;
    /**/ /* fprintf(stderr,
         "move %d, hf_xresult %d, lemlen %d\n",
         cCharsToMove,hf_xresult,cLemmaLength); /**/
    pchCharMovingTo = (char *) new_result + cCharsToInsert - 2;
    pchCharMovingFrom = (char *) new_result - 2; /* will move final zero, too */
    *(pchCharMovingTo+1) = '\0'; /* 1st of double final zero */
    *(pchCharMovingTo+2) = '\0'; /* 2nd of double final zero */
    hf_xresult = ((Char *)pchCharMovingTo) - hf_results + 2;
    for (iCharMoving = 0; iCharMoving < cCharsToMove; iCharMoving++) {
         /**/ /* fprintf(stderr,"Moving char %c...\n",*pchCharMovingFrom); /**/
      *pchCharMovingTo-- = *pchCharMovingFrom--;
    } /* of moving something to the rigth, overlapping */
      /**/ /* ob. fprintf(stderr,"inserting: <%s>, next <%s>, hf_xresult %d\n",
        pszIn+cInsertOffset,pszLemmaFound+strlen(pszLemmaFound)+1,hf_xresult); 
	/**/
        /**/ /* fprintf(stderr,"inserting: <%s> to old offset %d\n",
         szNewTagsEtc,cOldResKeep); 
          /**/
    /*    strncpy((char *) pszLemmaFound+cOldResKeep,(char *) pszIn+cInsertOffset,cCharsToInsert); */
    strncpy((char *) pszLemmaFound+cOldResKeep, szNewTagsEtc,
                                                strlen(szNewTagsEtc));
    /**/ /* fprintf(stderr,"after insert: <%s>, next <%s>, hf_xresult %d\n",
         pszLemmaFound,pszLemmaFound+strlen(pszLemmaFound)+1,hf_xresult); /**/
  } /* of moving lemma info (pat, tag, comment, flags) to previous lemma */
  else { /* new full result appended to hf_results */
    hf_ne_results[hf_res_count] = fNeSep; /* XX_HR 4 */
    hf_res_count++;
  } /* of appending full result */
  return(0);
} /* of AppendToResults */

FUNDEF4(int
InCache,
char *, pszcDictRoot,
Char **, ppszcDataFound,
int *, pfFailCode,
char **, pprgfDicPatOk)
{
int i;
dictbufcacheRecType *pdbcT;

  for (i = 0; i < irgDictCache; i++) {
               /**/ /* fprintf(stderr,"test data cache: %d, %s, %s\n",
                i,pszcDictRoot,rgpdbcAct[i]->szcKeyCached); /**/
    if (!strcmp(pszcDictRoot,(char *) rgpdbcAct[i]->szcKeyCached)) {
      /* found */
              /**/ /*fprintf(stderr,"found dict cache: %d, %s, %s\n",
                i,pszcDictRoot,rgpdbcAct[i]->szcKeyCached); /**/
      *ppszcDataFound = rgpdbcAct[i]->pszcData;
      *pfFailCode = rgpdbcAct[i]->fFail;
      *pprgfDicPatOk = rgpdbcAct[i]->rgfDicPatOk;
      if (i != 0) {
        pdbcT = rgpdbcAct[0];
        rgpdbcAct[0] = rgpdbcAct[i];
        rgpdbcAct[i] = pdbcT;
      }
      return 1;
    }
  }
  return 0;
} /* of InCache */

FUNDEF4(int
PutInCache,
char *, pszcDictRoot,
Char **, ppszcDataFound,
int, fFailCode,
char **, pprgfDicPatOk)
{
int i;
int iBufferOffset;
char szT[HB_STRINGMAX];

  /* first test if free space in cache: */
  if (irgDictCache >= crgDictCache) {
    /* if not, do not cache, but signal problem as then rrAct cannot
       be used -- would give less results or even wrong results !! */
    hf_inv_cvs((Char *) pszcDictRoot,(Char *) szT);
    strcat(szT," dbsm");
    hf_err(szT);
    return 666;
  }

  /* put buffer and pointer to data into cache record */
  rgpdbcAct[irgDictCache]->fFail = fFailCode;
  iBufferOffset = (*ppszcDataFound) - hf_dbuffer;
  if (*ppszcDataFound == NULL) {
    rgpdbcAct[irgDictCache]->pszcData = NULL;
  }
  else { 
    memcpy(rgpdbcAct[irgDictCache]->szBuffer,(*ppszcDataFound),
           hf_ddc + 1/* just to be sure */ - iBufferOffset);
    rgpdbcAct[irgDictCache]->pszcData = rgpdbcAct[irgDictCache]->szBuffer;
  }

          /**/ /* fprintf(stderr,
             "PutInCache (%d) offset (in: hf_dbuffer): %d\n",
             irgDictCache,(*ppszcDataFound)-hf_dbuffer);
             fprintf(stderr,
               "rgpdbcAct[irg...]->pszcData-rgpdbcAct[irg...]->szBuffer: %d\n",
               rgpdbcAct[irgDictCache]->pszcData
               - rgpdbcAct[irgDictCache]->szBuffer);
                /**/

  strcpy((char *) rgpdbcAct[irgDictCache]->szcKeyCached,pszcDictRoot);
  /* and return address of array where dic pattern flags should
     be set: */
  *pprgfDicPatOk = rgpdbcAct[irgDictCache]->rgfDicPatOk;
  /* and initialize it to all 0: */
  memset(rgpdbcAct[irgDictCache]->rgfDicPatOk,0,sizeof(char)*crgDictPat);

         /**/ /*
         for (i = 0; i < crgDictPat; i++) {
           if (rgpdbcAct[irgDictCache]->rgfDicPatOk[i] != 0) {
             fprintf(stderr,"memset err!!!!!!!!! %d\n",i);
             exit(1222);
           }
         }
         /**/
  /* change parm. val! to point into cache buffer, not dict data buffer! */
  *ppszcDataFound = rgpdbcAct[irgDictCache]->pszcData;
  irgDictCache++;
  return 0;
} /* of PutInCache */

FUNDEF3(int
OutputResultRecord,
resultRecType *, prgrrAct, /* active parsed result records */
int, crrMax, /* size */
int, fOutFlag)
{
int irrAct;
Char szSingleResult[HF_MAXBUILD]; /* single result as string, used as output
                                      from BuildSingleResult */
resultRecType *prrAct;

  for (irrAct = 0; irrAct < crrMax; irrAct++) {
    prrAct = &(prgrrAct[irrAct]);
    if (BuildSingleResult(szSingleResult,prrAct,fOutFlag)
        != 0) {
      hf_err("bsrq");
      return 503;
    }
           /**/ /* fprintf(stderr,"szSingleResult: <%d> %s;\n",
              strlen(szSingleResult),
              szSingleResult); /**/
    if (AppendToResults(szSingleResult,
                        prrAct->fNe) != 0) {
      hf_err("asrq");
      return 504;
    } 
  } /* of loop over all rr */
  return 0;
} /* of OutputResultRecord */

FUNDEF6(int
hf_LemmaSingle,
plangEnvRec, plangEnv,
Char *, s,
int, dot,
int, hyph,
int *, pcnt,
int, fOutFlag)
{
/* morphology routine;
 * strips prefixes (ne, nej, nejne)
 * calls dict search
 * calls endings search
 * checks classes
 * makes results
 *
 * supposes s length < 32
 *
 */

int iRC;

Char *next_data;
Char s_coded[HF_MAX_ROOT_LEN];
Char *js_coded;
Char *ns_coded;
Char *jns_coded;
Char re_coded[HF_MAX_ROOT_LEN];

/* JH 26.11.1994 */
longint iTag;
int cCodedLength;

int xe;
int ls,le;
int csl; /* neg values used here */
int fail_type;

Char *pszcCurrentRoot;
int iStemLoop; /* 0..3 for all roots (ne/nej/nejne) */
Char szCurrentRoot[HB_STRINGMAX]; /* debug printing */
int cCurrentRootLen; /* length of full current root */
int cRootActLen; /* length of active root, ending data */
int cDictRootActLen; /* length of active root, dictionary data */
int iEndLen; /* active ending length */
Char *pchEnd1st; /* ending data, ptr only to hf_endlist */
Char *pchEnd; /* current pattern number of an ending */
int fFailCode; /* fail code type from hf_[ed]search */
               /* signal either not found or longer cannot be found */
Char *pszcDataFound; /* dict data found */
Char *pszcPrevDataFound; /* previous dict data found */
Char *pszcNextData; /* next dict data */
Char chSave; /* saving char replaced by HF_EOS to get sz string */
int iEndPat; /* active pattern for orig stem re-creation */
int iDistDataIndex; /* index to rgDictPat & rgDistData arrays */
int iDictPat; /* dictionary pattern to match the dictionary */
int iDistData; /* ptr to rootchg/rule space */
int iPrevDistData; /* ptr to rootchg/rule space, 
                      used in prev. pass (-1 if 1st pass) */
Char *pszcRootChange; /* string, root derivational change */
Char pszRootChange[HF_MAX_ROOT_LEN];
                     /* string, root derivational change (coded) */
Char *pszDistRule; /* string, lemma & synonym derivation rule */
Char szcDictRoot[HF_MAX_ROOT_LEN]; /* re-created real dictionary root */
Char *pszRealData; /* real data ptr (rwr/actual data) */
int iStemClass; /* dict pat of stem item from dict */
Char szLemmaOut[HB_STRINGMAX]; /* debug only */
Char szDictLemma[HB_STRINGMAX]; /* debug */
char *prgfDicPatOkAct; /* ptr to flag array of o.k. pats from dict */
char fDataFromCache; /* flag */
int iPrevEndPat; /* remembering previous ending pattern in loop over endings */
int fNeSeparated; /* this function local falg */
int fNejSeparated; /* this function local falg */
resultRecType *prrAct; /* active parsed result record */
int irrAct; /* index when creating results from previously stored parsed
                       result records */
int crrAct; /* current 'size' of rrAct, or ptr to free space */
Char *pszcDictRoot;
Char chcInputRootFinal; /* for speedup */

char szDouble[16]; /* double prefix in correct order */
int cDouble; /* length of double prefix */
/* 2002/10/19 hajic change to reflect the language stored in data (dict) */
/* strings in variables set from lang in data not coded! */

  hf_cvs(s,s_coded);
  ns_coded = NULL;
  js_coded = NULL;
  jns_coded = NULL;

  /****/ /* fprintf(stderr,"grad: %s, neg: %s\n",
		 plangEnv->GRAD_PREFIX,plangEnv->NEG_PREFIX); /**/

  if (plangEnv->fCZ == 1) { /* Czech; do i fast but dirty... */
    if ((*s == 'n' || *s == 'N') &&
      (s[1] == 'e' || s[1] == 'E')) {
      ns_coded = s_coded+2;
      if (s[2] == 'j' || s[2] == 'J') {
        js_coded = s_coded+3;
        if ((s[3] == 'n' || s[3] == 'N') &&
          (s[4] == 'e' || s[4] == 'E')) {
            jns_coded = s_coded+5;
          } /* of 'nejne' */
      } /* of 'nej' */
    } /* of 'ne' */
  } /* of Czech */
  else { /* not Czech */
    /* do negative prefix first: */
    if (plangEnv->cNEG_PREFIX > 0) { /* prefix exists */
      if (!strncmp((char *)s,plangEnv->NEG_PREFIX,plangEnv->cNEG_PREFIX)) {
        ns_coded = s_coded + plangEnv->cNEG_PREFIX;
      }
      if (plangEnv->cGRAD_PREFIX > 0) { /* the other prefix exists, too */
        cDouble = plangEnv->cNEG_PREFIX + plangEnv->cGRAD_PREFIX; /* length */
        if (plangEnv->fNegFirst == 1) { /* negation first (left) */
	  strcpy(szDouble,plangEnv->NEG_PREFIX);
	  strcat(szDouble,plangEnv->GRAD_PREFIX);
	}
	else { /* degrees of comp first (left) */
	  strcpy(szDouble,plangEnv->GRAD_PREFIX);
	  strcat(szDouble,plangEnv->NEG_PREFIX);
	}
        if (!strncmp((char *)s,szDouble,cDouble)) { /* both prefixes found */
          jns_coded = s_coded + plangEnv->cGRAD_PREFIX;
        }
      } /* eo grad prefix inside neg. prefix testing */
    } /* eo neg prefix testing */
    if (plangEnv->cGRAD_PREFIX > 0) { /* prefix exists */
      if (!strncmp((char *)s,plangEnv->GRAD_PREFIX,plangEnv->cGRAD_PREFIX)) {
        js_coded = s_coded + plangEnv->cGRAD_PREFIX;
      }
    } /* eo grad prefix testing */
  } /* of else.. not Czech */

  /****/ /* fprintf(stderr,"word: %s:",s);
  if (ns_coded != NULL) { fprintf(stderr," NEG"); }
  if (js_coded != NULL) { fprintf(stderr," GRAD"); }
  if (jns_coded != NULL) { fprintf(stderr," BOTH"); } 
  fprintf(stderr,"\n"); /**/


  /* find endings: */
  hf_reverse(re_coded,s_coded); /* sorted in reverse order */

  next_data = re_coded;
  while (*next_data) { /* test endings only as lowercase to */
          /* handle nicely mixed strings: TEMPUSEM */
          /* needed if only uppercase in dict: TEMPUS */
    
    if (UpperCaseCode(*next_data)) { /* uppercase */
      *next_data += 1;
    }
    next_data++;
  }

  le = ls = strlen((char *)s_coded); /* max. length of stem == max. length */
                /* of ending + 1 (zero stem never tested */
  for (xe = 1; xe < le; xe++) {
      /* index zero: filled in advance by "0" ending in hf_init */
    if (xe >= hf_ekeymax) break; /* max. ending length ever possible */
            /* determined at compile time from ending tables */
    /* 01/09/01 JH: fixing '0' ending bug */
    if (xe == 1 && re_coded[0] == cCoded0) { /* single '0' symbol in
             input string, do not search for it in endings, 
             since it is used there (in endings) for empty string;
             but, e.g. 00 or 30 as ending is ok (length > 1) */
      continue;
    }
    chSave = re_coded[xe];
    re_coded[xe] = 0;
      /* make re_coded shorter (temporarily), length == xe */
    hf_esearch(re_coded,hf_elist+xe,&fail_type);
      /* find ending, put result (if any) to the hf_elist array */
      /* test result only by fail_type */
      /* cannot return nonzero as no file accessed in hf_find_index */
      /* keeping pointer is enough as endings are always in memory 
         all the time JH 29.08.97 */
    re_coded[xe] = chSave;
      /* restore full ending */
    if (fail_type) {
      xe++; /* simulate exit by too big xe */
      break; /* no longer ending can be found */
    }
  } /* of for xe */
  hf_xelist = xe-1; /* save (possibly) longest ending found */

  for (iStemLoop = 0; iStemLoop < 4; iStemLoop++) {
    pszcCurrentRoot = NULL;
    switch(iStemLoop) {
      case 0: fNeSeparated = 0; fNejSeparated = 0;
              if (s_coded != NULL) {
                if (strcmp((char *) s_coded,(char *) szPrevRootS)) {
                  pszcCurrentRoot = s_coded;  
                  strcpy((char *) szPrevRootS,(char *) s_coded);
                }
              }
              break;
      case 1: fNeSeparated = 0; fNejSeparated = 1;
              if (js_coded != NULL) {
                if (strcmp((char *) js_coded,(char *) szPrevRootJs)) {
                  pszcCurrentRoot = js_coded;  
                  strcpy((char *) szPrevRootJs,(char *) js_coded);
                }
              }
              break;
      case 2: fNeSeparated = 1; fNejSeparated = 0;
              if (ns_coded != NULL) {
                if (strcmp((char *) ns_coded,(char *) szPrevRootNs)) {
                  pszcCurrentRoot = ns_coded;  
                  strcpy((char *) szPrevRootNs,(char *) ns_coded);
                }
              }
              break;
      case 3: fNeSeparated = 1; fNejSeparated = 1;
              if (jns_coded != NULL) {
                if (strcmp((char *) jns_coded,(char *) szPrevRootJns)) {
                  pszcCurrentRoot = jns_coded;  
                  strcpy((char *) szPrevRootJns,(char *) jns_coded);
                }
              }
              break;
    }
    if (pszcCurrentRoot == NULL) continue; /* stem not set up */
    
    /**/ /**** hf_inv_cvs(pszcCurrentRoot,szCurrentRoot); /**/
    /**/ /**** fprintf(stderr,"Doing: ne: %d, nej: %d, stem: %s\n",
                   fNeSeparated,fNejSeparated,szCurrentRoot); /**/
    cCurrentRootLen = strlen((char *) pszcCurrentRoot); /* full root (max) length */

    cRootActLen = cCurrentRootLen - hf_xelist; /* shortest stem investigated */
    iEndLen = hf_xelist;
    if (cRootActLen < 1) { /* must start later in stem */
      iEndLen -= 1 - cRootActLen;
      cRootActLen = 1;
    }

    irgDictCache = 0;
    for (; iEndLen >= 0; iEndLen--, cRootActLen++) {
      if ((pchEnd1st = hf_elist[iEndLen]) == NULL) continue; /* no ending
                          of given length */
      chSave = pszcCurrentRoot[cRootActLen];
      chcInputRootFinal = pszcCurrentRoot[cRootActLen-1];
      pszcCurrentRoot[cRootActLen] = HF_EOS;

      /**/ /**** hf_inv_cvs(pszcCurrentRoot,szCurrentRoot); /**/
      /**/ /**** fprintf(stderr,"before stem change; stem: %d/%s, end len: %d\n",
               cRootActLen,szCurrentRoot,iEndLen); /**/
      pchEnd = pchEnd1st;
      iPrevEndPat = -1; /* for the first pass: valid pattern may be zero! */
      crrAct = 0;
      while (*pchEnd >= HF_FIRST_CHAR) {
        iEndPat = *pchEnd - HF_FIRST_CHAR;
        cCodedLength = hf_DecodeXData(&iTag,(char *) pchEnd+1);
	/**/ /**** fprintf(stderr,"act end pat: %d/%s, tag # %d/%s\n",
            iEndPat,rgpszPat[iEndPat],iTag,pszTag[iTag]); /**/
        /* fprintf(stderr,"iTag o %d ",iTag); */
        /* if current class is NejNo, length == 0 and iTag -2 */
        /* we should call XDecode in the next comparison,
           but for effectivity, this is ok unless the
           number of (ending-based) classes increases over 214 */
        if ((iEndPat >= cNegFirst || !fNeSeparated)
                /* if fNe set, must be pattern allowing negation */
            && /* may test superlative-allowing property of tag here now: */
            (!fNejSeparated ||
             (iTag >= 0 && *(pszTag[iTag]) == HF_GRAD_TAGC))
           ) {
          /* see if not same end pat as before -- result in rrAct already: */
          /* this will save a lot if all tags for end pat are together in
             a row in the endings file */
          if (iEndPat == iPrevEndPat) { /* yes, difference in tag only */
            /* for all result records created in the previous pass,
               add the current tag: */
            /**/ /* fprintf(stderr,
                   "SSS same endpat: %d, crrAct = %d, tag: %d\n",
                             iEndPat,crrAct,iTag); /**/
            for (irrAct = 0; irrAct < crrAct; irrAct++) {
              prrAct = &(rrAct[irrAct]);
              prrAct->rgiEndTag[prrAct->cEndTag++] = iTag;
            } /* of for all rrAct */
          } /* of same as previous pass, diff in tag */
          else { /* not same as last time -- must output rrAct,
                    and work anew... */
            /**/ /**** fprintf(stderr,
                   "WWW new endpat: %d, iPrevEndPat %d, crrAct = %d\n",
                             iEndPat,iPrevEndPat,crrAct); /**/
            if (iPrevEndPat != -1) {
              if (OutputResultRecord(rrAct,crrAct,fOutFlag) > 0) {
                return 701;
              }
            }
            iPrevEndPat = iEndPat; /* remember active pattern number */
            crrAct = 0; /* init array of result records */
            /* now must get all possible original stems from the distribution
               (regular derivation) data */
            iPrevDistData = -1;
            for (iDistDataIndex = rgDistDataIndex[iEndPat];
                 iDistDataIndex < rgDistDataIndex[iEndPat+1];
                 iDistDataIndex++) {
              iDictPat = rgDictPat[iDistDataIndex];
              iDistData = rgDistData[iDistDataIndex];

                /**/ /**** fprintf(stderr,
                 "iDistDataIndex: %d, prev rule: %d, this rule offset: %d\n",
                 iDistDataIndex,iPrevDistData,iDistData); /**/

              if (iPrevDistData == iDistData) { /* ptr same, thus rule same */
                /* keep pszcDictRoot, also data from dict must have been
                   cached, so set fDataFromCache (to check
                   array of dict pattern possibly ok), and restore 
                   pszcDataFound to previous value -- then go directly
                   to pattern test */
                fDataFromCache = 1;
                pszcDataFound = pszcPrevDataFound;
                goto CheckPats;
	      }
              iPrevDistData = iDistData;

              pszcRootChange = (Char *) szDistDataSpace + iDistData;
              pszDistRule = pszcRootChange + strlen((char *) pszcRootChange) + 1;
                  /**/ /**** hf_inv_cvs(pszcRootChange,pszRootChange);
                      fprintf(stderr,
                      "bef rdt: dictpat %d/%s, root chg %s, rule %s\n",
                      iDictPat,rgpszDictPat[iDictPat],pszRootChange,
                      pszDistRule); /**/
              pszcDictRoot = NULL;
              /* speedup: */
              if (*pszcRootChange == cCoded0) { 
                pszcDictRoot = pszcCurrentRoot;
              }
              else { /* not zero change */
                /* for speedup, test final char first: */
                /* even before calling the root change procedure */
                if (*(pszDistRule-2) == chcInputRootFinal &&
                    ReverseRootChange(pszcCurrentRoot,pszcRootChange,
                                      szcDictRoot) > 0) {
                  pszcDictRoot = szcDictRoot;
                }
              }
              fDataFromCache = 0;
CheckPats:;
              if (pszcDictRoot != NULL) {
                /* reverse chg possible: changed root returned in DictRoot */
    /**/ /**** hf_inv_cvs(pszcDictRoot,szCurrentRoot); /**/
    /**/ /**** fprintf(stderr,
            "rule match: prev data %d, dictpat %d/%s, root chg %s, rule %s\n",
              fDataFromCache,
               iDictPat,rgpszDictPat[iDictPat],pszRootChange,pszDistRule); /**/
    /**/ /**** fprintf(stderr,
                 "will search for dict root: %s\n",szCurrentRoot); /**/
  
                /* search dict root in dictionary: DICT @ DICT @ SEARCH */
                if (fDataFromCache == 0) { /* this rule differs from prev */
                  fDataFromCache = 1;
                  if (InCache((char *) pszcDictRoot,&pszcDataFound,&fFailCode,
                              &prgfDicPatOkAct) == 0)  {
                    /* not in cache */
                    hf_dsearch(pszcDictRoot,&pszcDataFound,&fFailCode);
                    if ((iRC = PutInCache((char *) pszcDictRoot,
                              &pszcDataFound,fFailCode,
                              &prgfDicPatOkAct)) > 0) return 10000+iRC;
                    fDataFromCache = 0;
                  }
                       /**/ /* fprintf(stderr,
                           "fData..., fFail from cache: %d, %d\n",
                           fDataFromCache,fFailCode); /**/  
                  if (fFailCode != 0) {
                    /* not found, & longer can't be found */
                           /**/ /* fprintf(stderr,"NOT FOUND ===\n"); /**/
                    pszcDataFound = NULL;
                  }
                    /**/ /*           else fprintf(stderr,
                    "... FOUND at least one dict record for this stem\n"); /**/
                  /* check if for current iDictPat as returned from
                     creating dict root lemma any match in cached data: */
                  pszcPrevDataFound = pszcDataFound;
                    /* remember start of data */
                } /* end of new rule different from previous */
                if (fDataFromCache) {
                  /**/ /* fprintf(stderr,"CCC Found in cache\n"); /**/
                  if (prgfDicPatOkAct[iDictPat] == 0) { 
                      /* no, will never match pattern */
                    /**/ /* fprintf(stderr,
                       "xxx: no, will not match dic pat %d\n",iDictPat); /**/
                    pszcDataFound = NULL;
                  }
                  /**/ /* else {
                    fprintf(stderr, 
                           "yyy: will match the dic pat %d\n",iDictPat);
                  } /**/
                } /* of dict data cache used */
                /**/ /* else fprintf(stderr,"NNN NOT Found in cache\n"); /**/
  
                cDictRootActLen = strlen((char *) pszcDictRoot);
                            /* save dict root length */
                while (pszcDataFound != NULL) {
                  /* check against iDictPat */
                  /* and check if more data for the same stem!! */
                  /* regardless of if success */
                  if (*pszcDataFound == HF_FIRST_CHAR - 1) {
                    /* data space contains pointer to hf_rwr_space */
                    pszRealData = hf_rwr_space + pszcDataFound[1];
                  }
                  else
                    pszRealData = pszcDataFound;
                  /* in pszRealData now: address of data string, be it */
                  /* directly in data space, or in common hf_rwr_space */
                  iStemClass = pszRealData[1];
    /**/ /* fprintf(stderr,"iStemClass (from dict:) %d\n",iStemClass); /**/
                  /* set it in dict cache (do not have to check if cache used
                     this pass, but it would not hurt, either; it is set): */
                  /* actually, should check: 
                     if (fDataFromCache && prgfDicPatOkAct[iStemClass] != 1) {
                       hf_err("face");
                       exit(9991);
                     }
                  */
                  prgfDicPatOkAct[iStemClass] = 1;
                  if (iStemClass == iDictPat) { /* nej- separability already
                                                   checked ok */
                    if ((dot  || iStemClass != 1) &&
                        (hyph || iStemClass != 2)) {
                      /* found! match! */
                      /* prepare new results record: */
                      if (crrAct >= HF_MAX_RESULT_REC) {
                        hf_err("mrre");
                        exit(165);
                      }
                      prrAct = &(rrAct[crrAct]);
                      prrAct->fNe = fNeSeparated;
                      prrAct->fNej = fNejSeparated;
                      prrAct->cEndTag = 0;
                      prrAct->iEndingLen = iEndLen;
                      crrAct++; /* prepare for the next time - also this is
                                   the number of used (active) records */
                      /* first, construct dict lemma: */
                      GetDictLemma(pszcDictRoot,&pszRealData,prrAct);
    
    /**/ /* hf_inv_cvs(prrAct->szcLemmaDict,szDictLemma); /**/
    /**/ /* fprintf(stderr,"dict lemma constructed: %s\n",szDictLemma); /**/
    /**/ /* hf_inv_cvs(prrAct->szcLemmaDictID,szDictLemma); /**/
    /**/ /* fprintf(stderr,"dict lemma ID: %s\n",szDictLemma); /**/
  
                      ConstructLemma(pszcDictRoot,(char *) pszDistRule,prrAct);
  
    /**/ /* hf_inv_cvs(prrAct->szcLemmaDerived,szLemmaOut); /**/
    /**/ /* fprintf(stderr,"lemma constructed: <%s>\n",szLemmaOut); /**/
    /**/ /* hf_inv_cvs(prrAct->szcLemmaDerivedID,szLemmaOut); /**/
    /**/ /* fprintf(stderr,"lemma deriv ID: <%s>\n",szLemmaOut); /**/
    /**/ /* hf_inv_cvs(prrAct->szcLemmaDerivedInfo,szLemmaOut); /**/
    /**/ /* fprintf(stderr,"lemma deriv Info: <%s>\n",szLemmaOut); /**/
    /**/ /* fprintf(stderr,"real data: %c%c%c%c...\n",hh_inv_tab[*pszRealData]
              ,hh_inv_tab[pszRealData[1]]
              ,hh_inv_tab[pszRealData[2]]
              ,hh_inv_tab[pszRealData[3]]); /**/

                      prrAct->pszcData = pszRealData;
                      prrAct->iEndPattern = iEndPat;
                      prrAct->iDictPattern = iStemClass;
                      prrAct->rgiEndTag[prrAct->cEndTag++] = iTag;
                    } /* of FOUND */
                  } /* of patterns match */
                  /* pszcDataFound points still to orig data space buffer: */
                  pszcNextData = pszcDataFound + 2;
                  pszcDataFound = NULL;
                  while (*pszcNextData >= HF_FIRST_CHAR) {
                    pszcNextData++;
                  }
                  if ((*pszcNextData == cDictRootActLen) ||
                      ((*pszcNextData == HF_FIRST_CHAR - 2) &&
                       (*(pszcNextData+1) == cDictRootActLen))) {
                    /* same lgt */
                    if (*pszcNextData == HF_FIRST_CHAR - 2) {
                      pszcNextData++;
		      /*** 
           fprintf(stderr,"long key, possibly dupl key %s found\n",s); /**/
                    }
                    pszcNextData++; /* ptr to add-to-length now */
                    if (*pszcNextData < HF_FIRST_CHAR) {
                      pszcDataFound = pszcNextData; /* nothing to add */
                      /* --> it is the same key! */
		      /*** if (*(pszcNextData-2) == HF_FIRST_CHAR - 2) {
                        fprintf(stderr,"long same key, dupl key %s found\n",s);
		      } /**/
                    }
                  }
                } /* of while same key */
              } /* of reverse change of root done */
              /* else get next ending pattern number and try again */
            } /* of loop over all reverse distribution items
               in rgDictPat & rgDistData arrays */
          } /* of not same as last time */
        } /* of negation prefix separation check ok */
      pchEnd += cCodedLength + 1;
      } /* of loop over all patterns of current ending */
      if (iPrevEndPat != -1 && crrAct > 0) {
        /* some result(s) remaining to be ouput */
        if (OutputResultRecord(rrAct,crrAct,fOutFlag) > 0) {
          return 702;
        }
      }
      pszcCurrentRoot[cRootActLen] = chSave;
    } /* of loop over all stem|ending cuts possible */
  } /* of loop for all (<= 4) stems */
  *pcnt = hf_res_count;
  return(0);
} /* of hf_LemmaSingle */

int
hf_init(path,pszCharSet)
char *path;
char *pszCharSet;
{
unsigned int buflgt;
unsigned int i;
unsigned int ui;
unsigned int uIndex;
unsigned int int_read;
longint iCompOffset;
Longint cont_lgt;
Longint offset;
unsigned int max_keylgt;
unsigned int max_bufneed;
int fail_type;
char dfn[200];
char efn[200];
char buffer[9]; /* JK 17.01.1993 */
longint il;
char *psz;
int iTabError;
int iVersion;

  /* set input/output table: */
  /* JH 22.11.97 */ /* hh_fill_inv(); */ /* from hh_cp895.h */
  if ((iTabError = hhSetTabFillInv(pszCharSet)) > 0) { /* now from hh_cp.h */
    hf_err("itab");
    return(112);
  }  

  /*************************************************** dictionary *******/

  strcpy((char *) dfn,(char *) path);
  strcat((char *) dfn,(char *) HF_DCPD);
#if defined(unix)
  if ((hf_cpd_dict = open(dfn, O_RDONLY)) < 0) {  /* JK 17.01.1993 */
#else
#if defined(_Windows) || defined(_WINDOWS)
  if ((hf_cpd_dict = _open(dfn, _O_BINARY | _O_RDONLY, _S_IREAD)) < 0) { 	/* JK 17.01.1993 */
#else
#ifdef __DJGPP__
  if ((hf_cpd_dict = open(dfn, O_BINARY|O_RDONLY, S_IRUSR)) < 0) {  /* OC 02.10.1997 */
#else
  if ((hf_cpd_dict = open(dfn, O_BINARY|O_RDONLY|SH_DENYNO, S_IREAD)) < 0) {  /* JK 17.01.1993 */
#endif
#endif
#endif
    hf_err("odic");
    return(1);
  }
  strcpy((char *) efn,(char *) path);
  strcat((char *) efn,(char *) HF_ECPD);
#if defined(unix)
  if ((hf_cpd_ending = open(efn, O_RDONLY)) < 0) {  /* JK 17.01.1993 */
#else
#if defined(_Windows) || defined(_WINDOWS)
  if ((hf_cpd_ending = _open(efn, _O_BINARY | _O_RDONLY, _S_IREAD)) < 0) { 	/* JK 17.01.1993 */
#else
#ifdef __DJGPP__
  if ((hf_cpd_ending = open(efn, O_BINARY|O_RDONLY, S_IRUSR)) < 0) {  /* OC 02.10.1997 */
#else
  if ((hf_cpd_ending = open(efn, O_BINARY|O_RDONLY|SH_DENYNO, S_IREAD)) < 0) {  /* JK 17.01.1993 */
#endif
#endif
#endif
    hf_err("oend");
    return(2);
  }
  read(hf_cpd_dict, buffer, 4);      /* hajic 09.07.2001 */
  sscanf(buffer,"%04x",&iVersion);   
  if (iVersion != DICVERSION) {
    hf_err("dver");
    return(28); 
  }
  read(hf_cpd_dict, buffer, 4);      /* hajic 09.07.2001 */
  read(hf_cpd_dict, buffer, 8);      /* JK 17.01.1993 */
#if !defined(__alpha)
  sscanf(buffer,"%08lx",&cont_lgt);   /* JK 17.01.1993 */
#else
  sscanf(buffer,"%08x",&cont_lgt);   /* JK 17.01.1993 */
#endif
  read(hf_cpd_dict, buffer, 4);       /* JK 17.01.1993 */
  sscanf(buffer,"%04x",&hf_ddc);      /* JK 17.01.1993, JH 03.02.1994 */
  read(hf_cpd_dict, buffer, 4);       /* JK 17.01.1993 */
  sscanf(buffer,"%04x",&hf_dsx);      /* JK 17.01.1993 */
  read(hf_cpd_dict, buffer, 4);       /* JK 17.01.1993 */
  sscanf(buffer,"%04x",&hf_dxx);      /* JK 17.01.1993 */
  read(hf_cpd_dict, buffer, 4);       /* JK 17.01.1993 */
  sscanf(buffer,"%04x",&max_keylgt);  /* JK 17.01.1993 */
  read(hf_cpd_dict, buffer, 4);       /* JK 17.01.1993 */
  sscanf(buffer,"%04x",&max_bufneed); /* JK 17.01.1993 */

/* JH 03.02.1994 tests eliminated
  if (hf_ddc >= HF_DMAX_BUF ||
    hf_dsx >= HF_DX_SP ||
    max_keylgt >= HF_DKEYMAX ||
    max_bufneed >= HF_DBUFNEED ||
    cont_lgt >= HF_DSIZE ||
    hf_dxx >= HF_DX_MAX) {
    hf_err("hf_mod.c needs recompilation");
    return(3);
  }
*/

/* JH 03.02.1994 BEG */
  /* memory allocation based on actual values from the compiled file */
  if ((hf_dbuffer = (Char *)
                    alloc(hf_ddc+100)) == NULL) {
    hf_err("aldb");
    return(20);
  }
  if ((hf_dx_index = (Longint *)
                     alloc(sizeof(Longint*)*(hf_dxx+4))) == NULL) {
    hf_err("aldx");
    return(20);
  }
  if ((hf_dx_space = (Char *)
                     alloc(hf_dsx+100)) == NULL) {
    hf_err("alds");
    return(20);
  }
  if ((hf_dx_space_index = (Char **)
                     alloc(sizeof(Char *)*(hf_dxx+4))) == NULL) {
    hf_err("alds");
    return(20);
  }
/* JH 03.02.1994 END */


  if (lseek(hf_cpd_dict,cont_lgt,SEEK_SET) < 0) { /* JK 17.01.1993 */
    hf_err("seei"); /* beg of index string space */
    return(9);
  }
  if (read(hf_cpd_dict,hf_dx_space,hf_dsx) != hf_dsx) { /* JK 17.01.1993 */
    return(4);
  }
  hf_dx_space[hf_dsx++] = 0;
  psz=(char *) hf_dx_space;
  for (i = 0; i < hf_dxx; i++) {
    hf_dx_space_index[i]=(Char *)psz;
    psz+=strlen((char *) psz)+1;
  }
  offset = 0;
  for (i = 0; i < hf_dxx; i++) {
    if (read(hf_cpd_dict, buffer, 4) != 4) { /* JK 17.01.1993 */
      hf_err("itdf");
      return(5);
    }
    sscanf(buffer,"%04x",&buflgt); /* JK 17.01.1993 */
    hf_dx_index[i] = (offset += buflgt);
  }
  hf_dboff = 0; /* buffer is empty */
  if (read(hf_cpd_dict, buffer, 2) != 2) {  /* JK 17.01.1993 */
    hf_err("rwrd");
    return(13);
  }
  sscanf(buffer,"%02x",&hf_sz_rwr_space); /* JK 17.01.1993 */
  for (i = 0; i < hf_sz_rwr_space; i++) {
    if (read(hf_cpd_dict, buffer, 2) != 2) {  /* JK 17.01.1993 */
      hf_err("rwrc");
      return(14);
    }
    sscanf(buffer,"%02x",&int_read); /* JK 17.01.1993 */
    hf_rwr_space[i] = int_read;
  }
  /* added JH 29.08.97 BEG */
  /* read patterns from dict file -- now different from endings file! */
  if (read(hf_cpd_dict, buffer, 2) != 2) {
    hf_err("rlan");
    return(140);
  }
  /* get language from read buffer (2 byte abbr only): */
  szLangDict[0] = buffer[0];
  szLangDict[1] = buffer[1];
  szLangDict[2] = HF_EOS;
 /**/ /* fprintf(stderr,"### lang dict read in: <%s>\n",szLangDict); /**/
  /* now really get the pattern strings: */
  /* size of string area first: */  
  if (read(hf_cpd_dict, buffer, 8) != 8) {
    hf_err("rpas");
    return(141);
  }
#if !defined(__alpha)
  sscanf(buffer,"%08lx",&cszDictPatSpace);
#else
  sscanf(buffer,"%08x",&cszDictPatSpace);
#endif
#if !defined(unix)
  if (cszDictPatSpace*sizeof(char) > 65500u) {
    hf_err("dosa");
    exit(143);
  }
#endif
  /* allocate space for strings: */
  ui = cszDictPatSpace + 2;
  if ((szDictPatSpace = (char *)
                    alloc(ui*sizeof(char))) == NULL) {
    hf_err("aldp");
    return(142);
  }

  /* now read the pattern names: (separated by tab) */
  crgDictPat = 0l;
  for (il = 0l; il < cszDictPatSpace; il++) {
    if (read(hf_cpd_dict, buffer, 1) != 1) {
      hf_err("rpat");
      return(144);
    }
    if (*buffer == '\t' || *buffer == '\n' || *buffer == ' ') {
      crgDictPat++;
      *buffer = HF_EOS;
    }
    szDictPatSpace[il] = *buffer;
  }
  szDictPatSpace[il++] = HF_EOS;
  szDictPatSpace[il] = HF_EOS;
  /* allocate space for dict pat strings: */
  ui = crgDictPat * sizeof(char *);
#if !defined(unix)
  if (ui > 65500u) {
    hf_err("dost");
    return(147);
  }
#endif
  if ((rgpszDictPat = (char **)
                    alloc(ui)) == NULL) {
    hf_err("aldt");
    return(148);
  }
  /* now fill in the index: */
  psz = szDictPatSpace;
  ui = 0;
  while (*psz != HF_EOS) {
    /* printf("indexing dict pat string %u: %s\n",ui,psz); */
    rgpszDictPat[ui++] = psz;
    /* NB: pat strings are never coded */
    while (*psz++ != HF_EOS); /* ignore the string */
  } /* of indexing pat strings */
  
/**/ /* fprintf(stderr,"dict pats: %ld, size %ld, [0] = %s, [1] = %s...\n",
                  crgDictPat,cszDictPatSpace,szDictPatSpace,
                  szDictPatSpace + strlen(szDictPatSpace) + 1); /**/
  if (read(hf_cpd_dict, buffer, 8) != 8) {
    hf_err("rmdk");
    return(149);
  }
#if !defined(__alpha)
  sscanf(buffer,"%08lx",&cMaxDataForKey);
#else
  sscanf(buffer,"%08x",&cMaxDataForKey);
#endif
/**/ /* fprintf(stderr,"cMaxDataForKey = %ld\n",cMaxDataForKey); /**/

  if ((rgpdbcAct = (dictbufcacheRecType **) 
                  alloc(sizeof(dictbufcacheRecType *)*crgDictCache)) == NULL) {
    hf_err("rmda");
    return(150);
  }
  for (irgDictCache = 0; irgDictCache < crgDictCache; irgDictCache++) {
    if ((rgpdbcAct[irgDictCache] = (dictbufcacheRecType *) 
                  alloc(sizeof(dictbufcacheRecType))) == NULL) {
      hf_err("rcia");
      return(151);
    }
    *(rgpdbcAct[irgDictCache]->szcKeyCached) = HB_EOS;
    rgpdbcAct[irgDictCache]->pszcData = NULL; /* indicate empty buf */
    rgpdbcAct[irgDictCache]->fFail = 0;
    if ((rgpdbcAct[irgDictCache]->szBuffer = (Char *) 
                  alloc(sizeof(Char)*(hf_ddc+100))) == NULL) {
      hf_err("rcib");
      return(152);
    }
    if ((rgpdbcAct[irgDictCache]->rgfDicPatOk = (char *) 
                  alloc(sizeof(char)*(crgDictPat))) == NULL) {
      hf_err("rcif");
      return(153);
    }
  } /* of loop over irgDictCache */
  irgDictCache = 0; /* first item to be filled in */
  /* added JH 29.08.97 END */

  /************************************************** endings ***********/
  /* endindgs */
  read(hf_cpd_ending, buffer, 2);     /* JH 24.11.1994 */
  sscanf(buffer,"%02x",&iMajorEnd);   /* JH 24.11.1994 */
  if (iMajorEnd != iMajor) {
    hf_err("majc");
    return(66);
  } /* JH 14.5.96 */
  read(hf_cpd_ending, buffer, 2);     /* JH 24.11.1994 */
  sscanf(buffer,"%02x",&iMinorEnd);   /* JH 24.11.1994 */
  read(hf_cpd_ending, buffer, 8);     /* JH 24.11.1994 */
#if !defined(__alpha)
  sscanf(buffer,"%08lx",&iTagsOffset);/* JH 24.11.1994 */
#else
  sscanf(buffer,"%08x",&iTagsOffset);/* JH 24.11.1994 */
#endif
  read(hf_cpd_ending, buffer, 4);     /* JH 24.11.1994 */
  sscanf(buffer,"%04x",&cTags);       /* JH 24.11.1994 */
  read(hf_cpd_ending, buffer, 8);     /* JH 24.11.1994 */
#if !defined(__alpha)
  sscanf(buffer,"%08lx",&cStrSize);   /* JH 24.11.1994 */
#else
  sscanf(buffer,"%08x",&cStrSize);   /* JH 24.11.1994 */
#endif
  read(hf_cpd_ending, buffer, 8);     /* JK 17.01.1993 */
#if !defined(__alpha)
  sscanf(buffer,"%08lx",&cont_lgt);   /* JK 17.01.1993 */
#else
  sscanf(buffer,"%08x",&cont_lgt);   /* JK 17.01.1993 */
#endif
  read(hf_cpd_ending, buffer, 4);     /* JK 17.01.1993 */
  sscanf(buffer,"%04x",&hf_edc);      /* JK 17.01.1993, JH 03.02.1994 */
  read(hf_cpd_ending, buffer, 4);     /* JK 17.01.1993 */
  sscanf(buffer,"%04x",&hf_esx);      /* JK 17.01.1993 */
  read(hf_cpd_ending, buffer, 4);     /* JK 17.01.1993 */
  sscanf(buffer,"%04x",&hf_exx);      /* JK 17.01.1993 */
  read(hf_cpd_ending, buffer, 4);     /* JK 17.01.1993 */
  sscanf(buffer,"%04x",&hf_ekeymax);  /* JK 17.01.1993, JH 03.02.1994 */
  read(hf_cpd_ending, buffer, 4);     /* JK 17.01.1993 */
  sscanf(buffer,"%04x",&max_bufneed); /* JK 17.01.1993 */
  read(hf_cpd_ending, buffer, 2);     /* JH 24.11.1994 */
  sscanf(buffer,"%02x",&i);           /* JH 24.11.1994 */
  cNegFirst = i;                      /* JH 24.11.1994 */
  read(hf_cpd_ending, buffer, 2);     /* JH 24.11.1994 */
  sscanf(buffer,"%02x",&i);           /* JH 24.11.1994 */
  cNejNo = i;                         /* JH 24.11.1994 */
  read(hf_cpd_ending, buffer, 8);      /* hajic 30.07.95 */
#if !defined(__alpha)
  sscanf(buffer,"%08lx",&iPatExOffset);   /* hajic 17.01.1997 */
#else
  sscanf(buffer,"%08x",&iPatExOffset);   /* hajic 17.01.1997 */
#endif
  /* add JH 29.08.97 BEG */
  read(hf_cpd_ending, buffer, 8);      /* hajic 29.08.97 */
#if !defined(__alpha)
  sscanf(buffer,"%08lx",&iDistOffset);   /* JK 17.01.1993 */
#else
  sscanf(buffer,"%08x",&iDistOffset);   /* JK 17.01.1993 */
#endif
  /* add JH 29.08.97 END */

  cAdjFirst = cNegFirst; /* JH 25.11.1994 */

  iCompOffset = HEADER_SIZE;
  lseek(hf_cpd_ending,iCompOffset,SEEK_SET);

/* JH 03.02.1994 tests eliminated
  if (hf_edc >= HF_EMAX_BUF ||
    hf_esx >= HF_EX_SP ||
    hf_ekeymax >= HF_EKEYMAX ||
    max_bufneed >= HF_EBUFNEED ||
    cont_lgt >= HF_ESIZE ||
    hf_exx >= HF_EX_MAX) {
    hf_err("hf_mod.c needs recompilation");
    return(6);
  }
*/

/* JH 03.02.1994 BEG */
  /* memory allocation based on actual values from the compiled file */
  if ((hf_ebuffer = (Char *)
                    alloc(cont_lgt+100)) == NULL) {
    hf_err("aleb");
    return(20);
  }
  if ((hf_ex_index = (Longint *)
                     alloc(sizeof(Longint*)*(hf_exx+4))) == NULL) {
    hf_err("alex");
    return(20);
  }
  if ((hf_ex_space = (Char *)
                     alloc(hf_esx+100)) == NULL) {
    hf_err("ales");
    return(20);
  }
  if ((hf_ex_space_index = (Char **)
                     alloc(sizeof(Char *)*(hf_exx+4))) == NULL) {
    hf_err("alds");
    return(20);
  }
  if ((hf_elist = (Char **)
                     alloc(sizeof(Char**)*(hf_ekeymax+2))) == NULL) {
    hf_err("alls");
    return(20);
  }
/* JH 03.02.1994 END */

/* JH 26.11.1994 BEG */
/* tag-related allocations */
  if ((pszTag = (char **)
                     alloc(sizeof(char *) * (cTags+1))) == NULL) {
    hf_err("tgca");
    return(20);
  }
  i = cStrSize;
  if ((pszStrSpace = (char *)
                     alloc(i+2)) == NULL) {
    hf_err("tgsa");
    return(20);
  }
/* JH 26.11.1994 END */

  /* endings: read ALL in memory */
  buflgt = cont_lgt - HEADER_SIZE;
  if (read(hf_cpd_ending,hf_ebuffer,buflgt) != buflgt) { /* JK 17.01.1993 */
    hf_err("einb");
    return(11);
  }
  /* no seek needed here */
  if (read(hf_cpd_ending,hf_ex_space,hf_esx) != hf_esx) { /* JK 17.01.1993 */
    return(7);
  }
  hf_ex_space[hf_esx++] = 0;
  psz=(char *) hf_ex_space;
  for (i = 0; i < hf_exx; i++) {
    hf_ex_space_index[i]=(Char *) psz;
    psz+=strlen(psz)+1;
  }
  offset = 0;
  for (i = 0; i < hf_exx; i++) {
    if (read(hf_cpd_ending, buffer, 4) != 4) {  /* JK 17.01.1993 */
      hf_err("itef");
      return(8);
    }
    sscanf(buffer,"%04x",&buflgt); /* JK 17.01.1993 */
    hf_ex_index[i] = (offset += buflgt) - HEADER_SIZE;
      /* -HEADER_SIZE: indexes are not in memory! */
  }
  hf_eboff = 0; /* beg of buffer */
  hf_esearch((Char *)"0",hf_elist,&fail_type);
  if (*hf_elist == NULL) {
    hf_err("0enf");
    return(12);
  }

  /* JH 26.11.1994: read in tags */

  lseek(hf_cpd_ending,iTagsOffset,SEEK_SET);
  pszStrFree = pszStrSpace;
  read(hf_cpd_ending, pszStrFree, 1);
  i = 0;
  while (*pszStrFree) {
    if (i >= cTags) {
      hf_err("tgcl");
      return(201);
    }
    pszTag[i++] = pszStrFree;
    while (*pszStrFree++) {
      read(hf_cpd_ending, pszStrFree, 1);
    }
    read(hf_cpd_ending, pszStrFree, 1);
  }
  if (i != cTags) {
    hf_err("tgcd");
    return(201);
  }
  /* JH 26.11.1994 end of tag reading */

  /* hajic 30.07.95 beg */
  lseek(hf_cpd_ending,iPatExOffset,SEEK_SET);
  /* now at start of five long numbers in %08x format */
#if !defined(__alpha)
  read(hf_cpd_ending, buffer, 8);
  sscanf(buffer,"%08lx",&crgPat);
  read(hf_cpd_ending, buffer, 8);
  sscanf(buffer,"%08lx",&cszPatSpace);
  read(hf_cpd_ending, buffer, 8);
  sscanf(buffer,"%08lx",&crgExpl);
  read(hf_cpd_ending, buffer, 8);
  sscanf(buffer,"%08lx",&cszExplSpace);
  read(hf_cpd_ending, buffer, 8);
  sscanf(buffer,"%08lx",&il);
#else
  read(hf_cpd_ending, buffer, 8);
  sscanf(buffer,"%08x",&crgPat);
  read(hf_cpd_ending, buffer, 8);
  sscanf(buffer,"%08x",&cszPatSpace);
  read(hf_cpd_ending, buffer, 8);
  sscanf(buffer,"%08x",&crgExpl);
  read(hf_cpd_ending, buffer, 8);
  sscanf(buffer,"%08x",&cszExplSpace);
  read(hf_cpd_ending, buffer, 8);
  sscanf(buffer,"%08x",&il);
#endif
  if (il != crgPat) {
    hf_err("pnmm");
  }
  /* counts/sizes read in... */
  /* printf("sizes: %ld %ld %ld %ld\n", */
    /*  crgPat,cszPatSpace,crgExpl,cszExplSpace); */
  /* allocations */
#if defined(HP)
/* Prekladac na HP nema u - specifikaci konstanty jako unsigned. */
  if (cszPatSpace > 65500) {
    hf_err("allp");
    return(202);
  }
#else
  if (cszPatSpace > 65500u) {
    hf_err("allp");
    return(202);
  }
#endif  
  ui = cszPatSpace;
  if ((szPatSpace = (char *) alloc(ui)) == NULL) {
    hf_err("alps");
    return(201);
  }
#if defined(HP)
/* Prekladac na HP nema u - specifikaci konstanty jako unsigned. */
  if (cszExplSpace > 65500) {
    hf_err("alle");
    return(202);
  }
#else
  if (cszExplSpace > 65500u) {
    hf_err("alle");
    return(202);
  }
#endif  
  ui = cszExplSpace;
  if ((szExplSpace = (char *) alloc(ui)) == NULL) {
    hf_err("alxs");
    return(201);
  }
#if defined(HP)
/* Prekladac na HP nema u - specifikaci konstanty jako unsigned. */
  if (crgchPat2Expl > 65500) {
    hf_err("allt");
    return(202);
  }
#else
  if (crgchPat2Expl > 65500u) {
    hf_err("allt");
    return(202);
  }
#endif  
  ui = crgchPat2Expl;
  if ((rgchPat2Expl = (char *) alloc(ui)) == NULL) {
    hf_err("alxt");
    return(201);
  }
  /* read in table pat --> expl */
  for (ui = 0; ui < crgchPat2Expl; ui++) {
    read(hf_cpd_ending, buffer, 2);
    /* should check for errors here ... */ /**/
    sscanf(buffer,"%02x",&uIndex);
    rgchPat2Expl[ui] = uIndex;
    /* printf(" >%u: c %c",ui,uIndex); */
  } /* of for */

  /* printf("--p--p--p---\n",*psz); */
  /* read in pattern strings */
  psz = szPatSpace;
  for (ui = 0; ui < cszPatSpace; ui++) {
    read(hf_cpd_ending, psz, 1);
    /* printf("%c",*psz); */
    /* should check for errors here ... */ /**/
    if (*psz == '\n') *psz = HF_EOS;
    psz++;
  } /* of for: reading pattern strings */
    /* printf("---------\n",*psz); */
  /* read  in expl. strings */
  psz = szExplSpace;
  for (ui = 0; ui < cszExplSpace; ui++) {
    read(hf_cpd_ending, psz, 1);
    /* printf("%c",*psz); */
    /* should check for errors here ... */ /**/
    if (*psz == '\n') *psz = HF_EOS;
    psz++;
  } /* of for: reading explanation strings */

  /* JH 21.08.95 index pat. strings */
  /**/ /* allocate rgpszPat properly, size crgPat not 256 !!!! */
  for (ui = 0; ui < crgPat; ui++) rgpszPat[ui] = "999";
  psz = szPatSpace;
  ui = 0;
  while (*psz != HF_EOS) {
    /* printf("indexing pat string %u: %s\n",ui,psz); */
    rgpszPat[ui++] = psz;
    /* NB: pat strings are never coded */
    while (*psz++ != HF_EOS); /* ignore the string */
  } /* of indexing pat strings */
  /* JH 21.08.95 index pat. strings: end */

  /* JH 21.08.95 index expl. strings */
  for (ui = 0; ui < 256; ui++) rgpszExpl[ui] = " ";
  psz = szExplSpace;
  while (*psz != HF_EOS) {
    /* printf("indexing string (coded): %s\n",psz); */
    rgpszExpl[(Char)(*psz)] = psz + 1;
    /* NB: expl strings are coded: as opposed to patterns */
    while (*psz++ != HF_EOS); /* ignore the string */
  } /* of indexing expl strings */
  /* JH 21.08.95 index expl. strings: end */

  /* hajic 30.07.95 end */

  /* add JH 29.08.97 BEG */
  /**************************** endings: read distribution data (derivation) */
  lseek(hf_cpd_ending,iDistOffset,SEEK_SET);
  /* now at start of three long numbers in %08x format */
  if (read(hf_cpd_ending, buffer, 8) != 8) {
    hf_err("ein1");
    return(261);
  }
#if !defined(__alpha)
  sscanf(buffer,"%08lx",&crgDistDataIndex);
#else
  sscanf(buffer,"%08x",&crgDistDataIndex);
#endif
  if (crgPat + 1 != crgDistDataIndex) {
    hf_err("pdna");
    return(251);
  }
/**/ /* fprintf(stderr,"crgPat: %ld, crgDistDataIndex: %ld\n",
             crgPat, crgDistDataIndex); /**/
  ui = crgDistDataIndex * sizeof(int);
#if !defined(unix)
  if (ui > 65500u) {
    hf_err("dosi");
    return(254);
  }
#endif
  /* allocate space for master index: */
  if ((rgDistDataIndex = (int *)
                    alloc(ui)) == NULL) {
    hf_err("aldi");
    return(255);
  }

  if (read(hf_cpd_ending, buffer, 8) != 8) {
    hf_err("ein2");
    return(262);
  }
#if !defined(__alpha)
  sscanf(buffer,"%08lx",&cDistRules);
#else
  sscanf(buffer,"%08x",&cDistRules);
#endif
/**/ /* fprintf(stderr,"cDistRules: %ld\n",
             cDistRules); /**/
  ui = cDistRules * sizeof(int);
#if !defined(unix)
  if (ui > 65500u) {
    hf_err("dosr");
    return(256);
  }
#endif
  /* allocate space for dict pattern numbers: */
  if ((rgDictPat = (int *)
                    alloc(ui)) == NULL) {
    hf_err("aldr");
    return(257);
  }
  /* allocate space for distribution string pointers (int): */
  ui = cDistRules * sizeof(int);
  if ((rgDistData = (int *)
                    alloc(ui)) == NULL) {
    hf_err("alde");
    return(258);
  }

  if (read(hf_cpd_ending, buffer, 8) != 8) {
    hf_err("ein3");
    return(263);
  }
#if !defined(__alpha)
  sscanf(buffer,"%08lx",&cszDistDataSpace);
#else
  sscanf(buffer,"%08x",&cszDistDataSpace);
#endif
/**/ /* fprintf(stderr,"cszDistDataSpace: %ld\n",
             cszDistDataSpace); /**/
  ui = (cszDistDataSpace + 1) * sizeof(char);
#if !defined(unix)
  if (ui > 65500u) {
    hf_err("dosb");
    exit(252);
  }
#endif
  /* allocate space for strings: */
  if ((szDistDataSpace = (char *)
                    alloc(ui)) == NULL) {
    hf_err("aldd");
    return(253);
  }
/**/ /* fprintf(stderr,"cszDistDataSpace: %ld, allocated string space\n",
             cszDistDataSpace); /**/
  /* end of reading sizes and allocating the three arrays + str area */

  /* now read the data into them: */
  for (ui = 0; ui < crgDistDataIndex; ui++) {
    if (read(hf_cpd_ending, buffer, 4) != 4) {
      hf_err("ein4");
      return(264);
    }
    sscanf(buffer,"%04x",&(rgDistDataIndex[ui]));
  }
/**/ /* fprintf(stderr,"last rgDistDataIndex[ui]: %ld, %d\n",ui-1,
                        rgDistDataIndex[ui-1]); /**/
  for (ui = 0; ui < cDistRules; ui++) {
    if (read(hf_cpd_ending, buffer, 4) != 4) {
      hf_err("ein5");
      return(265);
    }
    sscanf(buffer,"%04x",&(rgDictPat[ui]));
  }
/**/ /* fprintf(stderr,"last rgDictPat[ui]: %ld, %d\n",ui-1,
                        rgDictPat[ui-1]); /**/
  for (ui = 0; ui < cDistRules; ui++) {
    if (read(hf_cpd_ending, buffer, 4) != 4) {
      hf_err("ein6");
      return(266);
    }
    sscanf(buffer,"%04x",&(rgDistData[ui]));
  }
/**/ /* fprintf(stderr,"last rgDistData[ui]: %ld, %d\n",ui-1,
                        rgDistData[ui-1]); /**/
  for (ui = 0; ui < cszDistDataSpace; ui++) {
    if (read(hf_cpd_ending, buffer, 1) != 1) {
      hf_err("ein7");
      return(267);
    }
    szDistDataSpace[ui] = *buffer;
  }
  /**************** endings: read distribution data (derivation) -- end ******/
  /* add JH 29.08.97 END */

  /* 01/09/01 JH */
  cCoded0 = hh_tab['0']; /* for string/char for "epmty string" in
                             endings, der. rules */
  return(0);
} /* of hf_init */

FUNDEF6(int
hf_LemmaBase,
plangEnvRec, plangEnv,
Char *, s,
int, dot,
int, hyph,
int *, pcnt,
int, fOutFlag)
{
/* call morphology in three stages:
 * first, leave input word as is;
 * if no solution, try capitalized-only form (if different)
 * if no solution even then, try all lowercase (this works espec.
 * for words at sentence beginnings)
 *
 * stop when first successful solution found
 *
 * s must NOT be empty
 * dot != 0 iff s followed by dot in the text
 * hyph != 0 iff s followed by hyphen in the text
 */
Char scopy[32];
Char *pscopy;
Char c;
Char c1;
int lowered;
int ls;
  ls = strlen((char *)s);
  if (ls > 31) {
    goto not_found;
  }


  hf_xresult = 0;   /* start of final result list string space */
  hf_res_count = 0; /* no. of final results */

  *szPrevRootS = '\0'; /* global vars... */
  *szPrevRootNs = '\0';
  *szPrevRootJs = '\0';
  *szPrevRootJns = '\0';

  hf_LemmaSingle(plangEnv,s,dot,hyph,pcnt,fOutFlag);

   /* first attempt: leave capitals if any */
   /* regardless of result, try with some chars lowered ... Nemoci !!!! */

  strcpy((char *)scopy,(char *)s);
  pscopy = scopy;
  c1 = hh_tab[*pscopy]; /* keep first char code */
  pscopy++; /* leave first char for now */
  lowered = 0;
  while (*pscopy) {
     c = hh_tab[*pscopy];
     if (UpperCaseCode(c)) { /* uppercase */
      lowered = 1;
      *pscopy = hh_inv_tab[c+1];
     }
     pscopy++;
  }
  if (lowered) { /* yes, there is some chance */
    hf_LemmaSingle(plangEnv,scopy,dot,hyph,pcnt,fOutFlag);

    /* again, w/o regard to result, try all lowercase... */
    lowered = 0;
    if (UpperCaseCode(c1)) { /* uppercase */
      lowered = 1;
      *scopy = hh_inv_tab[c1+1];
    }
    if (lowered) {
      /* last chance */
      hf_LemmaSingle(plangEnv,scopy,dot,hyph,pcnt,fOutFlag);
    }
  } /* of try again */
  else { /* test first char, maybe at least *it* is uppercase... */
    lowered = 0;
    if (UpperCaseCode(c1)) { /* uppercase */
      lowered = 1;
      *scopy = hh_inv_tab[c1+1];
    }
    if (lowered) {
      /* last chance */
      hf_LemmaSingle(plangEnv,scopy,dot,hyph,pcnt,fOutFlag);
    }
  } /* of 2nd to end orig lowercase only */

  /* here: *pcnt contents depends on hf_LemmaSingle calls */
  hf_results[hf_xresult++] = 0;
  hf_results[hf_xresult++] = 0;
  hf_xresult = 0; /* prepare for hf_get_res */
  hf_no_res = 0; /* actual no. of get result, for hf_get_res ;XX_HR 4 */
  return(0);

not_found:
  *pcnt = 0;
  return(0);
} /* of hf_LemmaBase */

FUNDEF5(int
hf_lemma,
plangEnvRec, plangEnv,
Char *, s,
int, dot,
int, hyph,
int *, pcnt)
{
/* call morphology in three stages:
 * first, leave input word as is;
 * if no solution, try capitalized-only form (if different)
 * if no solution even then, try all lowercase (this works espec.
 * for words at sentence beginnings)
 *
 * stop when first successful solution found
 *
 * s must NOT be empty
 * dot != 0 iff s followed by dot in the text
 * hyph != 0 iff s followed by hyphen in the text
 */
int iRC;

  iRC = hf_LemmaBase(plangEnv,s,dot,hyph,pcnt,0); /* output as usual */
  return iRC;
} /* of hf_lemma */

FUNDEF5(int
hf_LemmaRawFlags,
plangEnvRec, plangEnv,
Char *, s,
int, dot,
int, hyph,
int *, pcnt)
{
/* call morphology in three stages:
 * first, leave input word as is;
 * if no solution, try capitalized-only form (if different)
 * if no solution even then, try all lowercase (this works espec.
 * for words at sentence beginnings)
 *
 * stop when first successful solution found
 *
 * s must NOT be empty
 * dot != 0 iff s followed by dot in the text
 * hyph != 0 iff s followed by hyphen in the text
 */
int iRC;

/*** fprintf(stderr,"doing rawflags, string %s\n",s); /**/
  iRC = hf_LemmaBase(plangEnv,s,dot,hyph,pcnt,1); /* raw flags only --
                no expansion */
  return iRC;
} /* of hf_LemmaRawFlags */

Char *
hf_get_res()
{
/* XX_HR 4
 * set hf_is_ne iff prefix (naj)ne- as SIDE EFFECT
 */
Char *res;
  res = hf_results + hf_xresult;
  if (*res) {
    hf_is_ne = hf_ne_results[hf_no_res++];  /*SIDE EFFECT ne- ;XX_HR 4 */
    hf_xresult += strlen((char *)res) + 1;
    return(res);
  }
  else {
    /* Zmena O.C. 21.7.1997 */
    hf_xresult=0;
    hf_no_res=0;
    return(NULL);
  }
} /* of hf_get_res */

/* JK ??.??.???? (17.01.1993?) hf_end */
void
hf_end()
{
longint i;
  close(hf_cpd_dict); /* JK 17.01.1993 */
  close(hf_cpd_ending); /* JK 17.01.1993 */
  free(hf_ebuffer);       /* XX_HR */
  free(hf_dbuffer);       /* JK 20.01.1994 */
  free(hf_elist);         /* JH 03.02.1994 */
  free(hf_ex_space);      /* JH 03.02.1994 */
  free(hf_ex_space_index);
  free(hf_dx_space);      /* JH 03.02.1994 */
  free(hf_dx_space_index);
  free(hf_ex_index);      /* JH 03.02.1994 */
  free(hf_dx_index);      /* JH 03.02.1994 */
  free(szPatSpace);       /* JH 27.08.1995 */
  free(szExplSpace);      /* JH 27.08.1995 */
  free(rgchPat2Expl);     /* JH 27.08.1995 */
  free(szDictPatSpace);
  free(rgpszDictPat);
  for (i = 0; i < crgDictCache; i++) {
    free(rgpdbcAct[i]->szBuffer);
    free(rgpdbcAct[i]->rgfDicPatOk);
    free(rgpdbcAct[i]);
  }
  free(rgpdbcAct);
  free(pszTag);
  free(pszStrSpace);
  free(rgDistDataIndex);
  free(rgDictPat);
  free(rgDistData);
  free(szDistDataSpace);

} /* of hf_end */

#define HF_DEFC
#endif

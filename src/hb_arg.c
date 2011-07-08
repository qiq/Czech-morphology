/*
  passing parameters to programs
  (C) Jan Hajic 1995 

NB: dodelat parametry k filenames: radsi dymanicky, apak subst. pri volani!
NB: dodelat kompletne volani GetValueOfAttr etc.!!


*/

#include <stdio.h>
#include <string.h>
#include <time.h>

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
#include "hb_arg.h"

/* 
#include <malloc.h>
/**/
/*
#include <alloc.h>
/**/

/* global static data */
static longint cargFiles = 0;
hb_argFilePtr *rgargFile = NULL;
char szT[HB_STRINGMAX];
char szNum[HB_STRINGMAX];
shortint cSubstLevel;

/* private functions: */
PROTO(int hb_ArgParseFileName, (char *szIn, 
                                char *szFNCopy, Shortint csMaxCopy,
                                char **ppszParmPtr));
PROTO(int hb_ArgParseParm, (shortint *pcszPar,
          char *szIn,
          char ***prgszPar));
PROTO(int hb_ArgFillFromFile, (char *szIn,
             hb_argAttrPtr *ppargFirst));

PROTO(int hb_ArgSubstitute, (char *szSubstituteTo,
                             hb_argAttrPtr paa1st, 
                             char *szSubResult,
                             shortint cMaxResult));

PROTO(int hb_ArgFileRef, (char *szIn, char **ppszAttr));

FUNDEF2(int hb_ArgFileRef, 
char *, szIn,
char **, ppszAttr)
{
char *pch;
  pch = szIn;
  /**/ /* fprintf(stderr,"fileref test %s\n",szIn); */
  while (*pch) {
    if (*pch++ == HB_ARG_VARSEP) {
  /**/ /* fprintf(stderr,"found %s\n",pch); */
      *ppszAttr = pch;
      return 1;
    }
  }
  return 0;
} /* of hb_ArgFileRef */

FUNDEF4(int hb_ArgSubstitute,
char *, szSubstituteTo,
hb_argAttrPtr, paa1st, 
char *, szSubResult,
shortint, cMaxResult)
{
struct Local {
 char sz[HB_LONGSTRINGMAX];
 char szActAttrName[HB_LONGSTRINGMAX];
 char szActValue[HB_LONGSTRINGMAX];
 char szRef[HB_LONGSTRINGMAX];
 char szRefResult[HB_LONGSTRINGMAX];
} *local;

char *pchIn;
char *pchResult;
longint iResult;
hb_argAttrPtr paa;
char *pchRef;
shortint iResCode;
char *pszAttr;
shortint iLev;

  if ((local = (struct Local *) hb_LongAlloc(sizeof(struct Local))) == NULL) {
    hb_FatalError(HB_ARG,999,"local alloc");
    return 999;
  }

  iResult = 0;
  pchResult = szSubResult;
  *pchResult = HB_EOS;
  pchIn = szSubstituteTo;

  /**/ /* ONLY PARAMETERS SHOULD BE LOCAL!!!!! */
  /**/ /* test it here. */
  /**/ /*for (iLev = 0; iLev < cSubstLevel; iLev++) fprintf(stdout,"-"); /**/
  /**/ /*fprintf(stdout,"*** hb_ArgSubstitute start: in: %s\n",pchIn); /**/
  cSubstLevel++;
  while (*pchIn != HB_EOS) {
    if (*pchIn == HB_ARG_SVAR) { /* start of var reference */
      pchRef = local->szRef;
      pchIn++;
      while (*pchIn != HB_ARG_EVAR) {
        *pchRef++ = *pchIn++;
      }      
      *pchRef++ = HB_EOS;
      /* solve embedded refs: */
      if ((iResCode = 
          hb_ArgSubstitute(local->szRef,paa1st,local->szRefResult,HB_LONGSTRINGMAX-2)) > 0) {
        hb_FatalError(HB_ARG,28,local->szRef); /**/
        hb_Free(local);
        cSubstLevel--;
        return 1;
      }
      if (iResCode < 0) { 
        cSubstLevel--;
        hb_Free(local);
        return iResCode;
      }
      /* now reference flat, just a string */
      /* search for equiv. attr name in list */
      do {
        paa = paa1st;
        while (paa != NULL) {
          if (paa->fLocked <= 0) {
            paa->fLocked++;
            if ((iResCode = hb_ArgSubstitute(paa->szAttr,
                             paa1st,local->szActAttrName,HB_LONGSTRINGMAX-2)) > 0) {
              hb_FatalError(HB_ARG,28,local->szRef); /**/
              hb_Free(local);
              cSubstLevel--;
              return 2;
            }
            if (iResCode < 0) {
              cSubstLevel--;
              hb_Free(local);
              return iResCode;
            }
            /**/ /*
            fprintf(stdout,"= strcmp: %s %s\n",
            local->szRefResult,local->szActAttrName); /**/
            if (!strcmp(local->szRefResult,local->szActAttrName)) { /* found */
              /* NB: taking first (NB: reverse order of attr) */
              if ((iResCode = hb_ArgSubstitute(paa->pszVal,
                                 paa1st,local->szActValue,HB_LONGSTRINGMAX-2)) > 0) {
                hb_FatalError(HB_ARG,28,local->szRef); /**/
                hb_Free(local);
                cSubstLevel--;
                return 3;
              }
              if (iResCode < 0) {
                cSubstLevel--;
                hb_Free(local);
                return iResCode;
              }
              if (strlen(szSubResult)+strlen(local->szActValue)+2 > cMaxResult) {
                hb_FatalError(HB_ARG,202,local->szActValue);
                hb_Free(local);
                return 90;
              }
              strcpy(pchResult,local->szActValue);
              pchResult += strlen(local->szActValue);
              paa->fLocked--;
              break;
            } /* attr found */         
            paa->fLocked--;
          } /* not locked */
          paa = paa->paaNext;
        }
        if (paa == NULL && 
            (iResCode = hb_ArgFileRef(local->szRefResult,&pszAttr)) > 0) {
          strcpy(local->sz,pszAttr);
          strcpy(local->szRefResult,local->sz);
        }
      } while (paa == NULL &&  iResCode > 0);
      if (paa == NULL) {
        /* not found */
        cSubstLevel--;
        hb_Free(local);
        return -1;
      }
    } /* of HB_ARG_SVAR */
    else {
      if (strlen(szSubResult)+3 > cMaxResult) {
        fprintf(stderr,"%s %d %d\n",szSubResult,strlen(szSubResult),cMaxResult);
        hb_FatalError(HB_ARG,201,"too long result string");
        hb_Free(local);
        return 91;
      }
      iResult++;
      *pchResult++ = *pchIn;
      *pchResult = HB_EOS;
    }
    pchIn++;
  } /* while not HB_EOS */
  *pchResult = HB_EOS;
  cSubstLevel--;
  /**/ /* for (iLev = 0; iLev < cSubstLevel; iLev++) fprintf(stderr,"~"); /**/
  /**/ /* fprintf(stdout,
       "*** hb_ArgSubstitute result out: %s\n",szSubResult); /**/
  hb_Free(local);
  return 0;
} /* of hb_ArgSubstitute */

FUNDEF3(int hb_ArgParseParm,
shortint *, pcszPar,
char *, szIn,
char ***,prgszPar)
{
struct Local {
 char szOneArg[HB_LONGSTRINGMAX];
} *local;

char *pchIn; /* to szIn */  
shortint cszPar;
shortint iState;
shortint iOneArg;
shortint cszParUB;

  if ((local = hb_LongAlloc(sizeof(struct Local))) == NULL) {
    hb_FatalError(HB_ARG,999,"local alloc");
    return 999;
  }
  cszParUB = 1; /* upper bound :-) */
  pchIn = szIn;
  while (*pchIn != HB_EOS) {
    if (*pchIn == HB_ARG_ARGSEP) cszParUB++;
    pchIn++;
  }
  if (NULL == (*prgszPar = (char **) 
               hb_LongAlloc(cszParUB*sizeof(char *)))) {
    hb_FatalError(HB_ARG,24,szIn);
    hb_Free(local);
    return 1;
  }

  iState = 0;
  cszPar = 0;
  pchIn = szIn;

  while (*pchIn != HB_EOS) {
    switch (iState) {
      case 0: {
        if (*pchIn == ' ' || *pchIn == '\t' || *pchIn == '\n'); /* ign */
        else if (*pchIn == HB_ARG_OPAR) {
          iOneArg = 0;
          iState = 1;
        }
        break;
      }
      case 1: {
        if (*pchIn == ' ' || *pchIn == '\t' || *pchIn == '\n'); /* ign */
        else if (*pchIn == HB_ARG_ARGSEP || *pchIn == HB_ARG_CPAR) {
          /* next argument follows/end of args */
          local->szOneArg[iOneArg++] = HB_EOS;
          if (NULL == ((*prgszPar)[cszPar] = (char *) 
               hb_LongAlloc(iOneArg*sizeof(char)))) {
            hb_FatalError(HB_ARG,24,szIn);
            hb_Free(local);
            return 1;
          }
          strcpy((*prgszPar)[cszPar],local->szOneArg);
          cszPar++;
          iOneArg = 0;
          if (*pchIn == HB_ARG_CPAR) iState = 2;
        }
        else {
          if (iOneArg < HB_LONGSTRINGMAX-2) {
            local->szOneArg[iOneArg++] = *pchIn;
          }
          else /* truncate w/o warning */ /**/;
        }
        break;
      }
      case 2: {
        if (*pchIn == ' ' || *pchIn == '\t' || *pchIn == '\n'); /* ign */
        else iState = 3; /* error: extra chars */ /**/
        break;
      }
      default: ;
    }
    pchIn++;
  }
  /**/ /* here should check iState == 2 */
  *pcszPar = cszPar;
  hb_Free(local);
  return 0;
} /* of hb_ArgParseParm */

FUNDEF4(int hb_ArgParseFileName,
char *, szIn,
char *, szFNCopy,
Shortint, csMaxCopy,
char **, ppszParmPtr)
{

/*
  separates filename and parameters in parenthesis; pszParmPtr is
  empty string if no params, or ptr to the left ( if parms present 
  NB: filename is copied to szFNCopy but params not, only ptr set!
*/

char *pchIn;
char *pchCopy;
Shortint cCopied;
char chInLit; /* EOS if not within quotes */

  if (szIn == NULL) {
    strcpy(szFNCopy,HB_ARG_STDIN);
    *ppszParmPtr = szFNCopy + strlen(szFNCopy); /* HB_EOS */
    return 0;
  }
  cCopied = 0;
  pchIn = szIn;
  pchCopy = szFNCopy;
  chInLit = HB_EOS;
  while (*pchIn && ((*pchIn != HB_ARG_OPAR) || (chInLit != 0))) {
    if (cCopied >= csMaxCopy - 2) {
      hb_FatalError(HB_ARG,23,szIn);
      return 1;
    }
    if (*pchIn == HB_ARG_SQ || *pchIn == HB_ARG_DQ) {
      if (*pchIn == chInLit) chInLit = HB_EOS; /* closing */
      else if (chInLit == HB_EOS) chInLit = *pchIn; /* opening */
      else /* the other quote quoted :-) */
        *pchCopy++ = *pchIn;
    }
    else *pchCopy++ = *pchIn; /* not a quote */
    pchIn++;
    cCopied++;
  } /* of while *pchIn */
  *pchCopy = HB_EOS;
  *ppszParmPtr = pchIn; /* EOS or ( */
  return 0;
} /* of hb_ArgParseFileName */

FUNDEF2(int hb_ArgFillFromFile, 
char *, szIn,
hb_argAttrPtr *, ppargFirst)
{
/* may be called recursively */
struct Local {
 char szN[HB_STRINGMAX];
 char szAttrP[HB_STRINGMAX];
 char szValP[HB_LONGSTRINGMAX];
 char szDir[HB_STRINGMAX];
 char szDirPar[HB_STRINGMAX];
} *local;

char *pszP;
int f;
char chRead;
longint iResult; 
int iState;
char *pchAttr;
char *pchVal;
hb_argAttrPtr paa;
char chInLit;
shortint iVarLevel;
longint cLineNo;
char *pchDir;
char *pchDirPar;
char **rgszPar;
shortint cszPar;
shortint cActPar;
char *pch;
char *pszIn;

  if ((local = hb_LongAlloc(sizeof(struct Local))) == NULL) {
    hb_FatalError(HB_ARG,999,"local alloc");
    return 999;
  }
  /* fills attr name (spaces allowed; compressed into single space) */
  /* and value (sp/tab separated fields replaced by single HB_ARG_FS, */
  /* ^F */
  /* continuation lines as in a shell: backslash \ must be at end of line */
  /* avoid = in attr name, and \ in value, but quotes (both ' and ") */
  /* do what they are supposed to do */

  /**/ /* fprintf(stderr,"hb_ArgFill...: stack %u\n",stackavail()); /**/
  hb_ArgParseFileName(szIn,local->szN,HB_STRINGMAX,&pszP);
  if (szIn == NULL || !strcmp(szIn,HB_ARG_STDIN)) {
    f = 0; /*stdin->fd*/
    pszIn = local->szN;
  }
  else {
    pszIn = szIn;
    f = hb_Open(local->szN,"rt");
  }
  cszPar = 0;

  /* parse parameters: */
  /**/ /* fprintf(stderr,"bef AgrParseParm: %u\n",stackavail());
  hb_ArgParseParm(&cszPar,pszP,&rgszPar); /**/

  /**/ /* fprintf(stdout,"%s opened, %d par parsed\n",local->szN,cszPar); /**/
   
  cActPar = 0;
  cLineNo = 1;
  iState = 1;
  chInLit = HB_EOS;
  pchAttr = local->szAttrP;
  pchVal = local->szValP;
  *pchVal = HB_EOS;
  iVarLevel = 0;
  iResult = 1;
  while (iResult && (iResult = hb_Read(f,(void *)&chRead,1)) >= 0) {
    if (iResult == 0) chRead = '\n';
    if (chRead == '\r') continue;
    switch (iState) {
      /* 1 parsing attr name
         2 parsing value
         3 directives, comments (#) 
         4 directive switch
         5 ignore till end of line
         6 include file parsing
         7 variable reference start (attr)
         8 variable reference start (var) */

    case 1: 
      if (chRead == HB_ARG_PRE && pchAttr == local->szAttrP) {
        pchDir = local->szDir;
  iState = 3;
      }
      else if (cszPar > cActPar) {
        hb_FatalError(HB_ARG,27,local->szN);
        hb_Free(local);
        return 8;
      }
      else if (chRead == HB_ARG_SQ || chRead == HB_ARG_DQ) {
        if (chRead == chInLit) {
          chInLit = HB_EOS; /* end of quote */
        }
        else if (chInLit == HB_EOS) {
    chInLit = chRead; /* beg of quote */
        }
        else {
          *pchAttr++ = chRead; /* the other quote */
        }
      }
      else if (chInLit != HB_EOS) {
        /* within quotes */
        *pchAttr++ = chRead; /* no matter what */
      }
      else if (chRead == HB_ARG_VAR) {
        /* variable reference start */
        *pchAttr++ = HB_ARG_SVAR;
        iState = 7;
      }
      else if (chRead == HB_ARG_IEVAR) {
        if (iVarLevel <= 0) {
          /**/
          hb_FatalError(HB_ARG,25,"too many variable closing parentheses");
          hb_Free(local);
          return 9;
        }
        *pchAttr++ = HB_ARG_EVAR;
        iVarLevel--;
      }
      else if (chRead == '=') {
        if (iVarLevel > 0) {
          /**/
          hb_FatalError(HB_ARG,25,"too few variable closing parentheses");
          hb_Free(local);
          return 9;
        }
        iVarLevel = 0;
        if (pchAttr == local->szAttrP) { /* error: empty attr name */
          sprintf(szT,"%ld",cLineNo);
          strcat(szT," (before =) of file(par) ");
          strcat(szT,pszIn);
          hb_FatalError(HB_ARG,25,szT);
          pchAttr = local->szAttrP;
          pchVal = local->szValP;
          *pchVal = HB_EOS;
          iState = 1;
          break;
        }
        if (*(pchAttr-1) == ' ') pchAttr--; /* ignore last space */
          /* (or spaces; already reduced to one */
        *pchAttr = HB_EOS;
        iState = 2;
      }
      else if (chRead == ' ' || chRead == '\t') {
        if (pchAttr == local->szAttrP) { /* beginning of attr name */
          /* ignore */
        }
        else if (*(pchAttr-1) == ' ') { 
          /* reduce seq. of spc into a single space; thus, ignore it here */
        } 
        else {
          *pchAttr++ = ' ';
        }
      }
      else if (chRead == '\n') {
        if (pchAttr == local->szAttrP) {
    /* ignore empty line */
        }
        else if (*(pchAttr-1) == HB_ARG_BS) {
    /* \n preceded by line continuation char, BS */
    *(--pchAttr) = HB_EOS; /* overwrite by EOS */
          /* now can test if anything left: */
    if (*(local->szAttrP) && *(pchAttr-1) == ' ') {
      /* do not add space, already there */
    }
    else if (*(local->szAttrP)) *pchAttr++ = ' ';
  }
  else if (*(pchAttr-1) == ' ' &&
                 *(pchAttr-2) == HB_ARG_BS) {
          pchAttr--; /* disregard FS */
    *(--pchAttr) = HB_EOS; /* overwrite by BS by EOS */
          /* now can test if anything left: */
    if (*local->szAttrP && *(pchAttr-1) == ' ') {
      /* do not add FS, already there */
    }
    else if (*(local->szAttrP)) *pchAttr++ = ' ';
  }
  else { /* error */
    *pchAttr = HB_EOS;
    sprintf(szT,"%ld",cLineNo);
    strcat(szT," of file(par) ");
    strcat(szT,pszIn);
    strcat(szT," (no = after attr name ");
    strcat(szT,local->szAttrP);
    strcat(szT,")");
    hb_FatalError(HB_ARG,25,szT);
    cLineNo++;
    pchAttr = local->szAttrP;
    pchVal = local->szValP;
    *pchVal = HB_EOS;                     
    iState = 1;
  }
  }
  else *pchAttr++ = chRead;
      break;
    case 2: /* now parsing value */
      if (chRead == HB_ARG_SQ || chRead == HB_ARG_DQ) {
        if (chRead == chInLit) {
          chInLit = HB_EOS; /* end of quote */
        }
        else if (chInLit == HB_EOS) {
    chInLit = chRead; /* beg of quote */
        }
        else {
          *pchVal++ = chRead; /* the other quote */
        }
      }
      else if (chInLit != HB_EOS) {
        /* within quotes */
        *pchVal++ = chRead; /* no matter what */
      }
      else if (chRead == HB_ARG_VAR) {
        /* variable reference start - val */
        *pchVal++ = HB_ARG_SVAR;
        iState = 8;
      }
      else if (chRead == HB_ARG_IEVAR) {
        if (iVarLevel <= 0) {
          /**/
          hb_FatalError(HB_ARG,25,
            "too many variable closing parentheses (val)");
          hb_Free(local);
          return 10;
        }
        *pchVal++ = HB_ARG_EVAR;
        iVarLevel--;
      }
      else if (chRead == ' ' || chRead == '\t') {
        if (pchVal == local->szValP) { /* beginning of value */
          /* ignore */
        }
        else if (*(pchVal-1) == HB_ARG_FS) { 
          /* reduce seq. of spc into a Field Sep; thus, ignore it here */
        } 
        else {
          if (iVarLevel == 0)
            *pchVal++ = HB_ARG_FS;
          else
            *pchVal++ = ' '; /* var (attr) reference */
        }
      }
      else if (chRead == '\n') {
        if (iVarLevel > 0) {
          /**/
          hb_FatalError(HB_ARG,25,
            "too few variable closing parentheses (val)");
          hb_Free(local);
          return 11;
        }
        if (pchVal == local->szValP) { /* beginning of Val */
          /* OK, empty value */
        }
        else if (*(pchVal-1) == HB_ARG_FS) { 
          /* ignore closing FS here */
          pchVal--;
        } 
        *pchVal = HB_EOS;
        if (*(local->szValP) && *(pchVal-1) == HB_ARG_BS) {
    /* \n preceded by line continuation char, BS */
          /* NB: FS possibly already deleted */
    *(--pchVal) = HB_EOS; /* overwrite by EOS */
          /* now test if still something in szVal */
    if (*(local->szValP) && *(pchVal-1) == HB_ARG_FS) {
      /* do not add FS, already there */
    }
          else if (*(local->szValP)) *pchVal++ = HB_ARG_FS;
          break; /* continue with status == 2, value */
  }

    /**/ /* fprintf(stdout,"%s = <%s>\n",local->szAttrP,local->szValP); /**/

        /* now allocate space and add attr/val pair to the current list */
        if (NULL == (paa = (hb_argAttrPtr) 
               hb_LongAlloc(sizeof(hb_argAttrType)))) {
          hb_FatalError(HB_ARG,24,local->szAttrP);
          hb_Free(local);
          return 2;
        }
        if (NULL == (paa->szAttr = (char *)
               hb_LongAlloc(sizeof(char)*(strlen(local->szAttrP)
                        +strlen(local->szValP)+2)))) {
          hb_FatalError(HB_ARG,24,local->szAttrP);
          hb_Free(local);
          return 3;
        }
        strcpy(paa->szAttr,local->szAttrP);
        paa->pszVal = paa->szAttr + strlen(paa->szAttr) + 1;
        strcpy(paa->pszVal,local->szValP);
        paa->fLocked = 0;
        /* link the structure as the first one: */
        paa->paaNext = *ppargFirst;
        *ppargFirst = paa;

        cLineNo++;
        pchAttr = local->szAttrP;
        pchVal = local->szValP;
        *pchVal = HB_EOS;
        iState = 1;
      }
      else *pchVal++ = chRead;
      break;
    case 3:
      if (chRead == ' ' || chRead == '\t' || chRead == '\n') {
        /* end of directive name */
        *pchDir = HB_EOS;
        if (*(local->szDir) == HB_EOS) {
    /* comment */
          if (chRead == '\n') {
      cLineNo++;
      iState = 1;
    }
    else 
      iState = 5;
  }
        else if (!strcmp(local->szDir,"include")) {
          if (chRead == '\n') {
      /**/ /* error!! */
      cLineNo++;
      iState = 1;
    }
    else 
      iState = 4;
          /**/ /* fprintf(stderr,"************** will include...\n"); */
  }
        else if (*(local->szDir) == HB_ARG_VAR) {
          /* parameter declaration */
          /* now allocate space and add var/val pair to the current list */
          if (cActPar >= cszPar) {
            hb_FatalError(HB_ARG,26,local->szN);
            hb_Free(local);
            return 7;
          }
          if (NULL == (paa = (hb_argAttrPtr) 
                 hb_LongAlloc(sizeof(hb_argAttrType)))) {
            hb_FatalError(HB_ARG,24,local->szDir);
            hb_Free(local);
            return 5;
          }
          if (NULL == (paa->szAttr = (char *)
                 hb_LongAlloc(sizeof(char)*
                   (strlen(local->szN)+
                    strlen(local->szDir)+1+
                    strlen(rgszPar[cActPar])+1)))) {
            hb_FatalError(HB_ARG,24,local->szDir);
            hb_Free(local);
            return 6;
          }
          strcpy(paa->szAttr,local->szN);
          paa->szAttr[strlen(paa->szAttr)+1] = HB_EOS;
          paa->szAttr[strlen(paa->szAttr)] = HB_ARG_VARSEP;
          strcat(paa->szAttr,(local->szDir)+1);
          paa->pszVal = paa->szAttr + strlen(paa->szAttr) + 1;
          strcpy(paa->pszVal,rgszPar[cActPar]);
          /* link the structure as the first one: */
          paa->paaNext = *ppargFirst;
          *ppargFirst = paa;
          cActPar++;
          if (chRead == '\n') {
      cLineNo++;
      iState = 1;
    }
          else 
            iState = 5; /* comment (ignore) till end of line */
        }
      }
      else {
  *pchDir++ = chRead;
      }
      break;
    case 4:
      if (chRead == ' ' || chRead == '\t') {
        /* ignore spaces */
      }
      else {
  pchDirPar = local->szDirPar;
        *pchDirPar++ = chRead;
  iState = 6;
      }
      break;
    case 5: /* ignore till end of line */
      if (chRead == '\n') {
        cLineNo++;
        iState = 1;
      }
      break;
    case 6:
      if (chRead == '\n') {
        *pchDirPar = HB_EOS;
        if (hb_ArgFillFromFile(local->szDirPar,ppargFirst)) {
          hb_Free(local);
          return 4;
        }
        cLineNo++;
        iState = 1;
      }
      else {
        *pchDirPar++ = chRead;
      }
      break;
    case 7: /* var. name start in attr */
      if (chRead == HB_ARG_ISVAR) {
        /* add filename */
        pch = local->szN;
        while (*pch != HB_EOS) {
          *pchAttr++ = *pch++;
        }
        *pchAttr++ = HB_ARG_VARSEP;
        iVarLevel++;
        iState = 1;
      }
      break;
    case 8: /* var. name start in val */
      if (chRead == HB_ARG_ISVAR) {
        /* add filename */
        pch = local->szN;
        while (*pch != HB_EOS) {
          *pchVal++ = *pch++;
        }
        *pchVal++ = HB_ARG_VARSEP;
        iVarLevel++;
        iState = 2;
      }
      break;
    default: ;
    } /* of switch */

  } /* of main reading loop */
  if (iResult < 0) { /* was hb_Read error */
    hb_Free(local);
    return 1;
  }

  /**/ /* fprintf(stdout,"^^^^^^^^^^^^^^^^^^^^^^^^^^^\n"); /**/
  /**/  /* paa = *ppargFirst;
  while (paa != NULL) {
    fprintf(stderr,"+++ %s = <%s>\n",paa->szAttr,paa->pszVal);
    paa = paa->paaNext;
  }
  /**/
  /**/ /* fprintf(stderr,"^^^^^^^^^^^^^^^^^^^^^^^^^^^\n"); */
  hb_Free(local);
  return 0;
} /* of hb_ArgFillFromFile */


/* public functions: */

FUNDEF1(int hb_ArgOpen,
char *, szName)
{
longint i;
longint iFree;
char *pszParm;
  /**/ /* fprintf(stderr,"hb_ArgOpen %s\n",szName); /**/
  /**/ /* fprintf(stderr,"hb_ArgOpen: 1 stack %u\n",stackavail()); */
  if (cargFiles == 0) {
    if (hb_ArgInit(HB_ARG_FILES_MAX_DEFAULT)) {
      return 1;
    }
  } /* of default init call */
  iFree = cargFiles;
  for (i = 0; i < cargFiles; i++) {
    if (rgargFile[i] == NULL) {
      iFree = i;
      break;
    }
  } /* searching for free space */
  if (iFree == cargFiles) {
    /* full :-( */
    hb_FatalError(HB_ARG,21,szName);
    return 2;
  }
  if (NULL == /* allocate file record */
      (rgargFile[iFree] = (hb_argFilePtr) hb_LongAlloc(sizeof(hb_argFileType)))) {
    hb_FatalError(HB_ARG,22,szName);
    return 3;
  }
  hb_ArgParseFileName(szName,
        rgargFile[iFree]->szFileName,HB_STRINGMAX,&pszParm);
  /**/ /* fprintf(stdout,"file %s, parm %s;\n",
                  rgargFile[iFree]->szFileName,pszParm); /**/
  rgargFile[iFree]->paaFirst = NULL;
  /**/ /* fprintf(stderr,"hb_ArgOpen: 2 stack %u\n",stackavail()); /**/
  hb_ArgFillFromFile(szName,&(rgargFile[iFree]->paaFirst));
  /**/ /* fprintf(stderr,"hb_ArgOpen: 3 stack %u\n",stackavail()); /**/
  return 0;
} /* of hb_ArgOpen */

FUNDEF1(int hb_ArgOpenWithLog,
char *, szName)
{
char szLogFileName[HB_STRINGMAX];
int iRC;
time_t tCurr;
char szTime[HB_STRINGMAX];
  iRC = hb_ArgOpen(szName);
  if (iRC != 0) return iRC;
  if (hb_ArgGetValue(szName,HB_ARGLOGFILENAME,szLogFileName,
        HB_STRINGMAX-2) > 0) {
    hb_OpenLog(NULL);
    iRC = 101;
  }
  else {
    hb_OpenLog(szLogFileName);
  }
  tCurr = time(NULL);
  strcpy(szTime,ctime(&tCurr));
  szTime[strlen(szTime)-1] = HB_EOS; /* delete \n */
  fprintf(stderr,"%s Logfile %s opened.\n",szTime,hb_GetLogFileName());
  if (iRC == 101) {
    fprintf(stderr,
      "%s: GetValue %s failed, this is default logfile (%s).\n",
          "hb_arg",HB_ARGLOGFILENAME,HB_STDERRLOG);
  }
  return iRC;
} /* of hb_ArgOpenWithLog */

FUNDEF1(int hb_ArgInit,
longint, cMaxFiles)
{
longint i;
  if (cargFiles != 0 || rgargFile != NULL) {
    hb_FatalError(HB_ARG,19,"");
    return 1;
  }
  cargFiles = cMaxFiles;
  if (NULL == (rgargFile = 
      (hb_argFilePtr *) hb_LongAlloc(sizeof(hb_argFileType)*cargFiles))) {
    sprintf(szT,"%ld",cMaxFiles);
    hb_FatalError(HB_ARG,20,szT);
    return 2;
  }
  for (i = 0; i < cargFiles; i++) {
    rgargFile[i] = NULL; /* init to NULL */
  }
  return 0;
} /* of hb_ArgInit */

FUNDEF0(int hb_ArgClose)
{
  if (rgargFile != NULL) {
    /**/
    free(rgargFile);
  }
  cargFiles = 0;
  return 0;
} /* of hb_ArgClose */

FUNDEF4(int hb_ArgGetValue, 
char *, szArgFileName,
char *, szAttr,
char *, szResult,
shortint, cMaxResult)
{
struct Local {
 char szAttrSubst[HB_LONGSTRINGMAX];
} *local;
longint i;
longint iFound;
hb_argAttrPtr paa1;
shortint iResCode;

    /**/ /* fprintf(stdout,"coreleft: %ld\n",coreleft()); /**/

  if ((local = hb_LongAlloc(sizeof(struct Local))) == NULL) {
    hb_FatalError(HB_ARG,999,"local alloc");
    return 999;
  }
  /* find file in list of opened files */
  for (iFound = 0; iFound < cargFiles; iFound++) {
    if (rgargFile[iFound] != NULL &&
        !strcmp(rgargFile[iFound]->szFileName,szArgFileName)) break;
  }
  if (iFound == cargFiles) { /* not found */
    strcpy(szT,szArgFileName);
    strcat(szT,"/");
    strcat(szT,szAttr);
    hb_FatalError(HB_ARG,28,szT);
    hb_Free(local);
    return 1;
  }
  /* search arg list, expanding names */
  *(local->szAttrSubst) = HB_ARG_SVAR;
  strcpy(local->szAttrSubst+1,szAttr);
  local->szAttrSubst[strlen(local->szAttrSubst)+1] = HB_EOS;
  local->szAttrSubst[strlen(local->szAttrSubst)] = HB_ARG_EVAR;
  paa1 = rgargFile[iFound]->paaFirst;

  /* expand val and return it */
  /***/ /* fprintf(stdout,
  "GetValue before subst: szAttr = %s, local->szAttrSubst = %s\n",
            szAttr,local->szAttrSubst); /**/
  cSubstLevel = 0;
  if ((iResCode = 
       hb_ArgSubstitute(local->szAttrSubst,paa1,szResult,cMaxResult)) > 0) {
    hb_FatalError(HB_ARG,28,szAttr); /**/
    hb_Free(local);
    return 2;
  }
  else if (iResCode < 0) {
    hb_Free(local);
    return 3;
  }
  else {
    hb_Free(local);
    return 0;
  }
} /* of hb_ArgGetValue */

FUNDEF3(int hb_ArgSetValue, 
char *, szArgFileName,
char *, szAttr,
char *, szVal)
{
longint i;
longint iFound;
hb_argAttrPtr paa;
shortint iResCode;

  /* find file in list of opened files */
  for (iFound = 0; iFound < cargFiles; iFound++) {
    if (rgargFile[iFound] != NULL &&
        !strcmp(rgargFile[iFound]->szFileName,szArgFileName)) break;
  }
  if (iFound == cargFiles) { /* not found */
    strcpy(szT,szArgFileName);
    hb_FatalError(HB_ARG,28,szT);
    return 1;
  }
  /* insert as the first attribute */
  if (NULL == (paa = (hb_argAttrPtr) 
      hb_LongAlloc(sizeof(hb_argAttrType)))) {
    hb_FatalError(HB_ARG,24,szAttr);
    return 2;
  }
  if (NULL == (paa->szAttr = (char *)
    hb_LongAlloc(sizeof(char)*(strlen(szAttr)+strlen(szVal)+2)))) {
    hb_FatalError(HB_ARG,24,szAttr);
    return 3;
  }
  paa->pszVal = paa->szAttr + strlen(szAttr) + 1;
  strcpy(paa->szAttr,szAttr);
  strcpy(paa->pszVal,szVal);
  paa->fLocked = 0;
  paa->paaNext = rgargFile[iFound]->paaFirst;
  rgargFile[iFound]->paaFirst = paa;
  return 0;
} /* of hb_ArgSetValue */

FUNDEF1(shortint hb_ArgCountFields,
char *, szValue)
{
char *pch;
shortint cFields;
  cFields = 0;
  pch = szValue;
  if (pch != NULL) {
    if (*pch != HB_EOS) cFields = 1;
    while (*pch != HB_EOS) {
      if (*pch++ == HB_ARG_FS) cFields++;
    }
    return cFields;
  }
  else return -1;
} /* of hb_ArgCountFields */

FUNDEF4(int hb_ArgGetField,
char *, szValue,
shortint, iField,
char *, szResult,
shortint, cMaxResult)
{
char *pch;
shortint cFields;
shortint iActField;
shortint iResult;
  /* indexed from zero as in C */
  cFields = hb_ArgCountFields(szValue);
  *szResult = HB_EOS;
  if (iField < 0 || iField >= cFields) {
    strcpy(szT,szValue);
    pch = szT;
    while (*pch != HB_EOS) { if (*pch == HB_ARG_FS) *pch = ' '; pch++; }
    strcat(szT,"[");
    sprintf(szNum,"%d",iField);
    strcat(szT,szNum);
    strcat(szT,"]");
    hb_FatalError(HB_ARG,29,szT);
    return -1;
  }
  pch = szValue;
  iActField = 0;
  if (iActField != iField) {
    if (pch != NULL) {
      while (*pch != HB_EOS) {
        if (*pch++ == HB_ARG_FS) {
          if (++iActField == iField) break;
        }
      }
    }
    else return -2; /* should not happen, hb_ArgCountFields would fail */
  }
  /* found here (must be the case); pch == start */
  iResult = 0;
  while (*pch != HB_EOS && *pch != HB_ARG_FS) {
    if (iResult < cMaxResult) {
      szResult[iResult++] = *pch;
    }
    pch++;
  }
  szResult[iResult] = HB_EOS;
  return 0;
} /* of hb_ArgCountFields */


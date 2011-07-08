/****************************************************************/
/* lingv. moduly - spol. cast #include                          */
/* ISO Latin 2 (8859-2) case routines                           */
/* Changelog:                                                   */
/* v0.1 JH  98/04/12                                            */
/*          pocatecni verze                                     */
/****************************************************************/

#include <stddef.h>
#include <string.h>
#include <stdio.h>

#include "hb_proto.h"
#include "hb_base.h"
#include "hb_il2ca.h"

char *hb_szUpperCase = "®©ÈØÁÉÍÓÚÝÏ«ÒÌÙÀÅ¦¬¥£¡ÂÃÄª¯ÆÇÊËÎÐÑÔÕÖÛÜÞß";
char *hb_szLowerCase = "¾¹èøáéíóúýï»òìùàå¶¼µ³±âãäº¿æçêëîðñôõöûüþß";

FUNDEF1(char hb_ToLower,
char, c)
{
char *pchFound;
  if (c >= 'A' && c <= 'Z') return (c - 'A' + 'a');
  else {
    if ((pchFound = strchr(hb_szUpperCase,c)) != NULL) {
      return (hb_szLowerCase[pchFound-hb_szUpperCase]);
    }
    else return c;
  }
} /* of hb_ToLower */

FUNDEF1(char hb_ToUpper,
char, c)
{
char *pchFound;
  if (c >= 'a' && c <= 'z') return (c - 'a' + 'A');
  else {
    if ((pchFound = strchr(hb_szLowerCase,c)) != NULL) {
      return (hb_szUpperCase[pchFound-hb_szLowerCase]);
    }
    else return c;
  }
} /* of hb_ToUpper */

FUNDEF1(int hb_IsUpper,
char, c)
{
  if (c >= 'A' && c <= 'Z') return 1;
  if (strchr(hb_szUpperCase,c) != NULL) return 1; else return 0;
} /* of hb_IsUpper */

FUNDEF1(int hb_IsLower,
char, c)
{
  if (c >= 'a' && c <= 'z') return 1;
  if (c == '§' ||
      c == 'ß' ||
      c == '×' ||
      c == '÷') return 1;
  if (strchr(hb_szLowerCase,c) != NULL) return 1; else return 0;
} /* of hb_IsLower */

FUNDEF1(int hb_IsDigit,
char, c)
{
  if (c >= '0' && c <= '9') return 1; else return 0;
} /* of hb_IsLower */

FUNDEF1(int hb_GetCase,
char *, sz)
{
char *pch;
int iCase;
int fAllUpper;
int fFirstUpper;
int fDigit;
int fAllDigit;
int fDot;
int fOtherUpper;

  pch = sz;
  fFirstUpper = hb_IsUpper(*pch);
  fAllUpper = 1;
  fDigit = 0;
  fAllDigit = 1;
  fDot = 0;
  fOtherUpper = 0;
  while (*pch != HB_EOS) {
    if (hb_IsLower(*pch)) fAllUpper = 0;
    if (hb_IsUpper(*pch)) {
      if (pch != sz) { /* not the first position */
        fOtherUpper = 1;
      }
    }
    if (hb_IsDigit(*pch)) {
      fDigit = 1;
    }
    else {
      if (*pch == '.') {
        if (fDot) {
          /* second dot, not a number: */
          fAllDigit = 0;
        }
        else { /* ok, first dot */
          fDot = 1;
        }
      }
      else { /* not a dot */
        if (*pch == '-') {
          if (sz == pch) {
            /* ok, minus sign at the beginning */
	  }
          else { /* hyphen in the middle */
            fAllDigit = 0;
	  }
	}
        else { /* not a digit, not a dot, not a hyphen... */
          fAllDigit = 0;
	}
      }
    } /* of not digit */
    pch++;
  }
  switch (8*fFirstUpper + 4*fAllUpper + 2*fDigit + fAllDigit) {
    case 0:
      if (fOtherUpper) 
        iCase = HB_MIXED; /* uppercase somewhere... */
      else
        iCase = HB_LOWER;
      break;
    case 1:
      iCase = HB_MIXED; /* cannot happen, anyway, unless just a . or -. */
      break;
    case 2:
      iCase = HB_MIXED; /* otherwise some lowercase letters */
      break;
    case 3:
      iCase = HB_UNDEFCASE; /* should not happen -- fAllUpper set, too */
      break;
    case 4:
      iCase = HB_UNDEFCASE; /* cannot happen, unless punctuation */
      break;
    case 5:
      iCase = HB_MIXED; /* precedence over upper, only . or -. */
      break;
    case 6:
      iCase = HB_MIXED; /* precedence over upper, of course */
      break;
    case 7:
      iCase = HB_NUM; /* precedence over upper, of course */
      break;
    case 8:
      if (fOtherUpper) 
        iCase = HB_MIXED; /* uppercase somewhere in the middle... */
      else
        iCase = HB_CAP;
      break;
    case 9:
      iCase = HB_UNDEFCASE; /* cannot happen (cap & alldigits) */ 
      break;
    case 10:
      iCase = HB_MIXED; /* precedence over cap */
      break;
    case 11:
      iCase = HB_UNDEFCASE; /* cannot happen (cap, digit & alldigits...) */ 
      break;
    case 12:
      if (strlen(sz) == 1)
        iCase = HB_CAP; /* cap has precedence over upper for 1-letter word */
      else
        iCase = HB_UPPER;
      break;
    case 13:
      iCase = HB_UNDEFCASE; /* cannot happen (cap, allupper, & alldigits) */ 
      break;
    case 14:
      iCase = HB_MIXED; /* (cap, allupper, & digit) */ 
      break;
    case 15:
      iCase = HB_UNDEFCASE; /* cannot happen (all set...) */ 
      break;
  }
  return iCase;
} /* of hb_GetCase */

FUNDEF3(int hb_ToCase,
char *, szOut,
char *, szIn,
int, iCase)
{
char *pch;

  strcpy(szOut,szIn);
  if (iCase == HB_LOWER) {
    pch = szOut;
    while (*pch != HB_EOS) {
      *pch = hb_ToLower(*pch);
      pch++;
    }
  }
  else if (iCase == HB_CAP) {
    pch = szOut;
    while (*pch != HB_EOS) {
      *pch = hb_ToLower(*pch);
      pch++;
    }
    *szOut = hb_ToUpper(*szOut);
  }
  else if (iCase == HB_UPPER) {
    pch = szOut;
    while (*pch != HB_EOS) {
      *pch = hb_ToUpper(*pch);
      pch++;
    }
  }
  /* else do nothing */
  return 0;
} /* of hb_ToCase */

FUNDEF1(char * hb_CaseName,
int, iCase)
{
  switch (iCase) {
    case HB_LOWER:
      return "lower";
    case HB_CAP:
      return "cap";
    case HB_UPPER:
      return "upper";
    case HB_MIXED:
      return "mixed";
    case HB_NUM:
      return "num";
    case HB_UNDEFCASE:
      return "undefcase";
  }
  return "";
}

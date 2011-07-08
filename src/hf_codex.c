#include <stdio.h>
#include <string.h>
#include "hb_proto.h"

#include "hf_codex.h"

#define HF_CODE_NEG 255
#define HF_CODE_BASE 215
#define HF_CODE_BASE_LONG 215l
#define HF_P11 249
#define HF_P12 250
#define HF_P13 251
#define HF_P2 252
#define HF_P3 253
#define HF_P4 254
#define HF_CODE_MAX 2136750625l
#define HF_LB_P11 215
#define HF_LB_P12 430
#define HF_LB_P13 645
#define HF_LB_P2 860

#define ShiftCode(x) ((x) + HF_FIRST_CHAR)
#define ShiftDecode(x) ((x) - HF_FIRST_CHAR)


FUNDEF2(int hf_CodeXData,
char *, szResult,
longint, iCode)
{
unsigned char szR[6];
int iPos;
unsigned char b;
longint iRest;
unsigned char iLow;

  if (iCode < 0) {
    *szR = HF_CODE_NEG;
    szR[1] = HF_EOS;
  }
  else if (iCode < HF_LB_P11) {
    b = iCode;
    *szR = ShiftCode(b);
    szR[1] = HF_EOS;
  }
  else if (iCode < HF_LB_P12) {
    *szR = HF_P11;
    b = iCode - HF_LB_P11;
    szR[1] = ShiftCode(b);
    szR[2] = HF_EOS;
  }
  else if (iCode < HF_LB_P13) {
    *szR = HF_P12;
    b = iCode - HF_LB_P12;
    szR[1] = ShiftCode(b);
    szR[2] = HF_EOS;
  }
  else if (iCode < HF_LB_P2) {
    *szR = HF_P13;
    b = iCode - HF_LB_P13;
    szR[1] = ShiftCode(b);
    szR[2] = HF_EOS;
  }
  else { /* two bytes min., greater or eq to HF_LB_P2 */
    iRest = iCode;
    iPos = 1;
    while (iRest > 0) {
      iLow = iRest % HF_CODE_BASE;
      iRest = iRest / HF_CODE_BASE;
      szR[iPos++] = ShiftCode(iLow);
    }
    if (iPos > 5) {
      fprintf(stderr,"hfFE"); exit(123);
    }
    szR[iPos] = HF_EOS;
    szR[0] = HF_P2 + iPos - 3;
  }
  strcpy(szResult,(char *)szR);
  return strlen(szResult);
} /* of hf_CodeXData */

FUNDEF2(int hf_DecodeXData,
longint *, iResult,
char *, szCode)
{
unsigned char cPrefix;
unsigned char *bzCode;

  /* returns length of code */
  /* szCode need not end by 0 !! */
  bzCode = (unsigned char *) szCode;
  cPrefix = *bzCode;

  if (cPrefix == HF_CODE_NEG) {
    *iResult = -1;
    return 1;
  }
  else if (cPrefix == HF_P11) {
    *iResult = ShiftDecode(bzCode[1]) + HF_LB_P11;
    return 2;
  }
  else if (cPrefix == HF_P12) {
    *iResult = ShiftDecode(bzCode[1]) + HF_LB_P12;
    return 2;
  }
  else if (cPrefix == HF_P13) {
    *iResult = ShiftDecode(bzCode[1]) + HF_LB_P13;
    return 2;
  }
  else if (cPrefix == HF_P2) {
    *iResult = ShiftDecode(bzCode[1]) +
                ShiftDecode(bzCode[2]) *
                HF_CODE_BASE_LONG;
    return 3;
  }
  else if (cPrefix == HF_P3) {
    *iResult = ShiftDecode(bzCode[1]) +
               (ShiftDecode(bzCode[2]) +
                 ShiftDecode(bzCode[3]) *
                 HF_CODE_BASE_LONG) *
                HF_CODE_BASE_LONG;
    return 4;
  }
  else if (cPrefix == HF_P4) {
    *iResult = ShiftDecode(bzCode[1]) +
               (ShiftDecode(bzCode[2]) +
                (ShiftDecode(bzCode[3]) +
                  ShiftDecode(bzCode[4]) *
                  HF_CODE_BASE_LONG) *
                 HF_CODE_BASE_LONG) *
                HF_CODE_BASE_LONG;
    return 5;
  }
  else {
    *iResult = ShiftDecode(bzCode[0]);
    if (*iResult < 0) {
      *iResult = -2;
      return 0;
    }
    return 1;
  }
} /* of hf_DecodeXData */


#ifndef __HB_IL2CAH
#define __HB_IL2CAH

/****************************************************************/
/* lingv. moduly - spol. cast #include                          */
/* ISO Latin 2 (8859-2) case routines                           */
/* Changelog:                                                   */
/* v0.1 JH  98/04/12                                            */
/*          pocatecni verze                                     */
/****************************************************************/

#include <stddef.h>

#include "hb_proto.h"
#include "hb_base.h"

#define HB_CAP 1
#define HB_UPPER 2
#define HB_MIXED 3
#define HB_LOWER 4
#define HB_NUM 5
#define HB_UNDEFCASE 0

PROTO(int hb_IsUpper, (char c));
PROTO(int hb_IsLower, (char c));
PROTO(char hb_ToUpper, (char c));
PROTO(char hb_ToLower, (char c));
PROTO(int hb_IsDigit, (char c));
PROTO(int hb_GetCase, (char *sz));
PROTO(int hb_ToCase, (char *szOut, char *szIn, int iCase));
PROTO(char *hb_CaseName, (int iCase));

#endif

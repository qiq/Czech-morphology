/**********************************************************/
/*                                                        */
/* conversion tables - common interface                   */
/*                                                        */
/*                                                        */
/*                                                        */
/* (C) Jan Hajic 1997                                     */
/*                                                        */
/**********************************************************/

#ifndef __HH_CPTAB
#define __HH_CPTAB

#include "hb_proto.h"

extern unsigned char *hh_tab;
extern unsigned char hh_inv_tab[];

#if defined(__cplusplus)
extern "C" {
#endif
PROTO(int hhSetTabFillInv, (char *szCharSet));
#if defined(__cplusplus)
}
#endif

#endif

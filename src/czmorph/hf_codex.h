#include "hb_proto.h"
#include "hb_base.h"

PROTO(int hf_CodeXData, (char *szResult, longint iCode));
PROTO(int hf_DecodeXData, (longint *iResult, char *szCode));


#define HF_EOS '\0'
#define HF_FIRST_CHAR ' '


#if !defined(__HHPROTO_H)

#define __HHPROTO_H

#include "proto.h"

#if defined(_hea)
#define FUNDEF0(function) function (void)
#define FUNDEF1(function,type1,arg1) function (type1 arg1)
#define FUNDEF2(function,type1,arg1,type2,arg2) \
    function (type1 arg1, type2 arg2)
#define FUNDEF3(function,type1,arg1,type2,arg2,type3,arg3) \
    function (type1 arg1, type2 arg2, type3 arg3)
#define FUNDEF4(function,type1,arg1,type2,arg2,type3,arg3,type4,arg4) \
    function (type1 arg1, type2 arg2, type3 arg3, type4 arg4)
#define FUNDEF5(function,type1,arg1,type2,arg2,type3,arg3,type4,arg4,type5,arg5) \
    function (type1 arg1, type2 arg2, type3 arg3, type4 arg4, \
        type5 arg5)
#define FUNDEF6(function,type1,arg1,type2,arg2,type3,arg3,type4,arg4,type5,arg5,type6,arg6) \
    function (type1 arg1, type2 arg2, type3 arg3, type4 arg4, \
        type5 arg5, type6 arg6)
#define FUNDEF7(function,type1,arg1,type2,arg2,type3,arg3,type4,arg4,type5,arg5,type6,arg6,type7,arg7) \
    function (type1 arg1, type2 arg2, type3 arg3, type4 arg4, \
        type5 arg5, type6 arg6, type7 arg7)
#define FUNDEF8(function,type1,arg1,type2,arg2,type3,arg3,type4,arg4,type5,arg5,type6,arg6,type7,arg7,type8,arg8) \
    function (type1 arg1, type2 arg2, type3 arg3, type4 arg4, \
        type5 arg5, type6 arg6, type7 arg7, type8 arg8)
#else
#define FUNDEF0(function) function ()
#define FUNDEF1(function,type1,arg1) function (arg1) type1 arg1;
#define FUNDEF2(function,type1,arg1,type2,arg2) \
    function (arg1,arg2) type1 arg1; type2 arg2;
#define FUNDEF3(function,type1,arg1,type2,arg2,type3,arg3) \
    function (arg1,arg2,arg3) type1 arg1; type2 arg2; type3 arg3;
#define FUNDEF4(function,type1,arg1,type2,arg2,type3,arg3,type4,arg4) \
    function (arg1,arg2,arg3,arg4) \
    type1 arg1; type2 arg2; type3 arg3; type4 arg4;
#define FUNDEF5(function,type1,arg1,type2,arg2,type3,arg3,type4,arg4,type5,arg5) \
    function (arg1,arg2,arg3,arg4,arg5) \
        type1 arg1; type2 arg2; type3 arg3; type4 arg4; \
        type5 arg5;
#define FUNDEF6(function,type1,arg1,type2,arg2,type3,arg3,type4,arg4,type5,arg5,type6,arg6) \
    function (arg1,arg2,arg3,arg4,arg5,arg6) \
        type1 arg1; type2 arg2; type3 arg3; type4 arg4; \
        type5 arg5; type6 arg6;
#define FUNDEF7(function,type1,arg1,type2,arg2,type3,arg3,type4,arg4,type5,arg5,type6,arg6,type7,arg7) \
    function (arg1,arg2,arg3,arg4,arg5,arg6,arg7) \
        type1 arg1; type2 arg2; type3 arg3; type4 arg4; \
        type5 arg5; type6 arg6; type7 arg7;
#define FUNDEF8(function,type1,arg1,type2,arg2,type3,arg3,type4,arg4,type5,arg5,type6,arg6,type7,arg7,type8,arg8) \
    function (arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8) \
        type1 arg1; type2 arg2; type3 arg3; type4 arg4; \
        type5 arg5; type6 arg6; type7 arg7; type8 arg8;
#endif

#endif

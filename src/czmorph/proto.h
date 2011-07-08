#if !defined(__PROTO_H)

#define __PROTO_H

#if defined(__STDC__) || defined(__MSDOS__)
#define _hea
#endif

#if !defined(PROTO)
/* Dwa typy prototypu: K+R a ANSI */
#if defined(_hea)
#define PROTO(function,args) function args
#define DOTS ,...
#else
#define PROTO(function,args) function()
#define DOTS
#endif
#endif

#endif

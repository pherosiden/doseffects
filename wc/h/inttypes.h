/***************************************************************************
 * FILE: inttypes.h/cinttypes (Format conversion of integer types)
 *
 * =========================================================================
 *
 *                          Open Watcom Project
 *
 * Copyright (c) 2004-2020 The Open Watcom Contributors. All Rights Reserved.
 * Portions Copyright (c) 1983-2002 Sybase, Inc. All Rights Reserved.
 *
 *    This file is automatically generated. Do not edit directly.
 *
 * =========================================================================
 *
 * Description: This header is part of the C99/C++ standard library.
 *              It declares various macros to help format stdint integers.
 ***************************************************************************/
#ifndef _INTTYPES_H_INCLUDED
#define _INTTYPES_H_INCLUDED

#ifndef _ENABLE_AUTODEPEND
 #pragma read_only_file
#endif

#ifdef __cplusplus

#include <cinttypes>

// C99 types in inttypes.h.
using std::imaxdiv_t;

// C99 functions in inttypes.h.
using std::imaxabs;
using std::imaxdiv;
using std::strtoimax;
using std::strtoumax;
using std::wcstoimax;
using std::wcstoumax;

#else /* __cplusplus not defined */

#ifndef __COMDEF_H_INCLUDED
 #include <_comdef.h>
#endif

#ifndef _STDINT_H_INCLUDED
 #include <stdint.h>
#endif

/* Format macros */

#define PRId8   "hhd"
#define PRId16  "hd"
#define PRId32  "ld"
#define PRId64  "lld"
#define PRIi8   "hhi"
#define PRIi16  "hi"
#define PRIi32  "li"
#define PRIi64  "lli"
#define PRIo8   "hho"
#define PRIo16  "ho"
#define PRIo32  "lo"
#define PRIo64  "llo"
#define PRIu8   "hhu"
#define PRIu16  "hu"
#define PRIu32  "lu"
#define PRIu64  "llu"
#define PRIx8   "hhx"
#define PRIx16  "hx"
#define PRIx32  "lx"
#define PRIx64  "llx"
#define PRIX8   "hhX"
#define PRIX16  "hX"
#define PRIX32  "lX"
#define PRIX64  "llX"
#define SCNd8   "hhd"
#define SCNd16  "hd"
#define SCNd32  "ld"
#define SCNd64  "lld"
#define SCNi8   "hhi"
#define SCNi16  "hi"
#define SCNi32  "li"
#define SCNi64  "lli"
#define SCNo8   "hho"
#define SCNo16  "ho"
#define SCNo32  "lo"
#define SCNo64  "llo"
#define SCNu8   "hhu"
#define SCNu16  "hu"
#define SCNu32  "lu"
#define SCNu64  "llu"
#define SCNx8   "hhx"
#define SCNx16  "hx"
#define SCNx32  "lx"
#define SCNx64  "llx"
#define SCNX8   "hhX"
#define SCNX16  "hX"
#define SCNX32  "lX"
#define SCNX64  "llX"

#define PRIdLEAST8   "hhd"
#define PRIdLEAST16  "hd"
#define PRIdLEAST32  "ld"
#define PRIdLEAST64  "lld"
#define PRIiLEAST8   "hhi"
#define PRIiLEAST16  "hi"
#define PRIiLEAST32  "li"
#define PRIiLEAST64  "lli"
#define PRIoLEAST8   "hho"
#define PRIoLEAST16  "ho"
#define PRIoLEAST32  "lo"
#define PRIoLEAST64  "llo"
#define PRIuLEAST8   "hhu"
#define PRIuLEAST16  "hu"
#define PRIuLEAST32  "lu"
#define PRIuLEAST64  "llu"
#define PRIxLEAST8   "hhx"
#define PRIxLEAST16  "hx"
#define PRIxLEAST32  "lx"
#define PRIxLEAST64  "llx"
#define PRIXLEAST8   "hhX"
#define PRIXLEAST16  "hX"
#define PRIXLEAST32  "lX"
#define PRIXLEAST64  "llX"
#define SCNdLEAST8   "hhd"
#define SCNdLEAST16  "hd"
#define SCNdLEAST32  "ld"
#define SCNdLEAST64  "lld"
#define SCNiLEAST8   "hhi"
#define SCNiLEAST16  "hi"
#define SCNiLEAST32  "li"
#define SCNiLEAST64  "lli"
#define SCNoLEAST8   "hho"
#define SCNoLEAST16  "ho"
#define SCNoLEAST32  "lo"
#define SCNoLEAST64  "llo"
#define SCNuLEAST8   "hhu"
#define SCNuLEAST16  "hu"
#define SCNuLEAST32  "lu"
#define SCNuLEAST64  "llu"
#define SCNxLEAST8   "hhx"
#define SCNxLEAST16  "hx"
#define SCNxLEAST32  "lx"
#define SCNxLEAST64  "llx"
#define SCNXLEAST8   "hhX"
#define SCNXLEAST16  "hX"
#define SCNXLEAST32  "lX"
#define SCNXLEAST64  "llX"

#define PRIdFAST8   "d"
#define PRIdFAST16  "d"
#define PRIdFAST32  "ld"
#define PRIdFAST64  "lld"
#define PRIiFAST8   "i"
#define PRIiFAST16  "i"
#define PRIiFAST32  "li"
#define PRIiFAST64  "lli"
#define PRIoFAST8   "o"
#define PRIoFAST16  "o"
#define PRIoFAST32  "lo"
#define PRIoFAST64  "llo"
#define PRIuFAST8   "u"
#define PRIuFAST16  "u"
#define PRIuFAST32  "lu"
#define PRIuFAST64  "llu"
#define PRIxFAST8   "x"
#define PRIxFAST16  "x"
#define PRIxFAST32  "lx"
#define PRIxFAST64  "llx"
#define PRIXFAST8   "X"
#define PRIXFAST16  "X"
#define PRIXFAST32  "lX"
#define PRIXFAST64  "llX"
#define SCNdFAST8   "d"
#define SCNdFAST16  "d"
#define SCNdFAST32  "ld"
#define SCNdFAST64  "lld"
#define SCNiFAST8   "i"
#define SCNiFAST16  "i"
#define SCNiFAST32  "li"
#define SCNiFAST64  "lli"
#define SCNoFAST8   "o"
#define SCNoFAST16  "o"
#define SCNoFAST32  "lo"
#define SCNoFAST64  "llo"
#define SCNuFAST8   "u"
#define SCNuFAST16  "u"
#define SCNuFAST32  "lu"
#define SCNuFAST64  "llu"
#define SCNxFAST8   "x"
#define SCNxFAST16  "x"
#define SCNxFAST32  "lx"
#define SCNxFAST64  "llx"
#define SCNXFAST8   "X"
#define SCNXFAST16  "X"
#define SCNXFAST32  "lX"
#define SCNXFAST64  "llX"

#define PRIdMAX     "lld"
#define PRIiMAX     "lli"
#define PRIoMAX     "llo"
#define PRIuMAX     "llu"
#define PRIxMAX     "llx"
#define PRIXMAX     "llX"
#define SCNdMAX     "lld"
#define SCNiMAX     "lli"
#define SCNoMAX     "llo"
#define SCNuMAX     "llu"
#define SCNxMAX     "llx"
#define SCNXMAX     "llX"

#ifdef _M_I86
#if defined(__SMALL__) || defined(__MEDIUM__)
#define PRIdPTR     "d"
#define PRIiPTR     "i"
#define PRIoPTR     "o"
#define PRIuPTR     "u"
#define PRIxPTR     "x"
#define PRIXPTR     "X"
#define SCNdPTR     "d"
#define SCNiPTR     "i"
#define SCNoPTR     "o"
#define SCNuPTR     "u"
#define SCNxPTR     "x"
#define SCNXPTR     "X"
#else
#define PRIdPTR     "ld"
#define PRIiPTR     "li"
#define PRIoPTR     "lo"
#define PRIuPTR     "lu"
#define PRIxPTR     "lx"
#define PRIXPTR     "lX"
#define SCNdPTR     "ld"
#define SCNiPTR     "li"
#define SCNoPTR     "lo"
#define SCNuPTR     "lu"
#define SCNxPTR     "lx"
#define SCNXPTR     "lX"
#endif
#else /* 32 bit */
#if defined(__COMPACT__) || defined(__LARGE__)
#define PRIdPTR     "lld"
#define PRIiPTR     "lli"
#define PRIoPTR     "llo"
#define PRIuPTR     "llu"
#define PRIxPTR     "llx"
#define PRIXPTR     "llX"
#define SCNdPTR     "lld"
#define SCNiPTR     "lli"
#define SCNoPTR     "llo"
#define SCNuPTR     "llu"
#define SCNxPTR     "llx"
#define SCNXPTR     "llX"
#else
#define PRIdPTR     "ld"
#define PRIiPTR     "li"
#define PRIoPTR     "lo"
#define PRIuPTR     "lu"
#define PRIxPTR     "lx"
#define PRIXPTR     "lX"
#define SCNdPTR     "ld"
#define SCNiPTR     "li"
#define SCNoPTR     "lo"
#define SCNuPTR     "lu"
#define SCNxPTR     "lx"
#define SCNXPTR     "lX"
#endif
#endif

#ifdef _M_IX86
 #pragma pack( __push, 1 )
#else
 #pragma pack( __push, 8 )
#endif

 #ifndef _WCHAR_T_DEFINED
 #define _WCHAR_T_DEFINED
  #define _WCHAR_T_DEFINED_
  typedef unsigned short wchar_t;
 #endif

typedef struct {
    intmax_t    quot;
    intmax_t    rem;
} imaxdiv_t;

_WCRTLINK extern intmax_t   imaxabs( intmax_t __j );
_WCRTLINK extern imaxdiv_t  imaxdiv( intmax_t __numer, intmax_t __denom );
_WCRTLINK extern intmax_t   strtoimax( const char *__nptr, char **__endptr, int __base );
_WCRTLINK extern uintmax_t  strtoumax( const char *__nptr, char **__endptr, int __base );
_WCRTLINK extern intmax_t   wcstoimax( const wchar_t *__nptr, wchar_t **__endptr, int __base );
_WCRTLINK extern uintmax_t  wcstoumax( const wchar_t *__nptr, wchar_t **__endptr, int __base );

#pragma pack( __pop )

#endif /* __cplusplus not defined */

#endif

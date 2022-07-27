/*
 *  env.h       Environment string operations
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
 */
#ifndef _ENV_H_INCLUDED
#define _ENV_H_INCLUDED

#ifndef _ENABLE_AUTODEPEND
 #pragma read_only_file
#endif

#ifndef __COMDEF_H_INCLUDED
 #include <_comdef.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* 
 *  ISO C types
 */
#ifndef __cplusplus
 #ifndef _WCHAR_T_DEFINED
 #define _WCHAR_T_DEFINED
  #define _WCHAR_T_DEFINED_
  typedef unsigned short wchar_t;
 #endif
#endif

/*
 *  POSIX 1003.1 Prototypes.
 */
_WCRTLINK extern int        clearenv( void );

#ifdef __cplusplus
namespace std {
#endif

_WCRTLINK extern char       *getenv( const char *__name );

#ifdef __cplusplus
} // namespace std
using std::getenv;
#endif

#if defined( _POSIX_SOURCE ) || !defined( _NO_EXT_KEYS ) /* extensions enabled */
_WCRTLINK extern int        putenv( const char *__env_string );
#endif /* extensions enabled */

_WCRTLINK extern int        setenv( const char *__name, const char *__newvalue, int __overwrite );
_WCRTLINK extern int        unsetenv( const char *__name );

_WCRTLINK extern int        _setenv( const char *__name, const char *__newvalue, int __overwrite );
_WCRTLINK extern wchar_t    *_wgetenv( const wchar_t *__name );
_WCRTLINK extern int        _wsetenv( const wchar_t *__name, const wchar_t *__newvalue, int __overwrite );
_WCRTLINK extern int        _wputenv( const wchar_t *__env_string );

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif

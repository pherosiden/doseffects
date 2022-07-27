/*
 *  jtime.h     Japanese time functions
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
#ifndef _JTIME_H_INCLUDED
#define _JTIME_H_INCLUDED

#ifndef _ENABLE_AUTODEPEND
 #pragma read_only_file
#endif

#ifndef __COMDEF_H_INCLUDED
 #include <_comdef.h>
#endif

#ifndef _TIME_H_INCLUDED
 #include <time.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

_WCRTLINK extern unsigned char   *jasctime( const struct tm *__timeptr );
_WCRTLINK extern unsigned char   *jctime( const time_t *__timer );

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif

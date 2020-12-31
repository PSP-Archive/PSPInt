/*
 * ============================================================================
 *  Title:    Project-Wide Config
 *  Author:   J. Zbiciak
 *  $Id: config.h,v 1.14 2001/11/02 02:00:02 im14u2c Exp $
 * ============================================================================
 *  _BIG_ENDIAN         -- Host machine is big endian
 *  _LITTLE_ENDIAN      -- Host machine is little endian
 * ============================================================================
 *
 * ============================================================================
 */

#ifndef _CONFIG_H
#define _CONFIG_H

#if defined (__APPLE__) && defined(__MACH__)
# define macosx
#endif

/*
 * ============================================================================
 *  If you get an error here, define _BIG_ENDIAN or _LITTLE_ENDIAN as 
 *  is required for your host machine!
 * ============================================================================
 */
#if (defined(sparc) || defined(macintosh) || defined(macosx)) && \
     !(defined(_LITTLE_ENDIAN) || defined(_BIG_ENDIAN))
# define _BIG_ENDIAN    
#endif

#if (defined(i386) || defined(WIN32) || defined(__i386__)) && \
     !(defined(_LITTLE_ENDIAN) || defined(_BIG_ENDIAN))
# define _LITTLE_ENDIAN
#endif

# if 0 //LUDO: 
#if !defined(_BIG_ENDIAN) && !defined(_LITTLE_ENDIAN)
#  include <endian.h>
/*#  include <bytesex.h>*/
#  ifndef __BYTE_ORDER
#    error Please manually set your machine endian in 'config.h'
#  endif
#  if __BYTE_ORDER==4321
#    define _BIG_ENDIAN
#  endif
#  if __BYTE_ORDER==1234
#    define _LITTLE_ENDIAN
#  endif
#  if !defined(_BIG_ENDIAN) && !defined(_LITTLE_ENDIAN)
#    error Unsupported __BYTE_ORDER.  Set your machine endian in 'config.h'.
#  endif
#endif
# else
# define _LITTLE_ENDIAN
# endif

#if defined(_BIG_ENDIAN) && defined(_LITTLE_ENDIAN)
#  error Both _BIG_ENDIAN and _LITTLE_ENDIAN defined.  Pick only 1!
#endif

/*
 * ============================================================================
 *  If we're using GCC/EGCS, suppress some silly compiler warnings.
 * ============================================================================
 */

#ifdef __GNUC__
# define UNUSED __attribute__ ((unused))
# define GNU_RESTRICT
#else
# define UNUSED
#endif

/*
 * ============================================================================
 *  If our compiler supports 'inline', enable it here.
 * ============================================================================
 */
#if defined(GNUC) || defined(_TMS320C6X)
# define INLINE inline
#else
# define INLINE
#endif

/*
 * ============================================================================
 *  Target-specific types, according to size.
 * ============================================================================
 */
#ifdef _TMS320C6X
typedef unsigned char       uint_8;
typedef unsigned short      uint_16;
typedef unsigned int        uint_32;
typedef unsigned long       uint_64;    /* Note:  Really only 40 bits.  */

typedef signed   char       sint_8;
typedef signed   short      sint_16;
typedef signed   int        sint_32;
typedef signed   long       sint_64;    /* Note:  Really only 40 bits.  */
#else
typedef unsigned char       uint_8;
typedef unsigned short      uint_16;
typedef unsigned int        uint_32;
typedef unsigned long long  uint_64;

typedef signed   char       sint_8;
typedef signed   short      sint_16;
typedef signed   int        sint_32;
typedef signed   long long  sint_64;
#endif

typedef volatile uint_8     v_uint_8;
typedef volatile uint_16    v_uint_16;
typedef volatile uint_32    v_uint_32;
typedef volatile uint_64    v_uint_64;

typedef volatile sint_8     v_sint_8;
typedef volatile sint_16    v_sint_16;
typedef volatile sint_32    v_sint_32;
typedef volatile sint_64    v_sint_64;


/*
 * ============================================================================
 *  Target-specific library compatibility issues
 * ============================================================================
 */

#ifdef linux
# define USE_STRCASECMP
#endif

#ifdef SOLARIS
/*# define NO_SNPRINTF*/
# define NO_GETOPT_LONG
# define NO_INOUT
# define USE_STRCASECMP
#endif

#ifdef WIN32
# define NO_GETTIMEOFDAY
/*# define NO_SNPRINTF*/
# define NO_GETOPT_LONG
# define NOGETOPT
# define NO_SETUID
# define NO_NANOSLEEP
# define NO_RAND48
# define NEED_INOUT
#endif

#ifdef macintosh
# define NO_GETTIMEOFDAY
# define NO_STRDUP
# define NO_SYS_TIME_H
# define NO_SDL
# define NO_SETUID
# define NO_NANOSLEEP
# define NO_RAND48
# define NO_INOUT
# define NO_STRICMP /* ? */
#endif

#ifdef macosx
# define NO_SDL
# define NO_RAND48
# define NO_INOUT
# define NO_GETOPT_LONG
# define NOGETOPT
# define USE_STRCASECMP /* ? */
#endif

#ifdef _TMS320C6X
# define NO_GETTIMEOFDAY
# define NO_STRDUP
# define NO_SYS_TIME_H
# define NO_UNISTD_H
# define NO_GETOPT_LONG
# define NO_SETUID
# define NO_NANOSLEEP
# define NO_RAND48
# define NO_INOUT
# define NO_STRICMP
# define HAVE_RESTRICT
# ifndef CLK_TCK
#  define CLK_TCK 200000000 /* Assume 200MHz C6201 device */
# endif
#endif

/*
 * ============================================================================
 *  Standard #includes that almost everyone needs
 * ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <assert.h>
#include <limits.h>
#include <errno.h>
#include <math.h>

#ifndef NO_SYS_TIME_H
# include <sys/time.h>
#endif

#ifndef NO_UNISTD_H
# include <unistd.h>
#endif

#ifdef _TMS320C6X           /* This seems to be TI-specific.    */
# include <file.h>
#endif

#ifdef linux
# include <sys/io.h>
#endif

# if 0 //LUDO: 
#ifdef NO_GETOPT_LONG
# include "plat/gnu_getopt.h"
#else
# include <getopt.h>
#endif
# endif

#ifdef USE_STRCASECMP
# define stricmp strcasecmp
#endif

#ifndef M_PI
# ifdef PI
#  define M_PI PI
# else
#  define M_PI (3.14159265358979323846)
# endif
#endif


/*
 * ============================================================================
 *  If this compiler implements the C99 'restrict' keyword, then enable it.
 * ============================================================================
 */
#ifdef HAVE_RESTRICT
# define RESTRICT restrict
#endif

#ifdef GNU_RESTRICT
# define RESTRICT __restrict__
#endif

#ifndef RESTRICT
# define RESTRICT
#endif

/*
 * ============================================================================
 *  Allow exposing "local" symbols by using LOCAL instead of static
 * ============================================================================
 */
#ifndef LOCAL
#define LOCAL static
#endif

/*
 * ============================================================================
 *  Include the "platform library" to handle missing functions
 * ============================================================================
 */

#include "plat/plat_lib.h"

#endif /* CONFIG_H */

/* ======================================================================== */
/*  This program is free software; you can redistribute it and/or modify    */
/*  it under the terms of the GNU General Public License as published by    */
/*  the Free Software Foundation; either version 2 of the License, or       */
/*  (at your option) any later version.                                     */
/*                                                                          */
/*  This program is distributed in the hope that it will be useful,         */
/*  but WITHOUT ANY WARRANTY; without even the implied warranty of          */
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU       */
/*  General Public License for more details.                                */
/*                                                                          */
/*  You should have received a copy of the GNU General Public License       */
/*  along with this program; if not, write to the Free Software             */
/*  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.               */
/* ======================================================================== */
/*                 Copyright (c) 1998-2003, Joseph Zbiciak                  */
/* ======================================================================== */

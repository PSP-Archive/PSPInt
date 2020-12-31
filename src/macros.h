
/*
 * ============================================================================
 *  MACROS:         Generally useful macros
 *
 *  Author:         J. Zbiciak
 * 
 *  $Id: macros.h,v 1.4 1999/05/03 08:02:31 im14u2c Exp $
 * ============================================================================
 *
 *  Available functionality implemented in this file:
 *
 *  HEXINT      -- Converts uppercase hex digit to int, or -1 if inval.  
 *  MASK_BIT    -- Converts an enum value (0..31) to a mask bit
 *
 *  PACK_STR    -- copies an ASCIIZ string to a header in "len + str" format
 *  PACK_I16    -- copies a 16-bit little-endian int into a header
 *  PACK_I32    -- copies a 32-bit little-endian int into a header
 *  PACK_RAW    -- copies raw data into a memory image
 *  ZERO_RAW    -- zeros out a region in a memory image
 *
 *  UNPACK_STR  -- copies a "len+str" format string into an ASCIIZ string
 *  UNPACK_I16  -- copies a 16-bit little-endian int out of a memory image
 *  UNPACK_I32  -- copies a 32-bit little-endian int out of a memory image
 *  UNPACK_RAW  -- copies raw data out of a memory image
 *
 *  Arguments for all pack/unpack macros:
 *      r       -- void pointer to raw data
 *      o       -- offset into raw data
 *      d       -- data (not present on ZERO_RAW)
 *      l       -- length of data (only on PACK_RAW, ZERO_RAW, UNPACK_RAW)
 *
 * ============================================================================
 */


#ifndef _MACRO_H
#define _MACRO_H

#include <string.h>     /* for memcpy(), strncpy()          */

/* ------------------------------------------------------------------------- */
/*  PACK_STR    -- copies an ASCIIZ string to a header in "len+str" format.  */
/* ------------------------------------------------------------------------- */

#define PACK_STR(r,o,d)                                     \
    do                                                      \
    {                                                       \
        unsigned char *data = (unsigned char *)r + (o);     \
        char *string = (char*)d;                            \
        int l;                                              \
                                                            \
        l = strlen(string);                                 \
        if (l > 12) l = 12;                                 \
        data[0] = l;                                        \
        strncpy(data + 1, string, 12);                      \
    } while (0)

#define PACK_I16(r,o,d)                                     \
    do                                                      \
    {                                                       \
        unsigned char *data = (unsigned char *)r + (o);     \
        unsigned val = (d);                                 \
                                                            \
        data[0] = ((val) & 0x000000FFU) >>  0;              \
        data[1] = ((val) & 0x0000FF00U) >>  8;              \
    } while (0)

#define PACK_I32(r,o,d)                                     \
    do                                                      \
    {                                                       \
        unsigned char *data = (unsigned char *)r + (o);     \
        unsigned val = (d);                                 \
                                                            \
        data[0] = ((val) & 0x000000FFU) >>  0;              \
        data[1] = ((val) & 0x0000FF00U) >>  8;              \
        data[2] = ((val) & 0x00FF0000U) >> 16;              \
        data[3] = ((val) & 0xFF000000U) >> 24;              \
    } while (0)

#define PACK_RAW(r,o,d,l)                                   \
     memcpy((void*)((char*)(r) + (o)), (const void*) (d), (size_t)(l))

#define ZERO_RAW(r,o,l)                                     \
     memset((void*)((char*)(r) + (o)), 0, (size_t)(l))


#define UNPACK_STR(r,o,d,l)                                 \
    do                                                      \
    {                                                       \
        unsigned char *data = (unsigned char *)r + (o);     \
                                                            \
        strncpy((char*) (d), data+1, (l));                  \
        (d)[12] = 0;                                        \
    } while (0)

#define UNPACK_I16(r,o,d)                                   \
    do                                                      \
    {                                                       \
        unsigned char *data = (unsigned char *)r + (o);     \
                                                            \
        (d) = (((unsigned) data[0] <<  0) & 0x000000FFU) |  \
              (((unsigned) data[1] <<  8) & 0x0000FF00U) ;  \
    } while (0)

#define UNPACK_I32(r,o,d)                                   \
    do                                                      \
    {                                                       \
        unsigned char *data = (unsigned char *)r + (o);     \
                                                            \
        (d) = (((unsigned) data[0] <<  0) & 0x000000FFU) |  \
              (((unsigned) data[1] <<  8) & 0x0000FF00U) |  \
              (((unsigned) data[2] << 16) & 0x00FF0000U) |  \
              (((unsigned) data[3] << 24) & 0xFF000000U);   \
    } while (0)

#define UNPACK_RAW(r,o,d,l)                                 \
     memcpy((void *)(d), (const void*)((char*)(r) + (o)), (size_t)(l))



/* --------------------------------------------------------------------- */
/*  HEXINT      -- Converts uppercase hex digit to int, or -1 if inval.  */
/* --------------------------------------------------------------------- */

#define HEXINT(c) ((((c)>='0') & ((c)<='9')) ? (c) - '0'      : \
                   (((c)>='A') & ((c)<='F')) ? (c) - 'A' + 10 : -1)



/* --------------------------------------------------------------------- */
/*  MASK_BIT    -- Converts an enum value (0..31) to a mask bit          */
/* --------------------------------------------------------------------- */

#define MASK_BIT(e) (1U<<(unsigned)(e))

/* --------------------------------------------------------------------- */
/*  BFE         -- Builds a `B'it`F'ield structure in the correct order  */
/*                 as required for the host machine's `E'ndian.          */
/* --------------------------------------------------------------------- */

#ifdef _BIG_ENDIAN
#define BFE(x,y) y; x
#endif

#ifdef _LITTLE_ENDIAN
#define BFE(x,y) x; y
#endif

#endif


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
/*                 Copyright (c) 1998-1999, Joseph Zbiciak                  */
/* ======================================================================== */

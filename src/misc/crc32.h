/* ======================================================================== */
/*  CRC-32 routines                                     J. Zbiciak, 2001    */
/* ------------------------------------------------------------------------ */
/*  This code is compatible with the CRC-32 that is used by the Zip file    */
/*  compression standard.  To use this code for that purpose, initialize    */
/*  your CRC to 0xFFFFFFFF, and XOR it with 0xFFFFFFFF afterwards.          */
/* ------------------------------------------------------------------------ */
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
/* ------------------------------------------------------------------------ */
/*                 Copyright (c) 2001-+Inf, Joseph Zbiciak                  */
/* ======================================================================== */
#ifndef CRC32_H_
#define CRC32_H_

/* ======================================================================== */
/*  CRC32_TBL    -- Lookup table used for the CRC-32 code.                  */
/* ======================================================================== */
extern uint_32 crc32_tbl[256];

/* ======================================================================== */
/*  CRC32_UPDATE -- Updates a 32-bit CRC using the lookup table above.      */
/*                  Note:  The 32-bit CRC is set up as a right-shifting     */
/*                  CRC with no inversions.                                 */
/*                                                                          */
/*                  All-caps version is a macro for stuff that can use it.  */
/* ======================================================================== */
uint_32 crc32_update(uint_32 crc, uint_8  data);
#define CRC32_UPDATE(c, d) ((c) >> 8 ^ crc32_tbl[((c) ^ (d)) & 0xFF])

/* ======================================================================== */
/*  CRC32_UPD16  -- Updates a 32-bit CRC using the lookup table above.      */
/*                  This function updates the CRC with a 16-bit value,      */
/*                  little-endian.                                          */
/* ======================================================================== */
uint_32 crc32_upd16(uint_32 crc, uint_16 data);

/* ======================================================================== */
/*  CRC32_UPD32  -- Updates a 32-bit CRC using the lookup table above.      */
/*                  This function updates the CRC with a 32-bit value,      */
/*                  little-endian.                                          */
/* ======================================================================== */
uint_32 crc32_upd32(uint_32 crc, uint_32 data);

/* ======================================================================== */
/*  CRC32_BLOCK  -- Updates a 32-bit CRC on a block of 8-bit data.          */
/*                  Note:  The 32-bit CRC is set up as a right-shifting     */
/*                  CRC with no inversions.                                 */
/* ======================================================================== */
uint_32 crc32_block(uint_32 crc, uint_8 *data, int len);

#endif
/* ======================================================================== */
/*                 Copyright (c) 2001-+Inf, Joseph Zbiciak                  */
/* ======================================================================== */

/*
 * ============================================================================
 *  Title:    File I/O Routines
 *  Author:   J. Zbiciak
 *  $Id: file.h,v 1.3 1999/09/26 11:44:42 im14u2c Exp $
 * ============================================================================
 *  This module contains routines for reading/writing files, including ROM
 *  images, CFG files, etc.
 * ============================================================================
 *  
 * ============================================================================
 */
#ifndef _FILE_H_
#define _FILE_H_

/*
 * ============================================================================
 *  FILE_READ_ROM16      -- Reads a 16-bit ROM image up to 64K x 16.
 *  
 *                          Leaves file pointer pointing at end of ROM image
 *                          if read is successful.  Returns # of words read
 *                          on success, -1 on failure.  The length returned
 *                          may be smaller than the length requested if the
 *                          file is not the expected length.
 * ============================================================================
 */
int         file_read_rom16     (FILE *f, int len, uint_16 img[]);
    
/*
 * ============================================================================
 *  FILE_READ_ROM8P2     -- Reads a 10-bit ROM image up to 64K x 16 in packed
 *                          8 plus 2 format.  The first 'len' bytes are 
 *                          the 8 LSB's of the ROM's decles.  The next
 *                          'len / 4' bytes hold the 2 MSBs, packed in little-
 *                          endian order.  This format is used by the VOL1,
 *                          VOL2 resource files, and is included for 
 *                          completeness.
 *  
 *                          Leaves file pointer pointing at end of ROM image
 *                          if read is successful.  Returns # of words read
 *                          on success, -1 on failure.  Short reads are not
 *                          supported for this format.
 * ============================================================================
 */
int         file_read_rom8p2    (FILE *f, int len, uint_16 img[]);


/*
 * ============================================================================
 *  FILE_READ_ROM8       -- Reads an 8-bit ROM image up to 64K x 16.
 *  
 *                          Leaves file pointer pointing at end of ROM image
 *                          if read is successful.  Returns words read
 *                          on success, -1 on failure.  The length returned
 *                          may be smaller than the length requested if the
 *                          file is not the expected length.
 * ============================================================================
 */
int         file_read_rom8      (FILE *f, int len, uint_16 img[]);


/*
 * ============================================================================
 *  FILE_EXISTS     -- Determines if a given file exists.
 * ============================================================================
 */
int file_exists(const char *pathname);

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

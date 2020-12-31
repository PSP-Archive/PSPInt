/*
 * ============================================================================
 *  Title:    File I/O Routines
 *  Author:   J. Zbiciak
 *  $Id: file.c,v 1.5 2001/11/02 02:00:03 im14u2c Exp $
 * ============================================================================
 *  This module contains routines for reading/writing files, including ROM
 *  images, CFG files, etc.
 *
 *  Currently, these routines operate on FILE*'s rather than on filenames,
 *  since I'd like these to be able to work in structured files someday.
 *  (eg. so I can read a ROM image out of an archive, or such.)
 * ============================================================================
 *  FILE_READ_ROM16      -- Reads a 16-bit big-endian ROM image.
 *  FILE_READ_ROM8P2     -- Reads a 10-bit ROM image in 8 plus 2 format
 *  FILE_READ_ROM10      -- Reads an 8-bit ROM image (eg. GROM).
 *  FILE_PARSE_CFG       -- Parses a CFG file and returns a linked list of 
 *                          configuration actions to be handled by the
 *                          machine configuration engine.
 * ============================================================================
 */

static const char rcs_id[]="$Id: file.c,v 1.5 2001/11/02 02:00:03 im14u2c Exp $";

#include <stdio.h>
#include <stdlib.h>

#include "config.h"
#include "file.h"



/*
 * ============================================================================
 *  FILE_READ_ROM16      -- Reads a 16-bit ROM image up to 64K x 16.
 *  
 *                          Leaves file pointer pointing at end of ROM image
 *                          if read is successful.  Returns 0 on success, -1
 *                          of failure.
 * ============================================================================
 */
int         file_read_rom16     (FILE *f, int len, uint_16 img[])
{
    int r;

    /* -------------------------------------------------------------------- */
    /*  Sanity check:  To all the arguments make sense?                     */
    /* -------------------------------------------------------------------- */
    if (!f || !img || len < 0 || len > 65536)
    {
        fprintf(stderr, "file_read_rom16:  Bad parameters!\n"
                        "                  %p, %10d, %p\n", f, len, img);
        exit(1);
    }

    /* -------------------------------------------------------------------- */
    /*  Read in the ROM image.                                              */
    /* -------------------------------------------------------------------- */
    len = fread((void*) img, 2, len, f);

#ifdef _LITTLE_ENDIAN
    /* -------------------------------------------------------------------- */
    /*  Byte-swap the ROM image.                                            */
    /* -------------------------------------------------------------------- */
    for (r = 0; r < len; r++)
    {
        img[r] = (img[r] >> 8) | (img[r] << 8);         /* Rotate 8 bits.   */
    }
#endif

    return len;
}
    
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
 *                          if read is successful.  Returns 0 on success, -1
 *                          of failure.
 * ============================================================================
 */
int         file_read_rom8p2    (FILE *f, int len, uint_16 img[])
{
    int r, blk8sz, blk2sz, blk8, blk2, shl;
    uint_8 *tmp;

    /* -------------------------------------------------------------------- */
    /*  Sanity check:  To all the arguments make sense?                     */
    /* -------------------------------------------------------------------- */
    if (!f || !img || len < 0 || len > 65536)
    {
        fprintf(stderr, "file_read_rom8p2:  Bad parameters!\n"
                        "                   %p, %10d, %p\n", f, len, img);
        exit(1);
    }

    /* -------------------------------------------------------------------- */
    /*  Calculate the sizes of the 8-bit and 2-bit sections, being careful  */
    /*  to round the decle count up to handle non-multiple-of-4 images.     */
    /* -------------------------------------------------------------------- */
    blk8sz = len;
    blk2sz = (len + 3) >> 2;

    /* -------------------------------------------------------------------- */
    /*  Read in the ROM image to a temporary storage buffer for unpacking.  */
    /* -------------------------------------------------------------------- */
    tmp = calloc(blk8sz + blk2sz, 1);

    if (!tmp)
    {
        fprintf(stderr, "file_read_rom8p2:  Out of memory.\n");
        exit(1);
    }

    r = fread(tmp, 1, blk8sz + blk2sz, f);

    if (r != blk8sz + blk2sz)
    {
        fprintf(stderr, "file_read_rom8p2:  Error reading ROM image.\n");
        perror("fread()");

        free(tmp);
        return -1;
    }

    /* -------------------------------------------------------------------- */
    /*  Unpack the ROM image into the user's buffer.                        */
    /* -------------------------------------------------------------------- */
    for (blk8 = 0, blk2 = blk8sz; blk8 < blk8sz; blk8++)
    {
        shl = 8 - ((blk8 & 3) << 1);

        img[blk8] = tmp[blk8] | (0x0300 & (tmp[blk2] << shl));

        if ((blk8 & 3) == 3) blk2++;
    }

    free(tmp);

    return len;
}


/*
 * ============================================================================
 *  FILE_READ_ROM8       -- Reads an 8-bit ROM image up to 64K x 16.
 *  
 *                          Leaves file pointer pointing at end of ROM image
 *                          if read is successful.  Returns 0 on success, -1
 *                          of failure.
 * ============================================================================
 */
int         file_read_rom8      (FILE *f, int len, uint_16 img[])
{
    int r; 
    uint_16 packed, byte0, byte1;

    /* -------------------------------------------------------------------- */
    /*  Sanity check:  To all the arguments make sense?                     */
    /* -------------------------------------------------------------------- */
    if (!f || !img || len < 0 || len > 65536)
    {
        fprintf(stderr, "file_read_rom8:  Bad parameters!\n"
                        "                 %p, %10d, %p\n", f, len, img);
        exit(1);
    }

    /* -------------------------------------------------------------------- */
    /*  Read in the ROM image.                                              */
    /* -------------------------------------------------------------------- */
    len = fread((void*) img, 1, len, f);

    /* -------------------------------------------------------------------- */
    /*  Unpack the ROM image.                                               */
    /* -------------------------------------------------------------------- */
    for (r = len; r >= 0; r -= 2)
    {
        packed = img[r >> 1];

#ifdef _LITTLE_ENDIAN
        byte1 = packed >> 8;
        byte0 = packed & 0xFF;
#else
        byte0 = packed >> 8;
        byte1 = packed & 0xFF;
#endif
        img[r + 0] = byte0;
        img[r + 1] = byte1;
    }

    return len;
}

/*
 * ============================================================================
 *  FILE_EXISTS     -- Determines if a given file exists.
 * ============================================================================
 */
int
file_exists
(
    const char *pathname
)
{
    /* --------------------------------------------------------------------- */
    /*  NOTE: access() may not be portable, but works for now.               */
    /* --------------------------------------------------------------------- */
    return access(pathname, R_OK|F_OK) != -1;
}


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

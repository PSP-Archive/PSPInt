/* ======================================================================== */
/*  INTELLICART ROM File I/O wrapper                    J. Zbiciak, 2003    */
/*                                                                          */
/*  This is a pretty simple "readfile"/"writefile" wrapper that handles     */
/*  reading and writing .ROM and .BIN+CFG files.                            */
/*                                                                          */
/*  The readfile wrapper implements the following rules:                    */
/*                                                                          */
/*   -- If the extension '.rom' is given, try to read the file as a .ROM    */
/*      format file.                                                        */
/*                                                                          */
/*   -- If the extension '.bin' is given, try to read the file as a .BIN    */
/*      + .CFG file pair.                                                   */
/*                                                                          */
/*   -- If neither of the above succeeds, try appending '.rom' and read     */
/*      the file as a .ROM file.                                            */
/*                                                                          */
/*   -- If none of those succeed, try appending '.bin' and read the file    */
/*      as a .BIN+CFG file.                                                 */
/*                                                                          */
/*  The writefile wrapper implements the following rules:                   */
/*                                                                          */
/*   -- If the extension '.rom' is given, write a .ROM file.                */
/*   -- If the extension '.bin' is given, write a .BIN and .CFG file.       */
/*   -- If no extension is given, write all three variants.                 */
/*                                                                          */
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
/*                 Copyright (c) 2003-+Inf, Joseph Zbiciak                  */
/* ======================================================================== */

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "config.h"
#include "icart/icartrom.h"
#include "icart/icartbin.h"


/* ======================================================================== */
/*  ICART_WRITEFILE -- Write combination of BIN+CFG or ROM from an icart.   */
/* ======================================================================== */
void icart_writefile(char *fname, icartrom_t *icart)
{
    char *bin_fn = NULL, *cfg_fn = NULL, *rom_fn = NULL;
    int name_len = strlen(fname);
    char *s = fname + name_len;
    char *ext = strrchr(fname, '.');  /* look for an extension */

    /* -------------------------------------------------------------------- */
    /*  Is it .ROM?                                                         */
    /* -------------------------------------------------------------------- */
    if (ext && stricmp(ext, ".rom")==0)
    {
        rom_fn = fname;
    } else 
    /* -------------------------------------------------------------------- */
    /*  Is it .BIN?                                                         */
    /* -------------------------------------------------------------------- */
    if (ext && stricmp(ext, ".bin")==0)
    {
        bin_fn = fname;
        cfg_fn = strdup(fname);
        if (!cfg_fn) 
        { 
            fprintf(stderr, "icart_writefile: Out of memory\n");
            exit(1);
        }

        s = cfg_fn + name_len - 4;
        strcpy(s, ".cfg");
    } else
    /* -------------------------------------------------------------------- */
    /*  Neither?  Write all three.                                          */
    /* -------------------------------------------------------------------- */
    {   
        /* no extension or unknown extension */
        bin_fn = malloc(3*(name_len + 5));
        if (!bin_fn)
        { 
            fprintf(stderr, "icart_writefile: Out of memory\n");
            exit(1);
        }
        cfg_fn = bin_fn + name_len + 5;
        rom_fn = cfg_fn + name_len + 5;

        sprintf(bin_fn, "%s.bin", fname);    
        sprintf(cfg_fn, "%s.cfg", fname);    
        sprintf(rom_fn, "%s.rom", fname);    
    }

    /* -------------------------------------------------------------------- */
    /*  If we found a .ROM name, write a .ROM file.                         */
    /* -------------------------------------------------------------------- */
    if (rom_fn)
    {
        icartrom_writefile(rom_fn, icart);
    }

    /* -------------------------------------------------------------------- */
    /*  If we found a .BIN name, write a .BIN and CFG file.                 */
    /* -------------------------------------------------------------------- */
    if (bin_fn)
    {
        icb_writefile(bin_fn, cfg_fn, icart);
    }
}


/* ======================================================================== */
/*  ICART_READFILE -- Make a best effort, trying to read a ROM/BIN+CFG.     */
/* ======================================================================== */
void icart_readfile(char *fname, icartrom_t *icart)
{
    char *rom1_fn = NULL, *bin1_fn = NULL, *cfg1_fn = NULL;
    char *rom2_fn = NULL, *bin2_fn = NULL, *cfg2_fn = NULL;
    int name_len = strlen(fname);
    char *s = fname + name_len;
    char *ext = strrchr(fname, '.');  /* look for an extension */
    FILE *f;

    /* -------------------------------------------------------------------- */
    /*  Is it .ROM?                                                         */
    /* -------------------------------------------------------------------- */
    if (ext && stricmp(ext, ".rom")==0)
    {
        rom1_fn = fname;
    }

    /* -------------------------------------------------------------------- */
    /*  Is it .BIN?                                                         */
    /* -------------------------------------------------------------------- */
    if (ext && stricmp(ext, ".bin")==0)
    {
        bin1_fn = fname;
        cfg1_fn = strdup(fname);
        if (!cfg1_fn) 
        { 
            fprintf(stderr, "icart_readfile: Out of memory\n");
            exit(1);
        }

        s = cfg1_fn + name_len - 4;
        strcpy(s, ".cfg");
    }

    /* -------------------------------------------------------------------- */
    /*  In case those fail, have a backup plan.                             */
    /* -------------------------------------------------------------------- */
    {   
        /* no extension or unknown extension */
        bin2_fn = malloc(3*(name_len + 5));
        if (!bin2_fn)
        { 
            fprintf(stderr, "icart_readfile: Out of memory\n");
            exit(1);
        }
        cfg2_fn = bin2_fn + name_len + 5;
        rom2_fn = cfg2_fn + name_len + 5;

        sprintf(bin2_fn, "%s.bin", fname);    
        sprintf(cfg2_fn, "%s.cfg", fname);    
        sprintf(rom2_fn, "%s.rom", fname);    
    }

    /* -------------------------------------------------------------------- */
    /*  Now try out all of our options.                                     */
    /* -------------------------------------------------------------------- */
    if (rom1_fn && (f = fopen(rom1_fn, "rb")))
    {
        fclose(f);
        icartrom_readfile(rom1_fn, icart);
        return;
    }

    if (bin1_fn && (f = fopen(bin1_fn, "rb")))
    {
        fclose(f);
        icb_read_bincfg(bin1_fn, cfg1_fn, icart);
        return;
    }

    if (rom2_fn && (f = fopen(rom2_fn, "rb")))
    {
        fclose(f);
        icartrom_readfile(rom2_fn, icart);
        return;
    }

    if (bin2_fn && (f = fopen(bin2_fn, "rb")))
    {
        fclose(f);
        icb_read_bincfg(bin2_fn, cfg2_fn, icart);
        return;
    }

    fprintf(stderr, "ERROR: Could not read a .ROM or .BIN for '%s'\n", fname);
    exit(1);
}

/* ======================================================================== */
/*                 Copyright (c) 2003-+Inf, Joseph Zbiciak                  */
/* ======================================================================== */

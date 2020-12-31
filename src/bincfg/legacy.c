/* ======================================================================== */
/*  Title:    Legacy INTVPC BIN+CFG support                                 */
/*  Author:   J. Zbiciak, J. Tanner                                         */
/* ------------------------------------------------------------------------ */
/*  This module implements a memory peripheral with the semantics of        */
/*  INTVPC's BIN+CFG file, at least for the most part.  ECS paged ROM is    */
/*  supported indirectly by instantiating Paged ROMs from mem/mem.c.        */
/*                                                                          */
/*  The routines for reading BIN+CFG files are in bincfg/bincfg.h, not      */
/*  this file.                                                              */
/* ======================================================================== */


#include "config.h"
#include "periph/periph.h"
#include "mem/mem.h"
#include "file/file.h"
#include "bincfg.h"
#include "legacy.h"

/* ======================================================================== */
/*  LEGACY_READ -- read from a legacy BIN+CFG.                              */
/* ======================================================================== */
uint_32 legacy_read (periph_t *per, periph_t *ign, uint_32 addr, uint_32 data)
{
    legacy_t *l = (legacy_t*)per;
    uint_32 mask;

    (void)ign;
    (void)data;

    if ((l->loc[addr].flags & BC_SPAN_R) != 0)
    {
        mask = ~(~0 << l->loc[addr].width);
        return l->loc[addr].data & mask;
    }

    return ~0U;
}

/* ======================================================================== */
/*  LEGACY_WRITE -- write to a legacy BIN+CFG.                              */
/* ======================================================================== */
void  legacy_write(periph_t *per, periph_t *ign, uint_32 addr, uint_32 data)
{
    legacy_t *l = (legacy_t*)per;
    uint_32 mask;

    (void)ign;
    (void)data;

    if ((l->loc[addr].flags & BC_SPAN_W) != 0)
    {
        mask = ~(~0 << l->loc[addr].width);
        l->loc[addr].data = data & mask;
    }
}

/* ======================================================================== */
/*  LEGACY_POKE  -- write to a legacy BIN+CFG, ignoring read-only status.   */
/* ======================================================================== */
void legacy_poke (periph_t *per, periph_t *ign, uint_32 addr, uint_32 data)
{
    legacy_t *l = (legacy_t*)per;
    uint_32 mask;

    (void)ign;
    (void)data;

    mask = ~(~0 << l->loc[addr].width);
    l->loc[addr].data = data & mask;
}


/* ======================================================================== */
/*  LEGACY_APPLY_BINCFG -- Populates a legacy_t from a BIN+CFG.             */
/* ======================================================================== */
LOCAL int legacy_apply_bincfg(bc_cfgfile_t *bc, legacy_t *l)
{
    int addr, ofs;
    bc_memspan_t *span;
    int num_ecs = 0;

//LUDO: if (l->bc == NULL) 
{
  l->bc  = bc;
}

if (l->loc == NULL) {
    l->loc = calloc(65536, sizeof(struct legacy_loc_t));
}
    if (!l->loc) return -1;

    /* -------------------------------------------------------------------- */
    /*  Traverse the memspan list, registering memory segments in legacy_t  */
    /* -------------------------------------------------------------------- */
    for (span = bc->span; span; span = (bc_memspan_t *)span->l.next)
    {
        int slen;

        slen = span->e_addr - span->s_addr + 1;

        /* ---------------------------------------------------------------- */
        /*  If this span has an ECS page, skip it for now.  We'll come      */
        /*  back to it in a minute.                                         */
        /* ---------------------------------------------------------------- */
        if ( span->epage != BC_SPAN_NOPAGE ||
            (span->flags & BC_SPAN_EP) != 0)
        {
            num_ecs++;
            continue;
        }

        /* ---------------------------------------------------------------- */
        /*  Assertion:  If PRELOAD, then span->data.                        */
        /* ---------------------------------------------------------------- */
        assert(((span->flags & BC_SPAN_PL) == 0) || (span->data != 0));

        /* ---------------------------------------------------------------- */
        /*  Add the segment to the legacy_t.                                */
        /* ---------------------------------------------------------------- */
        for (addr = span->s_addr, ofs = 0; addr <= span->e_addr; addr++, ofs++)
        {
            l->loc[addr].flags |= span->flags;

            if (span->data)
                l->loc[addr].data  = span->data[ofs];
            if ((span->flags & BC_SPAN_PK) == 0)
                l->loc[addr].width = span->width;
        }
    }

    /* -------------------------------------------------------------------- */
    /*  Now allocate ECS Paged ROMs as needed.                              */
    /* -------------------------------------------------------------------- */
    l->npg_rom = num_ecs;
    if (num_ecs > 0)
    {
        uint_32 slen, i;

        l->pg_rom = calloc(sizeof(mem_t), num_ecs);
        num_ecs = 0;

        for (span = bc->span; span; span = (bc_memspan_t *)span->l.next)
        {
            /* ------------------------------------------------------------ */
            /*  Skip spans that are not ECS pages.                          */
            /* ------------------------------------------------------------ */
            if ( span->epage == BC_SPAN_NOPAGE &&
                (span->flags & BC_SPAN_EP) == 0)
            {
                continue;
            }

            /* ------------------------------------------------------------ */
            /*  Assertion:  If ECS Paged, then span->data && bank >= 0      */
            /* ------------------------------------------------------------ */
            assert(span->data && span->epage != BC_SPAN_NOPAGE);

            /* ------------------------------------------------------------ */
            /*  Get the page size and round up to next power of 2.          */
            /* ------------------------------------------------------------ */
            slen = span->e_addr - span->s_addr; /* size - 1 */
            for (i = 0; slen >> i == 0; i++)
                slen |= slen >> i;
            slen++;

            if (slen != (uint_32)span->e_addr - span->s_addr + 1)
            {
                span->data = realloc(span->data, slen);
                span->e_addr = span->s_addr + slen - 1;
                if (!span->data)
                    return -1;
            }

            /* ------------------------------------------------------------ */
            /*  Make a paged ROM out of this.                               */
            /* ------------------------------------------------------------ */
            if (mem_make_prom(l->pg_rom + num_ecs, span->width, span->s_addr, 
                             slen, span->epage, span->data))
            {
                free(l->loc);
                free(l->pg_rom);
                return -1;
            }

            /* ------------------------------------------------------------ */
            /*  Since we claimed span->data, null it out.                   */
            /* ------------------------------------------------------------ */
            span->data = NULL;

            num_ecs++;
        }
    }

    return 0;
}

/* ======================================================================== */
/*  LEGACY_READ_BINCFG -- Reads a .BIN and optional .CFG file.              */
/* ======================================================================== */
LOCAL int legacy_read_bincfg(char *bin_fn, char *cfg_fn, legacy_t *l)
{
    FILE *fc  = NULL;
    bc_cfgfile_t *bc = NULL;

    /* -------------------------------------------------------------------- */
    /*  Read the .CFG file.  This process  open it, then parse it.          */
    /*  Otherwise, we skip it -- lack of .CFG file is non-fatal.            */
    /* -------------------------------------------------------------------- */
    if (cfg_fn && (fc = fopen(cfg_fn, "r")) != NULL)
    {
        bc = bc_parse_cfg(fc, bin_fn, cfg_fn);
        fclose(fc);
    } else
    {
        bc = bc_parse_cfg(NULL, bin_fn, NULL);
    }
    if (!bc)
        return -1;

#ifndef BC_NODOMACRO
    /* -------------------------------------------------------------------- */
    /*  Apply any statically safe macros.  Ignore errors.                   */
    /* -------------------------------------------------------------------- */
    bc_do_macros(bc, 0);
#endif

    /* -------------------------------------------------------------------- */
    /*  Populate the config with corresponding BIN data.                    */
    /* -------------------------------------------------------------------- */
    if (bc_read_data(bc))
    {
        fprintf(stderr, "Error reading data for CFG file.\n");
        goto err;
    }

    /* -------------------------------------------------------------------- */
    /*  Apply the configuration.  This generates the icartrom.              */
    /* -------------------------------------------------------------------- */
    if (legacy_apply_bincfg(bc, l))
    {
        fprintf(stderr, "Error applying CFG file\n");
        goto err;
    }

    return 0;

err:
#ifndef BC_NOFREE
    /* -------------------------------------------------------------------- */
    /*  Discard the parsed config.                                          */
    /* -------------------------------------------------------------------- */
    bc_free_cfg(bc);
    l->bc = NULL;
#endif
    return -1;

}

/* ======================================================================== */
/*  LEGACY_BINCFG -- Try to determine if a file is BIN+CFG or ROM, and      */
/*                   read it in if it is BIN+CFG.                           */
/*                                                                          */
/*  The return value from this function requires explanation.  If we        */
/*  figure out a .ROM file associated with this fname, we will a distinct   */
/*  char * that points to its filename.  If we determine the file is a      */
/*  BIN+CFG file pair, we will try to load it.  On success, we will return  */
/*  fname directly.  Otherwise, we will return NULL.                        */
/* ======================================================================== */
char *legacy_bincfg
(
    legacy_t        *l,         /*  Legacy BIN+CFG structure        */
    char            *fname      /*  Basename to use for CFG/BIN     */
)
{
    char *rom1_fn = NULL, *bin1_fn = NULL, *cfg1_fn = NULL;
    char *rom2_fn = NULL, *bin2_fn = NULL, *cfg2_fn = NULL, *int2_fn = NULL;
    char *itv2_fn = NULL;
    int name_len = strlen(fname);
    char *s = fname + name_len;
    char *ext = strrchr(fname, '.');  /* look for an extension */

    /* -------------------------------------------------------------------- */
    /*  Silly case:  If the filename ends in a '.', strip it off.           */
    /* -------------------------------------------------------------------- */
    if (ext && ext[1] == '\0')
    {
        *ext = '\0';
        ext = NULL;
    }
 
    /* -------------------------------------------------------------------- */
    /*  Is it .ROM?                                                         */
    /* -------------------------------------------------------------------- */
    if (ext && stricmp(ext, ".rom")==0)
    {
        rom1_fn = fname;
    }
    /* -------------------------------------------------------------------- */
    /*  Is it .BIN or .INT or .ITV?                                         */
    /* -------------------------------------------------------------------- */
    else if (ext && (stricmp(ext, ".bin")==0 || 
                     stricmp(ext, ".int")==0 || 
                     stricmp(ext, ".itv")==0) )
    {
        bin1_fn = fname;
        cfg1_fn = strdup(fname);
        if (!cfg1_fn) 
        { 
            fprintf(stderr, "legacy_bincfg: Out of memory\n");
            exit(1);
        }

        s = cfg1_fn + name_len - 4;
        strcpy(s, ".cfg");
    }
    /* -------------------------------------------------------------------- */
    /*  In case those fail, have a backup plan.                             */
    /*                                                                      */
    /*  For instance, on the Mac version of "Intellivision Lives!", the     */
    /*  ROM images are equivalent to a .BIN, but they have no extension.    */
    /*  They do, occasionally, have a corresponding .CFG file, with ".cfg"  */
    /*  as the extension.  (Pointed out by JJT.)                            */
    /* -------------------------------------------------------------------- */
    else 
    {
        /* ---------------------------------------------------------------- */
        /*  Handle the no-extension-but-matching-cfg case.                  */
        /* ---------------------------------------------------------------- */
        if (!ext)
        {
            bin1_fn = fname;
            cfg1_fn = malloc(name_len+5);
            sprintf( cfg1_fn, "%s.cfg", fname );
        }
        
        /* ---------------------------------------------------------------- */
        /*  Also search for all known extensions appended to the file name. */
        /*  This lets users type "jzintv foo" and it'll find "foo.rom" or   */
        /*  "foo.bin" or whatever.                                          */
        /* ---------------------------------------------------------------- */
        bin2_fn = malloc(5*(name_len + 5));
        if (!bin2_fn)
        { 
            fprintf(stderr, "legacy_bincfg: Out of memory\n");
            exit(1);
        }
        cfg2_fn = bin2_fn + name_len + 5;
        rom2_fn = cfg2_fn + name_len + 5;
        int2_fn = rom2_fn + name_len + 5;
        itv2_fn = int2_fn + name_len + 5;
        
        sprintf(bin2_fn, "%s.bin", fname);    
        sprintf(cfg2_fn, "%s.cfg", fname);    
        sprintf(rom2_fn, "%s.rom", fname);    
        sprintf(int2_fn, "%s.int", fname);    
        sprintf(itv2_fn, "%s.itv", fname);
    }

    /* -------------------------------------------------------------------- */
    /*  Now try out all of our options.                                     */
    /* -------------------------------------------------------------------- */
    if (rom1_fn && file_exists(rom1_fn))
    {
        return strdup(rom1_fn);
    }

    if (bin1_fn && file_exists(bin1_fn))
    {
        if (legacy_read_bincfg(bin1_fn, cfg1_fn, l))
            return NULL;

        goto finish;
    }

    if (rom2_fn && file_exists(rom2_fn))
    {
        return strdup(rom2_fn);
    }

    if (bin2_fn && file_exists(bin2_fn))
    {
        if (legacy_read_bincfg(bin2_fn, cfg2_fn, l))
            return NULL;

        goto finish;
    }

    if (int2_fn && file_exists(int2_fn))
    {
        if (legacy_read_bincfg(int2_fn, cfg2_fn, l))
            return NULL;

        goto finish;
    }
    
    if (itv2_fn && file_exists(itv2_fn))
    {
        fprintf( stderr, "Trying %s, %s\n", itv2_fn, cfg2_fn ); //Debug
        if (legacy_read_bincfg(itv2_fn, cfg2_fn, l))
            return NULL;

        goto finish;
    }

    return NULL;

finish:
    /* -------------------------------------------------------------------- */
    /*  Set up peripheral function pointers to support reads of the right   */
    /*  width.  Ignore writes and explicitly disallow ticks.                */
    /* -------------------------------------------------------------------- */
    l->periph.read    = legacy_read;
    l->periph.write   = legacy_write;
    l->periph.peek    = legacy_read;
    l->periph.poke    = legacy_poke;

    l->periph.tick        = NULL;
    l->periph.min_tick    = ~0U;
    l->periph.max_tick    = ~0U;
    l->periph.addr_base   = 0;
    l->periph.addr_mask   = 0xFFFF;

    return fname;
}

/* ======================================================================== */
/*  LEGACY_REGISTER -- Actually registers the legacy ROMs.  Also frees      */
/*                     the saved bc_cfgfile_t.                              */
/* ======================================================================== */
int legacy_register
(
    legacy_t *l,
    periph_bus_p bus
    /* note: should pass in cp1600 struct here, since prom will need it */
)
{
    bc_cfgfile_t *bc = l->bc;
    bc_memspan_t *span;
    int num_ecs = 0;
    int addr, need_register;

    /* -------------------------------------------------------------------- */
    /*  Register the legacy ROM.  Eventually, we might 'optimize' this as   */
    /*  I did for the Intellicart, where I have multiple periph_t's based   */
    /*  on what flags are set for each range.                               */
    /* -------------------------------------------------------------------- */
    for (span = bc->span; span; span = (bc_memspan_t *)span->l.next)
    {
        int slen;

        slen = span->e_addr - span->s_addr + 1;

        /* ---------------------------------------------------------------- */
        /*  Handle ECS-style pages differently.                             */
        /* ---------------------------------------------------------------- */
        if ( span->epage != BC_SPAN_NOPAGE ||
            (span->flags & BC_SPAN_EP) != 0)
        {
            char name[17];

            sprintf(name, "Paged ROM %d", num_ecs);
            periph_register(bus, &(l->pg_rom[num_ecs].periph), 
                            span->s_addr, span->e_addr, name);
            num_ecs++;
            continue;
        }

        /* ---------------------------------------------------------------- */
        /*  Handle other segments directly.  If all the addresses in a      */
        /*  span are already mapped, skip this span.  We could/should       */
        /*  optimize this at some point to only register areas not yet      */
        /*  mapped, but I'm lazy right now.  The optimization would rarely  */
        /*  if ever kick in.                                                */
        /* ---------------------------------------------------------------- */
        need_register = 0;
        for (addr = span->s_addr; addr <= span->e_addr; addr++)
        {
            if (!(l->loc[addr].flags & LOC_MAPPED))
                need_register = 1;

            l->loc[addr].flags |= LOC_MAPPED;
        }

        if (need_register)
            periph_register(bus, &l->periph, span->s_addr, span->e_addr, 
                            span->flags & BC_SPAN_PK ? "[macro poke]"
                                                     : "Legacy BIN+CFG");
    }

#ifndef BC_NOFREE
    /* -------------------------------------------------------------------- */
    /*  Discard the parsed config.                                          */
    /* -------------------------------------------------------------------- */
    bc_free_cfg(l->bc);
#endif

    assert(num_ecs == l->npg_rom);

    return 0;
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
/* ------------------------------------------------------------------------ */
/*           Copyright (c) 2003-+Inf, Joseph Zbiciak, John Tanner           */
/* ======================================================================== */

/*
 * ============================================================================
 *  Title:    STIC -- Standard Television Interface Chip, AY-3-8900
 *  Author:   J. Zbiciak
 *  $Id$
 * ============================================================================
 *  This module implements the STIC Chip.  Its functionality is currently
 *  far from complete.  For instance, I don't implement MOBs yet.
 * ============================================================================
 *  The STIC processor is active over several regions of memory:
 *
 *   0x0000 - 0x001F     -- MOB (Movable OBject) controls 
 *                          0x0000 + n  X Position, MOB #n
 *                          0x0008 + n  Y Position, MOB #n
 *                          0x0010 + n  Attribute Register, MOB #n
 *                          0x0018 + n  Collision Detect, MOB #n
 *
 *   0x0020 - 0x0021     -- STIC control registers  
 *                          0x0020      Display Enable
 *                          0x0021      Graphics Mode
 *
 *   0x0028 - 0x002C     -- Global color settings
 *                          0x0028 + n  Color Stack (n == 0..3)
 *                          0x002C      Border Color
 *
 *   0x0030 - 0x0032     -- Display framing
 *                          0x0030      Pixel Column Delay (0..7)
 *                          0x0031      Pixel Row Delay (0..7)
 *                          0x0032      Edge inhibit (bit0->left, bit1->top)
 *
 *   0x0200 - 0x035F     -- Background Card Table (BACKTAB)
 *
 *   0x3800 - 0x39FF     -- Graphics RAM
 *  
 *                          Note:  BACKTAB and GRAM are actually physically
 *                          separate.  This STIC implementation 'snoops'
 *                          these accesses for performance reasons.
 *
 *  For now, I handle all of the control register accesses into one 
 *  peripheral entry and all bus-snooping accesses in a second peripheral
 *  entry.
 * ============================================================================
 */

#ifndef _STIC_H
#define _STIC_H 1

#include "cp1600/req_bus.h"

/*
 * ============================================================================
 *  STIC_T           -- Main STIC structure.
 *
 *  Note: These bitfields seem to be broken at the present.
 *
 *  STIC_MOB_X_T     -- MOB X Position Bitfield Structure
 *  STIC_MOB_Y_T     -- MOB Y Position Bitfield Structure
 *  STIC_MOB_A_T     -- MOB Attribute Bitfield Structure
 *  STIC_MOB_C_T     -- MOB Collision Bitfield Structure
 * ============================================================================
 */

struct stic_t
{
    /* -------------------------------------------------------------------- */
    /*  The STIC is split into three separate peripherals, since a RD/WR    */
    /*  to something the STIC is interested in needs to go one of three     */
    /*  places -- either (1) to update the general state of the STIC via a  */
    /*  control register, (2) to update the BACKTAB, or (3) to update a     */
    /*  card definition in GRAM.                                            */
    /* -------------------------------------------------------------------- */
    periph_t    stic_cr;
    periph_t    snoop_btab;
    periph_t    snoop_gram;

    /* -------------------------------------------------------------------- */
    /*  We just keep a raw image of the STIC registers and decode manually  */
    /* -------------------------------------------------------------------- */
    uint_32     raw [0x40];
    uint_8      gmem[0x140 * 8];

    /* -------------------------------------------------------------------- */
    /*  We store several bitmaps and display lists.                         */
    /* -------------------------------------------------------------------- */
    int         fifo_ptr;                   /* Video FIFO pointer.          */
    uint_16     btab_sr [240];              /* BACKTAB as it is in Sys. RAM */
    uint_16     btab    [240];              /* BACKTAB as STIC sees it      */
    uint_32     last_bg [12];               /* Last background color by row */
    uint_8      bt_bmp  [240*8];            /* BACKTAB 1-bpp display list.  */
    uint_32     mob_img [ 16*16  / 8];      /* Expanded/mirrored MOB 4bpp.  */
    uint_16     mob_bmp [8][16];            /* Expanded/mirrored MOBs 1bpp. */
    uint_32     mpl_img [192*224 / 8];      /* MOB image.                   */
    uint_32     mpl_vsb [192*224 /32];      /* MOB visibility bitmap.       */
    uint_32     mpl_pri [192*224 /32];      /* MOB priority bitmap.         */
    uint_32     xbt_img [192*112 / 8];      /* Re-tiled BACKTAB image.      */
    uint_32     xbt_bmp [192*112 /32];      /* Re-tiled BACKTAB 1-bpp.      */
    uint_32     image   [192*224 / 8];      /* Final 192x224 image, 4-bpp.  */

    uint_8      *disp;
    gfx_t       *gfx;

#if 0
    /* -------------------------------------------------------------------- */
    /*  Private GROM, GRAM, BACKTAB memory pointers for efficiency.         */
    /* -------------------------------------------------------------------- */
    uint_16     *grom;
    uint_16     *gram;
    uint_16     *backtab;
#endif

    /* -------------------------------------------------------------------- */
    /*  IRQ and BUSRQ generation.                                           */
    /* -------------------------------------------------------------------- */
    req_bus_t   *req_bus;       /* Bus for INTRQ, BUSRQ, INTAK, BUSAK.      */

    /* -------------------------------------------------------------------- */
    /*  STIC internal flags.                                                */
    /* -------------------------------------------------------------------- */
    void        (*upd)(struct stic_t*); /* Update fxn for curr disp mode.   */
    int         phase;          /* 0 == Start of VBlank, 1 == Start of Pic  */
    int         ve_post;        /* Where vid-enables are posted.            */
    int         vid_enable;     /* Must be set every vsync to enable vid.   */
    int         mode;           /* 1 == FG/BG mode, 0 == Color Stack.       */
    int         bt_dirty;       /* BACKTAB is dirty.                        */
    int         gr_dirty;       /* GRAM is dirty.                           */
    int         ob_dirty;       /* MOBs are dirty.                          */
    uint_64     gmem_accessible;/* CPU can access GRAM/GROM.                */
    uint_64     stic_accessible;/* CPU can access STIC registers            */
    uint_64     next_busrq;     /* Time of next STIC->CPU bus request.      */
    uint_64     next_irq;       /* Time of next STIC->CPU Interrupt request */
    uint_64     next_phase;     /* Don't switch phases until after this     */

    /* -------------------------------------------------------------------- */
    /*  Performance monitoring.  :-)                                        */
    /* -------------------------------------------------------------------- */
    struct stic_time_t
    {
        double  full_update;
        double  draw_btab;
        double  draw_mobs;
        double  fix_bord; 
        double  merge_planes;
        double  push_vid;
        double  mob_colldet;
        double  gfx_vid_enable;
        int     total_frames;
    } time;

    /* -------------------------------------------------------------------- */
    /*  Demo recording                                                      */
    /* -------------------------------------------------------------------- */
    demo_t      *demo;
};

#ifndef STIC_T_
#define STIC_T_ 1
typedef struct stic_t stic_t;
#endif

/*
 * ============================================================================
 *  NONE OF THIS TIMING CRAP IS USED.  I'M DOING THIS A MUCH SIMPLER WAY NOW.
 * ============================================================================
 *  The STIC allows timely GRAM servicing only during VBLANK.  Outside 
 *  VBLANK, the CPU is stalled with wait-states.  The following macros
 *  define the time period for VBLANK and DISPACTV.  
 *
 *  TODO:  Consider making these config variables instead.
 *
 *  STIC_COLBURST    -- Color burst frequency (4x the system clock rate)
 *  STIC_FRAMCLKS    -- Number of system clocks in one frame
 *
 *  These values are calculated from the following facts and assumptions.
 *
 *   -- A full NTSC frame has 525 lines divided into two fields,
 *
 *   -- As a result each field has 262.5 lines.
 *
 *   -- The STIC has a vertical resolution of 192 pixels in the graphical
 *      display area.
 *
 *   -- I think we can safely assume that the STIC treats non-graphics card
 *      areas as "blanking" time, even though it is actively outputting the
 *      background color.  (I may end up getting proven wrong here, but the
 *      spec sheet seems to indicate this.)
 *
 *   -- One display period in system clocks is the colorburst rate
 *      divided by 240 for 60Hz NTSC.
 *
 *   -- So, the DISPACTV time is 192/262.5 of the total display period
 *      and VBLANK is the remainder.
 *
 *  Note:  This doesn't really take into account the fact that the active
 *  display period shifts slightly when the vertical delay register is
 *  set (for vertical scrolling).  For the existing set of Intellivision
 *  games, I don't think it really matters.  The _really_ tricky programmers
 *  that relied on exact synchronization behavior between the display chip
 *  and the CPU didn't show up until the C64 demo competitions really took
 *  off, I think.
 * ============================================================================
 */

#define STIC_COLBURST   3579545 /* Exact */
#define STIC_FRAMCLKS   14934

#define STIC_INTRQ_HOLD         (2920)
#define STIC_BUSRQ_HOLD_FIRST   (68)    /* First BUSRQ is really short?     */
#define STIC_BUSRQ_HOLD_NORMAL  (108)   /* Typical BUSRQs during display    */
#define STIC_BUSRQ_HOLD_EXTRA   (38)    /* 14th BUSRQ is half-length        */
#define STIC_MUST_BUSAK_TIME    (68)    /* CPU must ACK a BUSRQ by here     */
#define STIC_BUSRQ_MARGIN       (STIC_BUSRQ_HOLD_NORMAL - STIC_MUST_BUSAK_TIME)

#define STIC_STIC_ACCESSIBLE    (2900)  /* STIC time window during vblank   */
#define STIC_GMEM_ACCESSIBLE    (3796)  /* GRAM/GROM time window during vbl */

#define STIC_INITIAL_OFFSET     (2782)  /* Starting point of first frame.   */
#define STIC_INITIAL_PHASE      (

/*
 * ============================================================================
 *  STIC_CTRL_RD     -- Read from a STIC control register.
 *  STIC_CTRL_WR     -- Write to a STIC control register.
 *  STIC_BTAB_WR     -- Monitor writes to BACKTAB
 *  STIC_GRAM_WR     -- Monitor writes to GRAM
 * ============================================================================
 */

uint_32 stic_ctrl_rd(periph_t *per, periph_t *ign, uint_32 addr, uint_32 data);
void    stic_ctrl_wr(periph_t *per, periph_t *ign, uint_32 addr, uint_32 data);
void    stic_btab_wr(periph_t *per, periph_t *ign, uint_32 addr, uint_32 data);
void    stic_gram_wr(periph_t *per, periph_t *ign, uint_32 addr, uint_32 data);

/*
 * ============================================================================
 *  STIC_INIT        -- Initialize the STIC
 *  STIC_TICK        -- Perform a STIC update
 * ============================================================================
 */

int stic_init
(
    stic_t          *stic,
    uint_16         *grom_img,
    req_bus_t       *irq,
    gfx_t           *gfx,
    demo_t          *demo
);

uint_32 stic_tick
(
    periph_p        per,
    uint_32         len
);

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

/*
 * ============================================================================
 *  Title:    
 *  Author:   J. Zbiciak
 *  $Id: gfx.h,v 1.14 2001/02/03 02:34:21 im14u2c Exp $
 * ============================================================================
 *  GFX_INIT         -- Initializes a gfx_t object and gfx subsystem
 *  GFX_TICK         -- Services a gfx_t tick
 *  GFX_VID_ENABLE   -- Alert gfx that video has been enabled or blanked
 * ============================================================================
 *  GFX_PVT_T        -- Private internal state to gfx_t structure.
 *  GFX_T            -- Generic graphics object.  The graphics object is a 
 *                      periph also so that screen updates, etc. can be 
 *                      scheduled via the global tick mechanism.
 * ============================================================================
 *  The graphics subsystem provides an abstraction layer between the 
 *  emulator and the graphics library being used.  Theoretically, this 
 *  should allow easy porting to other graphics libraries.
 * ============================================================================
 */

#ifndef _GFX_H
#define _GFX_H

/*
 * ============================================================================
 *  GFX_PVT_T        -- Private internal state to gfx_t structure.
 *  GFX_T            -- Generic graphics object.  The graphics object is a 
 *                      periph also so that screen updates, etc. can be 
 *                      scheduled via the global tick mechanism.
 * ============================================================================
 */

typedef struct gfx_pvt_t *gfx_pvt_p;

typedef struct gfx_t
{
    periph_t    periph;             /*  Yes, gfx_t is a peripheral.         */
    uint_8      *vid;               /*  Display bitmap (160x200x8bpp).      */
    uint_8      bbox[8][4];         /*  Bounding boxes for the 8 MOBs       */
    int         dirty;              /*  FLAG: Display needs update.         */
    uint_32     drop_frame;         /*  while > 0 drop frames.              */
    uint_32     dropped_frames;     /*  counts dropped frames.              */
    uint_32     tot_frames;         /*  total frames                        */
    uint_32     tot_dropped_frames; /*  total dropped frames                */

    v_uint_32   hidden;             /*  Visibility flag (set by event_t)    */
    v_uint_32   scrshot;            /*  Screen-shot/movie requested         */

    int         b_color, b_dirty;   /*  Border color and dirty flag.        */
    int         x_blank, y_blank;   /*  FLAG: Blank top row, left column.   */
    int         x_delay, y_delay;   /*  X/Y display delay.                  */
    int         debug_blank;        /*  FLAG: If set, dim instead of blank  */

    gfx_pvt_p   pvt;                /*  Private data.                       */
} gfx_t;

/*
 * ============================================================================
 *  GFX_INIT          -- Initializes a gfx_t object and gfx subsystem
 *  GFX_TICK          -- Services a gfx_t tick
 *  GFX_VID_ENABLE    -- Alert gfx that video has been enabled or blanked
 * ============================================================================
 */
int      gfx_init        (gfx_t   *gfx, int x, int y, int bpp, int fs);
uint_32  gfx_tick_640x480(periph_p gfx, uint_32 len);
uint_32  gfx_tick_320x200(periph_p gfx, uint_32 len);
uint_32 gfx_tick_320x200x16(periph_p gfx_periph, uint_32 len);
void     gfx_vid_enable  (gfx_t   *gfx, int enabled);

/*
 * ============================================================================
 *  GFX_SET_BORD     -- Set the border / offset parameters for the display
 * ============================================================================
 */
void gfx_set_bord
(
    gfx_t *gfx,         /*  Graphics object.                        */
    int bord
);

/*
 * ============================================================================
 *  GFX_SET_TITLE    -- Sets the window title
 * ============================================================================
 */
int gfx_set_title(gfx_t *gfx, const char *title);

#define GFX_SHOT  (1)
#define GFX_MVTOG (2)
#define GFX_MOVIE (4)
#define GFX_RESET (8)

/*
 * ============================================================================
 *  GFX_SCRSHOT      -- Write a 320x200 screen shot to a PPM file.
 * ============================================================================
 */
void gfx_scrshot(uint_8 *scr);

/*
 * ============================================================================
 *  GFX_MOVIEUPD     -- Update a movie-in-progress
 * ============================================================================
 */
void gfx_movieupd(gfx_t *gfx);


#endif/*_GFX_H*/

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

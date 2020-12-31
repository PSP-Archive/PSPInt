/*
 * ============================================================================
 *  Title:    Controller pads via Joe Fisher's Classic Gaming Controller
 *  Author:   J. Zbiciak
 *  $Id$
 * ============================================================================
 *  Some code in this module comes from Joe Fisher's reference code.
 * ============================================================================
 *  This module implements the controller pads.
 *  Pads are peripherals that extend periph_t.
 * ============================================================================
 */

static const char rcs_id[]="$Id$";

#include "config.h"
#include "periph/periph.h"
#include "pads/pads_cgc.h"

/* ======================================================================== */
/*  PAD_CGC_INIT -- Initializes a Classic Gaming Controller interface.      */
/* ======================================================================== */
int pad_cgc_init
(
    pad_cgc_t       *pad,           /*  pad_cgc_t structure to initialize   */
    uint_32         addr,           /*  Base address of pad.                */
    int             cgc_num,        /*  CGC number in system (Win32)        */
    char*           cgc_dev         /*  CGC number in system (Linux)        */
)
{

#ifdef WIN32
    extern int pad_cgc_win32_init(pad_cgc_t*, uint_32, int);
    return pad_cgc_win32_init(pad, addr, cgc_num);
#endif

#ifdef linux  
    extern int pad_cgc_linux_init(pad_cgc_t*, uint_32, char*);
    return pad_cgc_linux_init(pad, addr, cgc_dev);
#endif
                  
    (void)pad; (void)addr; (void)cgc_num; 
    fprintf(stderr, "Error:  CGC not supported on this platform.\n");
    return -1;
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
/*                 Copyright (c) 2004-+Inf, Joseph Zbiciak                  */
/* ======================================================================== */


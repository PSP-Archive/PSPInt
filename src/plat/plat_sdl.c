/*
 * ============================================================================
 *  Title:    Platform-specific Initialization Functions for SDL.
 *  Author:   J. Zbiciak
 *  $Id: plat_sdl.c,v 1.3 1999/09/13 02:35:09 im14u2c Exp $
 * ============================================================================
 *  Platform-specific initialization for SDL.
 * ============================================================================
 *  PLAT_INIT -- Platform-specific initialization. Returns non-zero on fail.
 * ============================================================================
 */

#include "../config.h"
#include <SDL/SDL.h>

#ifdef DIRECT_INTV2PC
# include <unistd.h>
# ifndef WIN32
#  include <sys/io.h>
# endif
# include "../periph/periph.h"
# include "../pads/pads_intv2pc.h"
#endif

int plat_init(void)
{
    /* -------------------------------------------------------------------- */
    /*  Call SDL_Init and ask for Audio and Video.   This call is made      */
    /*  before we drop elevated privs (if we have any), so this should      */
    /*  allow us to get DGA access if we are suid-root.                     */
    /* -------------------------------------------------------------------- */
    if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_JOYSTICK |
                 SDL_INIT_NOPARACHUTE) != 0)
    {   
        fprintf(stderr, "plat_init:  SDL_Init failed!  Reason: '%s'\n",
                SDL_GetError());
        return -1;
    }

    atexit(SDL_Quit);

# if 0 //LUDO: 
#if defined(DIRECT_INTV2PC) && !defined(WIN32)
    /* -------------------------------------------------------------------- */
    /*  If direct hand-controller interface support is compiled in, try     */
    /*  to give ourself permission to the printer-port I/O address ranges.  */
    /*  Do this only if our EUID == 0.  We have to do this blindly,         */
    /*  because we do the platform init before we do argument and config    */
    /*  parsing since we need to drop privs as quickly as possible.         */
    /* -------------------------------------------------------------------- */
    if (!geteuid())
    {
        pads_intv2pc_ports_ok = 0;
        if (ioperm(0x378, 3, 1) == 0) pads_intv2pc_ports_ok |= 1;
        if (ioperm(0x278, 3, 1) == 0) pads_intv2pc_ports_ok |= 2;
        if (ioperm(0x3BC, 3, 1) == 0) pads_intv2pc_ports_ok |= 4;
    }
#endif
# endif

#if defined(DIRECT_INTV2PC) && defined(WIN32)
    pads_intv2pc_ports_ok = 7;
#endif
    

# if 0 //LUDO: 
#if !defined(WIN32) && !defined(macintosh)
    /* -------------------------------------------------------------------- */
    /*  If we have elevated privileges, drop them here, now, immediately.   */
    /* -------------------------------------------------------------------- */
    if (getegid() != getgid()) setegid(getgid());
    if (geteuid() != getuid()) seteuid(getuid());
#endif
# endif

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
/* ======================================================================== */
/*                 Copyright (c) 1998-1999, Joseph Zbiciak                  */
/* ======================================================================== */

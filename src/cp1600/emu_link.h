/* ======================================================================== */
/*  EMU_LINK                                                                */
/*                                                                          */
/*  Allows programs running inside the emulator to invoke emulator-specific */
/*  functionality.  jzIntv performs this task via the SIN instruction.      */
/* ======================================================================== */

#ifndef EMU_LINK_H_
#define EMU_LINK_H_

typedef int (*emu_link_api_t)(cp1600_t *, int *);

/* ======================================================================== */
/*  EMU_LINK_INIT -- Initialize EMU_LINK subsystem.                         */
/* ======================================================================== */
int emu_link_init(void);

/* ======================================================================== */
/*  EMU_LINK_REGISTER -- Register an API with EMU_LINK                      */
/* ======================================================================== */
int emu_link_register(emu_link_api_t fn, int callno);

/* ======================================================================== */
/*  EMU_LINK_DISPATCH -- Dispatch to an EMU_LINK API                        */
/* ======================================================================== */
void emu_link_dispatch(cp1600_t *cpu);

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
/*                   Copyright (c) 2005, Joseph Zbiciak                     */
/* ======================================================================== */


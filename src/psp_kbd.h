/*
 *  Copyright (C) 2006 Ludovic Jacomme (ludovic.jacomme@gmail.com)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

# ifndef _PSP_KBD_H_
# define _PSP_KBD_H_

# define PSP_ALL_BUTTON_MASK 0xFFFF

 enum intel_keys_emum {
  INTEL_RESET = 0,   
  INTEL_PD0L_KP1,   
  INTEL_PD0L_KP2,   
  INTEL_PD0L_KP3,   
  INTEL_PD0L_KP4,   
  INTEL_PD0L_KP5,   
  INTEL_PD0L_KP6,   
  INTEL_PD0L_KP7,   
  INTEL_PD0L_KP8,   
  INTEL_PD0L_KP9,   
  INTEL_PD0L_KPC,   
  INTEL_PD0L_KP0,   
  INTEL_PD0L_KPE,   
  INTEL_PD0L_A_T,   
  INTEL_PD0L_A_L,   
  INTEL_PD0L_A_R,   
  INTEL_PD0L_D_E,   
  INTEL_PD0L_D_ENE, 
  INTEL_PD0L_D_NE,  
  INTEL_PD0L_D_NNE, 
  INTEL_PD0L_D_N,   
  INTEL_PD0L_D_NNW, 
  INTEL_PD0L_D_NW,  
  INTEL_PD0L_D_WNW, 
  INTEL_PD0L_D_W,   
  INTEL_PD0L_D_WSW, 
  INTEL_PD0L_D_SW,  
  INTEL_PD0L_D_SSW, 
  INTEL_PD0L_D_S,   
  INTEL_PD0L_D_SSE, 
  INTEL_PD0L_D_SE,  
  INTEL_PD0L_J_E,   
  INTEL_PD0L_J_ENE, 
  INTEL_PD0L_J_NE,  
  INTEL_PD0L_J_NNE, 
  INTEL_PD0L_J_N,   
  INTEL_PD0L_J_NNW, 
  INTEL_PD0L_J_NW,  
  INTEL_PD0L_J_WNW, 
  INTEL_PD0L_J_W,   
  INTEL_PD0L_J_WSW, 
  INTEL_PD0L_J_SW,  
  INTEL_PD0L_J_SSW, 
  INTEL_PD0L_J_S,   
  INTEL_PD0L_J_SSE, 
  INTEL_PD0L_J_SE,  
  INTEL_PD0L_J_ESE, 
  INTEL_PD0R_KP1,   
  INTEL_PD0R_KP2,   
  INTEL_PD0R_KP3,   
  INTEL_PD0R_KP4,   
  INTEL_PD0R_KP5,   
  INTEL_PD0R_KP6,   
  INTEL_PD0R_KP7,   
  INTEL_PD0R_KP8,   
  INTEL_PD0R_KP9,   
  INTEL_PD0R_KPC,   
  INTEL_PD0R_KP0,   
  INTEL_PD0R_KPE,   
  INTEL_PD0R_A_T,   
  INTEL_PD0R_A_L,   
  INTEL_PD0R_A_R,   
  INTEL_PD0R_D_E,   
  INTEL_PD0R_D_ENE, 
  INTEL_PD0R_D_NE,  
  INTEL_PD0R_D_NNE, 
  INTEL_PD0R_D_N,   
  INTEL_PD0R_D_NNW, 
  INTEL_PD0R_D_NW,  
  INTEL_PD0R_D_WNW, 
  INTEL_PD0R_D_W,   
  INTEL_PD0R_D_WSW, 
  INTEL_PD0R_D_SW,  
  INTEL_PD0R_D_SSW, 
  INTEL_PD0R_D_S,   
  INTEL_PD0R_D_SSE, 
  INTEL_PD0R_D_SE,  
  INTEL_PD0R_J_E,   
  INTEL_PD0R_J_ENE, 
  INTEL_PD0R_J_NE,  
  INTEL_PD0R_J_NNE, 
  INTEL_PD0R_J_N,   
  INTEL_PD0R_J_NNW, 
  INTEL_PD0R_J_NW,  
  INTEL_PD0R_J_WNW, 
  INTEL_PD0R_J_W,   
  INTEL_PD0R_J_WSW, 
  INTEL_PD0R_J_SW,  
  INTEL_PD0R_J_SSW, 
  INTEL_PD0R_J_S,   
  INTEL_PD0R_J_SSE, 
  INTEL_PD0R_J_SE,  
  INTEL_PD0R_J_ESE, 
  INTEL_MAX_KEY 
  };

# define KBD_UP           0
# define KBD_RIGHT        1
# define KBD_DOWN         2
# define KBD_LEFT         3
# define KBD_TRIANGLE     4
# define KBD_CIRCLE       5
# define KBD_CROSS        6
# define KBD_SQUARE       7
# define KBD_SELECT       8
# define KBD_START        9
# define KBD_HOME        10
# define KBD_HOLD        11
# define KBD_LTRIGGER    12
# define KBD_RTRIGGER    13

# define KBD_MAX_BUTTONS 14

# define KBD_JOY_UP      14
# define KBD_JOY_RIGHT   15
# define KBD_JOY_DOWN    16
# define KBD_JOY_LEFT    17

# define KBD_ALL_BUTTONS 18

  typedef struct intel_key_trans {
    char   name[15];
    u32   *pvalue;
    u32    mask_and[2];
    u32    mask_or[2];

  } intel_key_trans;

  extern int psp_screenshot_mode;
  extern int psp_kbd_mapping[ KBD_ALL_BUTTONS ];
  extern struct intel_key_trans psp_intel_key_to_name[INTEL_MAX_KEY];

  extern int  psp_update_keys(void);
  extern void kbd_wait_start(void);
  extern void psp_init_keyboard(void);
  extern void psp_kbd_wait_no_button(void);
  extern int  psp_kbd_is_danzeff_mode(void);
  extern int psp_kbd_load_mapping(char *kbd_filename);

# endif

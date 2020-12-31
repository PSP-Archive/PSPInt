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

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <fcntl.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>
#include <SDL/SDL_ttf.h>

#include <pspctrl.h>
#include <pspkernel.h>
#include <pspdebug.h>
#include <pspiofilemgr.h>

#include "global.h"
#include "config.h"
#include "psp_sdl.h"
#include "psp_kbd.h"
#include "psp_menu.h"
#include "psp_fmgr.h"
#include "psp_menu_kbd.h"
#include "psp_menu_set.h"
#include "psp_battery.h"

extern SDL_Surface *back_surface;
static int psp_menu_dirty = 1;

# define MENU_SET_JOYSTICK     0
# define MENU_SET_ANALOG       1
# define MENU_SET_SOUND        2
# define MENU_SET_SKIP_FPS     3
# define MENU_SET_RENDER       4
# define MENU_SET_CLOCK        5

# define MENU_SET_LOAD         6
# define MENU_SET_SAVE         7
# define MENU_SET_RESET        8
                                 
# define MENU_SET_BACK         9

# define MAX_MENU_SET_ITEM (MENU_SET_BACK + 1)

  static menu_item_t menu_list[] =
  {
    { "Active Joystick     :" },
    { "Swap Analog/Cursor  :" },
    { "Sound enable        :" },
    { "Skip frame          :" },
    { "Render mode         :" },
    { "Clock frequency     :" },
    { "Load settings"         },
    { "Save settings"         },
    { "Reset settings"        },
    { "Back to Menu"          }
  };

  static int cur_menu_id = MENU_SET_LOAD;

  static int intel_snd_enable        = 0;
  static int intel_render_mode       = 0;
  static int psp_reverse_analog      = 0;
  static int psp_active_joystick     = 0;
  static int psp_cpu_clock           = 222;
  static int intel_skip_fps          = 0;
  static int intel_slow_down_max     = 0;

static void
psp_settings_menu_reset(void);

static void 
psp_display_screen_settings_menu(void)
{
  char buffer[64];
  int menu_id = 0;
  int slot_id = 0;
  int color   = 0;
  int x       = 0;
  int y       = 0;
  int y_step  = 0;

  //if (psp_menu_dirty) 
  {

    psp_sdl_blit_background();
    psp_menu_dirty = 0;

    psp_sdl_draw_rectangle(10,10,459,249,PSP_MENU_BORDER_COLOR,0);
    psp_sdl_draw_rectangle(11,11,457,247,PSP_MENU_BORDER_COLOR,0);

    psp_sdl_back2_print( 30, 6, " Start+L+R: EXIT ", PSP_MENU_WARNING_COLOR);

    psp_display_screen_menu_battery();

    psp_sdl_back2_print( 370, 6, " R: RESET ", PSP_MENU_NOTE_COLOR);

    psp_sdl_back2_print(30, 254, " []: Cancel  O/X: Valid  SELECT: Back ", 
                       PSP_MENU_BORDER_COLOR);

    psp_sdl_back2_print(370, 254, " By Zx-81 ",
                       PSP_MENU_AUTHOR_COLOR);
  }
  
  x      = 20;
  y      = 25;
  y_step = 10;
  
  for (menu_id = 0; menu_id < MAX_MENU_SET_ITEM; menu_id++) {
    color = PSP_MENU_TEXT_COLOR;
    if (cur_menu_id == menu_id) color = PSP_MENU_SEL_COLOR;

    psp_sdl_back2_print(x, y, menu_list[menu_id].title, color);

    if (menu_id == MENU_SET_SOUND) {
      if (intel_snd_enable) strcpy(buffer,"yes");
      else                 strcpy(buffer,"no ");
      string_fill_with_space(buffer, 4);
      psp_sdl_back2_print(210, y, buffer, color);
    } else
    if (menu_id == MENU_SET_ANALOG) {
      if (psp_reverse_analog) strcpy(buffer,"yes");
      else                    strcpy(buffer,"no ");
      string_fill_with_space(buffer, 4);
      psp_sdl_back2_print(210, y, buffer, color);
    } else
    if (menu_id == MENU_SET_JOYSTICK) {
      if (psp_active_joystick) strcpy(buffer,"player 2");
      else                     strcpy(buffer,"player 1 ");
      string_fill_with_space(buffer, 10);
      psp_sdl_back2_print(210, y, buffer, color);
    } else
    if (menu_id == MENU_SET_SKIP_FPS) {
      sprintf(buffer,"%d", intel_skip_fps);
      string_fill_with_space(buffer, 4);
      psp_sdl_back2_print(210, y, buffer, color);
    } else
    if (menu_id == MENU_SET_RENDER) {

      if (intel_render_mode == INTEL_RENDER_NORMAL) strcpy(buffer, "normal");
      else 
      if (intel_render_mode == INTEL_RENDER_X2    ) strcpy(buffer, "fit");

      string_fill_with_space(buffer, 13);
      psp_sdl_back2_print(210, y, buffer, color);
    } else
    if (menu_id == MENU_SET_CLOCK) {
      sprintf(buffer,"%d", psp_cpu_clock);
      string_fill_with_space(buffer, 4);
      psp_sdl_back2_print(210, y, buffer, color);

      y += y_step;
    } else
    if (menu_id == MENU_SET_RESET) {
      y += y_step;
    }

    y += y_step;
  }

  psp_menu_display_save_name();
}

static void
psp_settings_menu_clock(int step)
{
  if (step > 0) {
    if (psp_cpu_clock == 222) {
      psp_cpu_clock = 300;
    } else if (psp_cpu_clock == 300) {
      psp_cpu_clock = 333;
    } else {
      psp_cpu_clock = 222;
    }
  } else {
    if (psp_cpu_clock == 222) {
      psp_cpu_clock = 333;
    } else if (psp_cpu_clock == 333) {
      psp_cpu_clock = 300;
    } else {
      psp_cpu_clock = 222;
    }
  }
}

static void
psp_settings_menu_skip_fps(int step)
{
  if (step > 0) {
    if (intel_skip_fps < 25) intel_skip_fps++;
  } else {
    if (intel_skip_fps > 0) intel_skip_fps--;
  }
}

static void
psp_settings_menu_render(int step)
{
  if (step > 0) {
    if (intel_render_mode < INTEL_LAST_RENDER) intel_render_mode++;
    else                                       intel_render_mode = 0;
  } else {
    if (intel_render_mode > 0) intel_render_mode--;
    else                       intel_render_mode = INTEL_LAST_RENDER;
  }
}

static void
psp_settings_menu_init(void)
{
  intel_snd_enable     = INTEL.intel_snd_enable;
  intel_render_mode    = INTEL.intel_render_mode;
  intel_skip_fps       = INTEL.psp_skip_max_frame;
  intel_slow_down_max  = INTEL.intel_slow_down_max;
  psp_cpu_clock        = INTEL.psp_cpu_clock;
  psp_reverse_analog   = INTEL.psp_reverse_analog;
  psp_active_joystick  = INTEL.psp_active_joystick;
}

static void
psp_settings_menu_load(int format)
{
  int ret;

  ret = psp_fmgr_menu(format);
  if (ret ==  1) /* load OK */
  {
    psp_display_screen_settings_menu();
    psp_sdl_back2_print(270,  80, "File loaded !", 
                       PSP_MENU_NOTE_COLOR);
    psp_menu_dirty = 1;
    psp_sdl_flip();
    sleep(1);
    psp_settings_menu_init();
  }
  else 
  if (ret == -1) /* Load Error */
  {
    psp_display_screen_settings_menu();
    psp_sdl_back2_print(270,  80, "Can't load file !", 
                       PSP_MENU_WARNING_COLOR);
    psp_menu_dirty = 1;
    psp_sdl_flip();
    sleep(1);
  }
}

static void
psp_settings_menu_validate(void)
{
  /* Validate */
  INTEL.intel_snd_enable    = intel_snd_enable;
  INTEL.intel_render_mode   = intel_render_mode;
  INTEL.psp_cpu_clock       = psp_cpu_clock;
  INTEL.psp_reverse_analog  = psp_reverse_analog;
  if (INTEL.psp_active_joystick != psp_active_joystick) {
    INTEL.psp_active_joystick = psp_active_joystick;
    intel_kbd_reset();
  }
  INTEL.psp_skip_max_frame  = intel_skip_fps;
  INTEL.intel_slow_down_max = intel_slow_down_max;
  INTEL.psp_skip_cur_frame  = 0;

  scePowerSetClockFrequency(INTEL.psp_cpu_clock, INTEL.psp_cpu_clock, INTEL.psp_cpu_clock/2);
}

static void
psp_settings_menu_save_config()
{
  int error;

  psp_settings_menu_validate();

  error = intel_save_settings();

  if (! error) /* save OK */
  {
    psp_display_screen_settings_menu();
    psp_sdl_back2_print(270,  80, "File saved !", 
                       PSP_MENU_NOTE_COLOR);
    psp_menu_dirty = 1;
    psp_sdl_flip();
    sleep(1);
  }
  else 
  {
    psp_display_screen_settings_menu();
    psp_sdl_back2_print(270,  80, "Can't save file !", 
                       PSP_MENU_WARNING_COLOR);
    psp_menu_dirty = 1;
    psp_sdl_flip();
    sleep(1);
  }
}

int
psp_settings_menu_exit(void)
{
  SceCtrlData c;

  psp_display_screen_settings_menu();
  psp_sdl_back2_print(270,  80, "press X to confirm !", PSP_MENU_WARNING_COLOR);
  psp_menu_dirty = 1;
  psp_sdl_flip();

  psp_kbd_wait_no_button();

  do
  {
    sceCtrlReadBufferPositive(&c, 1);
    c.Buttons &= PSP_ALL_BUTTON_MASK;

    if (c.Buttons & PSP_CTRL_CROSS) psp_sdl_exit(0);

  } while (c.Buttons == 0);

  psp_kbd_wait_no_button();

  return 0;
}

static void
psp_settings_menu_save()
{
  int error;

  psp_settings_menu_validate();
  error = intel_save_settings();

  if (! error) /* save OK */
  {
    psp_display_screen_settings_menu();
    psp_sdl_back2_print(270,  80, "File saved !", 
                       PSP_MENU_NOTE_COLOR);
    psp_menu_dirty = 1;
    psp_sdl_flip();
    sleep(1);
  }
  else 
  {
    psp_display_screen_settings_menu();
    psp_sdl_back2_print(270,  80, "Can't save file !", 
                       PSP_MENU_WARNING_COLOR);
    psp_menu_dirty = 1;
    psp_sdl_flip();
    sleep(1);
  }
}

static void
psp_settings_menu_reset(void)
{
  psp_display_screen_settings_menu();
  psp_sdl_back2_print(270, 80, "Reset Settings !", 
                     PSP_MENU_WARNING_COLOR);
  psp_menu_dirty = 1;
  psp_sdl_flip();
  intel_default_settings();
  psp_settings_menu_init();
  sleep(1);
}

int 
psp_settings_menu(void)
{
  SceCtrlData c;
  long        new_pad;
  long        old_pad;
  int         last_time;
  int         end_menu;

  psp_kbd_wait_no_button();

  old_pad   = 0;
  last_time = 0;
  end_menu  = 0;

  psp_settings_menu_init();

  psp_menu_dirty = 1;

  while (! end_menu)
  {
    psp_display_screen_settings_menu();
    psp_sdl_flip();

    while (1)
    {
      sceCtrlReadBufferPositive(&c, 1);
      c.Buttons &= PSP_ALL_BUTTON_MASK;

      if (c.Buttons) break;
    }

    new_pad = c.Buttons;

    if ((old_pad != new_pad) || ((c.TimeStamp - last_time) > PSP_MENU_MIN_TIME)) {
      last_time = c.TimeStamp;
      old_pad = new_pad;

    } else continue;

    if ((c.Buttons & (PSP_CTRL_LTRIGGER|PSP_CTRL_RTRIGGER|PSP_CTRL_START)) ==
        (PSP_CTRL_LTRIGGER|PSP_CTRL_RTRIGGER|PSP_CTRL_START)) {
      /* Exit ! */
      psp_sdl_exit(0);
    } else
    if ((c.Buttons & PSP_CTRL_RTRIGGER) == PSP_CTRL_RTRIGGER) {
      psp_settings_menu_reset();
      end_menu = 1;
    } else
    if ((new_pad & PSP_CTRL_CROSS ) || 
        (new_pad & PSP_CTRL_CIRCLE) || 
        (new_pad & PSP_CTRL_RIGHT ) ||
        (new_pad & PSP_CTRL_LEFT  )) 
    {
      int step;

      if (new_pad & PSP_CTRL_LEFT)  step = -1;
      else 
      if (new_pad & PSP_CTRL_RIGHT) step =  1;
      else                          step =  0;

      switch (cur_menu_id ) 
      {
        case MENU_SET_SOUND      : intel_snd_enable = ! intel_snd_enable;
        break;              
        case MENU_SET_SKIP_FPS   : psp_settings_menu_skip_fps( step );
        break;              
        case MENU_SET_ANALOG     : psp_reverse_analog = ! psp_reverse_analog;
        break;              
        case MENU_SET_JOYSTICK   : psp_active_joystick = ! psp_active_joystick;
        break;              
        case MENU_SET_RENDER     : psp_settings_menu_render( step );
        break;              
        case MENU_SET_CLOCK      : psp_settings_menu_clock( step );
        break;
        case MENU_SET_LOAD       : psp_settings_menu_load(FMGR_FORMAT_SET);
                                   psp_menu_dirty = 1;
                                   old_pad = new_pad = 0;
        break;              
        case MENU_SET_SAVE       : psp_settings_menu_save();
                                   psp_menu_dirty = 1;
                                   old_pad = new_pad = 0;
        break;                     
        case MENU_SET_RESET      : psp_settings_menu_reset();
        break;                     
                                   
        case MENU_SET_BACK       : end_menu = 1;
        break;                     
      }

    } else
    if(new_pad & PSP_CTRL_UP) {

      if (cur_menu_id > 0) cur_menu_id--;
      else                 cur_menu_id = MAX_MENU_SET_ITEM-1;

    } else
    if(new_pad & PSP_CTRL_DOWN) {

      if (cur_menu_id < (MAX_MENU_SET_ITEM-1)) cur_menu_id++;
      else                                     cur_menu_id = 0;

    } else  
    if(new_pad & PSP_CTRL_SQUARE) {
      /* Cancel */
      end_menu = -1;
    } else 
    if(new_pad & PSP_CTRL_SELECT) {
      /* Back to INTEL */
      end_menu = 1;
    }
  }
 
  if (end_menu > 0) {
    psp_settings_menu_validate();
  }

  psp_kbd_wait_no_button();

  psp_sdl_clear_screen( PSP_MENU_BLACK_COLOR );
  psp_sdl_flip();
  psp_sdl_clear_screen( PSP_MENU_BLACK_COLOR );
  psp_sdl_flip();

  return 1;
}


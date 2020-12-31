/*
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

# ifndef _PSP_SDL_H_
# define _PSP_SDL_H_

# define psp_debug(m)   loc_psp_debug(__FILE__,__LINE__,m)

# define PSP_SDL_NOP   0
# define PSP_SDL_XOR   1

# define PSP_LINE_SIZE  512

# define PSP_SDL_SCREEN_WIDTH    480
# define PSP_SDL_SCREEN_HEIGHT   272

  typedef unsigned char   uchar;
  typedef unsigned int    uint;
  typedef unsigned short  ushort;


  extern int psp_load_fonts(void);
  extern int psp_print_text(char * str, int colour, int v, int h);

  extern void loc_psp_debug(char *file, int line, char *message);

  /* PG -> SDL function */

  extern void psp_sdl_print(int x,int y, char *str, int color);
  extern void psp_sdl_clear_screen(int color);
  extern void psp_sdl_fill_rectangle(int x, int y, int w, int h, int color, int mode);
  extern void psp_sdl_draw_rectangle(int x, int y, int w, int h, int border, int mode);
  extern void psp_sdl_put_char(int x, int y, int color, int bgcolor, uchar c, int drawfg, int drawbg);
  extern void psp_sdl_fill_print(int x,int y,const char *str, int color, int bgcolor);
  extern void psp_sdl_flip(void);

  extern void psp_sdl_lock(void);
  extern void psp_sdl_unlock(void);
  extern void psp_sdl_flush(void);
  extern void psp_sdl_save_bmp(char *filename);
  extern void psp_sdl_blit_background();
  extern void psp_sdl_exit(int status);

# endif

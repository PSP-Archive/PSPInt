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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#include <pspctrl.h>

#include "global.h"
#include "psp_sdl.h"
#include "psp_sound.h"

void 
audio_pause(void)
{
  if (INTEL.intel_snd_enable) {
    SDL_PauseAudio(1);
  }
}

void 
audio_resume(void)
{
  if (INTEL.intel_snd_enable) {
    SDL_PauseAudio(0);
  }
}

void 
audio_shutdown(void)
{
  SDL_CloseAudio();
}

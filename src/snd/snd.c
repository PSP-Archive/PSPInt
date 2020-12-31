/*
 * ============================================================================
 *  Title:    Sound Interface Abstraction
 *  Author:   J. Zbiciak
 *  $Id: snd.c,v 1.14 2001/11/02 02:00:03 im14u2c Exp $
 * ============================================================================
 *
 * ============================================================================
 *  SND_TICK     -- Update state of the sound universe.  Drains audio data
 *                  from the PSGs and prepares it for SDL's sound layer.
 *  SND_FILL     -- Audio callback used by SDL for filling SDL's buffers.
 *  SND_REGISTER -- Registers a PSG with the SND module.
 *  SND_INIT     -- Initialize a SND_T
 * ============================================================================
 */

static const char rcs_id[]="$Id: snd.c,v 1.14 2001/11/02 02:00:03 im14u2c Exp $";

#include <SDL/SDL.h>
#include <SDL/SDL_audio.h>
#include <SDL/SDL_error.h>
#include "config.h"
#include "global.h"
#include "periph/periph.h"
#include "snd.h"

int snd_buf_size = SND_BUF_SIZE_DEFAULT;
int snd_buf_cnt  = SND_BUF_CNT_DEFAULT;
LOCAL sint_32 *mixbuf = NULL;

/* ======================================================================== */
/*  Cheesy WAV header.                                                      */
/* ======================================================================== */

LOCAL uint_8 snd_wav_hdr[44] =
{
    0x52, 0x49, 0x46, 0x46, 0x24, 0x08, 0x02, 0x00, 
    0x57, 0x41, 0x56, 0x45, 0x66, 0x6D, 0x74, 0x20,
    0x10, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 
    0x44, 0xAC, 0x00, 0x00, 0x88, 0x58, 0x01, 0x00,  /* rate in 1st 4 here */
    0x02, 0x00, 0x10, 0x00, 0x64, 0x61, 0x74, 0x61, 
    0x00, 0x00, 0x00, 0x00,  /* these four bytes are total file length */
};

/*
 * ============================================================================
 *  SND_TICK     -- Update state of the sound universe.  Drains audio data
 *                  from the PSGs and prepares it for SDL's sound layer.
 * ============================================================================
 */
uint_32 snd_tick(periph_p periph, uint_32 len)
{
    snd_t *snd = (snd_t*)periph;
    int min_num_dirty;
    int i, j, k, mix, drop = 0, will_drop;
    sint_16 *clean;
    uint_64 new_now;
    int not_silent = snd->raw_start;
    int dly_drop = 0;

    /* -------------------------------------------------------------------- */
    /*  Trival case:  No sound devices == no work for us.                   */
    /* -------------------------------------------------------------------- */
    if (snd->src_cnt == 0)
    {
        return len;
    }


    /* -------------------------------------------------------------------- */
    /*  If all of our buffers are dirty, we can't do anything, so return    */
    /*  the fact that we've made no progress.                               */
    /* -------------------------------------------------------------------- */
    if (snd->mixbuf.num_clean == 0)
    {
        return 0;
    }

#if 0
    /* -------------------------------------------------------------------- */
    /*  Get any buffer-drop requests from the snd_fill routine.             */
    /* -------------------------------------------------------------------- */
    SDL_LockAudio();
    drop += snd->mixbuf.drop;
    SDL_UnlockAudio();
#endif


    SDL_LockAudio();
    /* -------------------------------------------------------------------- */
    /*  Step through all of the sound sources' dirty buffers and determine  */
    /*  how many additional buffers we'll be dropping.  Move them to the    */
    /*  front.                                                              */
    /* -------------------------------------------------------------------- */
    will_drop = 0;
    for (i = 0; i < snd->src_cnt; i++)
    {
        if (snd->src[i]->drop > will_drop)
            will_drop = snd->src[i]->drop;
        snd->src[i]->tot_drop += snd->src[i]->drop;
        snd->src[i]->drop      = 0;
    }

    drop += will_drop;

    /* -------------------------------------------------------------------- */
    /*  If we're dumping sound to a raw audio file, don't actually drop     */
    /*  anything until after we've written the audio out to file.           */
    /* -------------------------------------------------------------------- */
    if (snd->raw_file) 
    {
        dly_drop = drop;
        drop = 0;
    }

    /* -------------------------------------------------------------------- */
    /*  Calculate the minimum number of dirty buffers to process versus     */
    /*  the number of clean buffers we have available, also taking into     */
    /*  account the room we'll have since we're dropping buffers.           */
    /* -------------------------------------------------------------------- */
    min_num_dirty = snd->mixbuf.num_clean + drop;
    for (i = 0; i < snd->src_cnt; i++)
    {
        if (snd->src[i]->num_dirty < min_num_dirty)
            min_num_dirty = snd->src[i]->num_dirty;
    }

    /* -------------------------------------------------------------------- */
    /*  Update the drop count by the number of buffers that we can drop     */
    /*  during this update pass.                                            */
    /* -------------------------------------------------------------------- */
    snd->mixbuf.drop = drop - min_num_dirty;
    if (snd->mixbuf.drop < 0) 
        snd->mixbuf.drop = 0;
    snd->mixbuf.tot_drop += drop - snd->mixbuf.drop;

    SDL_UnlockAudio();

    /* -------------------------------------------------------------------- */
    /*  Merge the dirty buffers together into mix buffers, and place the    */
    /*  dirty buffers back on the clean list.  This will allows the sound   */
    /*  devices to continue generating sound while we wait for the mixed    */
    /*  data to play.                                                       */
    /* -------------------------------------------------------------------- */
    for (i = drop; i < min_num_dirty; i++)
    {
        /* ---------------------------------------------------------------- */
        /*  Remove a buffer from the clean list for mixing.                 */
        /* ---------------------------------------------------------------- */
        SDL_LockAudio();
        clean = snd->mixbuf.clean[--snd->mixbuf.num_clean];
        SDL_UnlockAudio();

        /* ---------------------------------------------------------------- */
        /*  Simple case:  One source -- no mixing required.                 */
        /* ---------------------------------------------------------------- */
        if (snd->src_cnt == 1)
        {
            sint_16 *tmp;

            /*memcpy(clean, snd->src[0]->dirty[i], snd_buf_size * 2);*/

            /* ------------------------------------------------------------ */
            /*  XXX: THIS IS AN EVIL HACK.  I SHOULD BE DOING THE MEMCPY!   */
            /* ------------------------------------------------------------ */
            tmp = snd->src[0]->dirty[i];
            snd->src[0]->dirty[i] = clean;
            clean = tmp;

            /* ------------------------------------------------------------ */
            /*  Handle writing sound to raw files.                          */
            /* ------------------------------------------------------------ */
            if (snd->raw_file || !snd->raw_start)
                for (j = 0; j < snd_buf_size && !not_silent; j++)
                    not_silent = clean[j];
            
            goto one_source;
        }

        /* ---------------------------------------------------------------- */
        /*  Accumulate all of the source buffers at 32-bit precision.       */
        /* ---------------------------------------------------------------- */
        memset(mixbuf, 0, snd_buf_size * sizeof(int));
        for (j = 0; j < snd->src_cnt; j++)
        {
            for (k = 0; k < snd_buf_size; k++)
                mixbuf[k] += snd->src[j]->dirty[i][k];
        }

        /* ---------------------------------------------------------------- */
        /*  Saturate the mix results to 16-bit precision and place them     */
        /*  in the formerly-clean mix buffer.                               */
        /* ---------------------------------------------------------------- */
        for (j = 0; j < snd_buf_size; j++)
        {
            mix = mixbuf[j];
            if (mix >  0x7FFF) mix =  0x7FFF;
            if (mix < -0x8000) mix = -0x8000;
            clean[j] = mix;
            not_silent |= mix;
        }

        /* ---------------------------------------------------------------- */
        /*  "Atomically" place this in the dirty buffer list so that the    */
        /*  snd_fill() routine can get to it.  Handle delayed-drops due to  */
        /*  file writing here.                                              */
        /* ---------------------------------------------------------------- */
one_source:

        if (dly_drop == 0)
        {
            SDL_LockAudio();
            if (snd->mixbuf.num_dirty == 0)
                snd->mixbuf.top_dirty_ptr = 0;

            snd->mixbuf.dirty[snd->mixbuf.num_dirty++] = clean;
            SDL_UnlockAudio();
        }

        /* ---------------------------------------------------------------- */
        /*  If we're also writing this out to an audio file, do that last.  */
        /* ---------------------------------------------------------------- */
        if (snd->raw_file && not_silent)
        {
            fwrite(clean, sizeof(sint_16), snd_buf_size, 
                    snd->raw_file);
            snd->raw_start = 1;
        }

        /* ---------------------------------------------------------------- */
        /*  If this frame wasn't actually put on the mixer's dirty list     */
        /*  because we're dropping it, then put it back on the clean list.  */
        /* ---------------------------------------------------------------- */
        if (dly_drop > 0)
        {
            SDL_LockAudio();
            snd->mixbuf.clean[snd->mixbuf.num_clean++] = clean;
            SDL_UnlockAudio();
            dly_drop--;
        }
    }

    /* -------------------------------------------------------------------- */
    /*  Now fix up our sources' clean and dirty lists.                      */
    /* -------------------------------------------------------------------- */
    for (i = 0; i < snd->src_cnt; i++)
    {
        /* ---------------------------------------------------------------- */
        /*  Place the formerly dirty buffers on the clean list.             */
        /* ---------------------------------------------------------------- */
        for (j = 0; j < min_num_dirty; j++)
            snd->src[i]->clean[snd->src[i]->num_clean++] = 
                snd->src[i]->dirty[j];

        /* ---------------------------------------------------------------- */
        /*  Remove the formerly dirty buffers from the dirty list.          */
        /* ---------------------------------------------------------------- */
        snd->src[i]->num_dirty -= min_num_dirty;

        for (j = 0; j < snd->src[i]->num_dirty; j++)
            snd->src[i]->dirty[j] = snd->src[i]->dirty[min_num_dirty + j];
    }

    /* -------------------------------------------------------------------- */
    /*  Unpause the audio driver if we're sufficiently piped up.            */
    /* -------------------------------------------------------------------- */
    SDL_LockAudio();
    if (snd->mixbuf.num_dirty > snd->mixbuf.num_clean) {
        if (INTEL.intel_snd_enable) {
          SDL_PauseAudio(0);
        }
    }
    SDL_UnlockAudio();

    /* -------------------------------------------------------------------- */
    /*  Finally, figure out how many system ticks this accounted for.       */
    /* -------------------------------------------------------------------- */
    snd->samples += min_num_dirty * snd_buf_size;
    new_now = (double)snd->samples * 894886 / snd->rate;

    if (new_now < snd->periph.now)      /* Uhoh... are we slipping away? */
        fprintf(stderr, "snd_t:  Too slow...\n");   
    else
        len = new_now - snd->periph.now;

    /* -------------------------------------------------------------------- */
    /*  If we're writing a sound sample file, update the file length.       */
    /* -------------------------------------------------------------------- */
    if (snd->raw_file && not_silent)
    {
        uint_32 bytes = ftell(snd->raw_file) - 44;
        fseek(snd->raw_file, 40, SEEK_SET);
        fputc((bytes >>  0) & 0xFF, snd->raw_file);
        fputc((bytes >>  8) & 0xFF, snd->raw_file);
        fputc((bytes >> 16) & 0xFF, snd->raw_file);
        fputc((bytes >> 24) & 0xFF, snd->raw_file);
        fseek(snd->raw_file, 0, SEEK_END);
    }

    return len;
}

/*
 * ============================================================================
 *  SND_FILL     -- Audio callback used by SDL for filling SDL's buffers.
 * ============================================================================
 */
void    snd_fill(void *udata, uint_8 *stream, int len)
{
    snd_t *snd = (snd_t*)udata;

    snd->tot_dirty += snd->mixbuf.num_dirty;
    snd->tot_frame++;

    /* -------------------------------------------------------------------- */
    /*  Sad case:  We're slipping behind.                                   */
    /* -------------------------------------------------------------------- */
    if (snd->mixbuf.num_dirty == 0) 
    {
#if 0
        /* ---------------------------------------------------------------- */
        /*  WARNING: If you enable this drop, wrap the code above which     */
        /*           decrements drop w/ Lock and Unlock!!!                  */
        /*  snd->mixbuf.drop++;                                             */
        /* ---------------------------------------------------------------- */
#endif
        /*printf("snd_fill: drop\n");*/
        return;     /* Ack, nothing to do. */
    }

    /* -------------------------------------------------------------------- */
    /*  Do it if we can.                                                    */
    /* -------------------------------------------------------------------- */
    if (len > 0)
    {
        int i;

        /* ---------------------------------------------------------------- */
        /*  Write out the audio stream PRONTO!                              */
        /* ---------------------------------------------------------------- */
        memcpy(stream, snd->mixbuf.dirty[0], len);

        /* ---------------------------------------------------------------- */
        /*  Put our buffer back on the clean buffer list.                   */
        /* ---------------------------------------------------------------- */
        snd->mixbuf.clean[snd->mixbuf.num_clean++] = snd->mixbuf.dirty[0];

        /* ---------------------------------------------------------------- */
        /*  Update the dirty buffer list.                                   */
        /* ---------------------------------------------------------------- */
        snd->mixbuf.num_dirty--;
        for (i = 0; i < snd->mixbuf.num_dirty; i++)
            snd->mixbuf.dirty[i] = snd->mixbuf.dirty[i + 1];

        return;
    }
}

/*
 * ============================================================================
 *  SND_REGISTER -- Registers a sound input buffer with the sound object
 * ============================================================================
 */
int snd_register
(
    periph_p    per,            /* Sound object.                            */
    snd_buf_t   *src            /* Sound input buffer.                      */
)
{   
    int i;
    snd_t *snd = (snd_t*)per;

    /* -------------------------------------------------------------------- */
    /*  Initialize the sound buffer to all 0's.                             */
    /* -------------------------------------------------------------------- */
    memset(src, 0, sizeof(snd_buf_t));

    /* -------------------------------------------------------------------- */
    /*  Set up its buffers as 'clean'.                                      */
    /* -------------------------------------------------------------------- */
    src->num_clean = snd_buf_cnt;
    src->num_dirty = 0;

    src->buf   = calloc(sizeof(sint_16) * snd_buf_size, src->num_clean);
    src->clean = calloc(src->num_clean, sizeof(sint_16 *));
    src->dirty = calloc(src->num_clean, sizeof(sint_16 *));

    if (!src->buf || !src->clean || !src->dirty)
    {
        fprintf(stderr, "snd_register: Out of memory allocating sndbuf.\n");
        return -1;
    }

    for (i = 0; i < src->num_clean; i++)
    {
        src->clean[i] = src->buf + i * snd_buf_size;
        src->dirty[i] = NULL;
    }


    /* -------------------------------------------------------------------- */
    /*  Add this sound source to our list of sound sources.                 */
    /* -------------------------------------------------------------------- */
    snd->src_cnt++;
    snd->src = realloc(snd->src, snd->src_cnt * sizeof(snd_buf_t*));
    if (!snd->src) 
    {
        fprintf(stderr, "Error:  Out of memory in snd_register()\n");
        return -1;
    }
    snd->src[snd->src_cnt - 1] = src;

    return 0;
}

/*
 * ============================================================================
 *  SND_INIT     -- Initialize a SND_T
 * ============================================================================
 */
int snd_init(snd_t *snd, int rate, char *raw_file)
{
    int i;
    SDL_AudioSpec wanted;

    memset(snd, 0, sizeof(snd_t));

    /* -------------------------------------------------------------------- */
    /*  Set up SDL hooks for SND.  We do this first since 'OpenAudio'       */
    /*  can pass us back a different sound-buffer size than we request.     */
    /* -------------------------------------------------------------------- */
    wanted.freq      = rate;
#ifdef _BIG_ENDIAN 
    wanted.format    = AUDIO_S16MSB;
#else
    wanted.format    = AUDIO_S16;
#endif
#ifndef SDL_08
    wanted.channels  = 1;           /* 1 = mono, 2 = stereo                 */
#endif
    wanted.samples   = snd_buf_size;
    wanted.callback  = snd_fill;
    wanted.userdata  = (void*)snd;

    /* Open the audio device, forcing the desired format */
    if ( SDL_OpenAudio(&wanted, NULL) < 0 ) 
    {
            fprintf(stderr, "snd:  Couldn't open audio: %s\n", SDL_GetError());
            return -1;
    }

    snd_buf_size = wanted.samples;

    /* -------------------------------------------------------------------- */
    /*  Set up SND's internal varables.                                     */
    /*  Channels and Interleave are ignored currently.                      */
    /* -------------------------------------------------------------------- */
    snd->rate = rate;

    /* -------------------------------------------------------------------- */
    /*  Set up SND as a peripheral.                                         */
    /* -------------------------------------------------------------------- */
    snd->periph.read      = NULL;
    snd->periph.write     = NULL;
    snd->periph.peek      = NULL;
    snd->periph.poke      = NULL;
    snd->periph.tick      = snd_tick;
    snd->periph.min_tick  = snd_buf_size * 894886 / (2*rate);
    snd->periph.max_tick  = snd->periph.min_tick * 3;
    snd->periph.addr_base = ~0U;
    snd->periph.addr_mask = ~0U;

    /* -------------------------------------------------------------------- */
    /*  Set up our mix buffers as 'clean'.                                  */
    /* -------------------------------------------------------------------- */
    snd->mixbuf.num_clean = snd_buf_cnt;
    snd->mixbuf.num_dirty = 0;
    snd->mixbuf.buf   = calloc(sizeof(sint_16)*snd_buf_size,snd->mixbuf.num_clean);
    snd->mixbuf.clean = calloc(snd->mixbuf.num_clean, sizeof(sint_16 *));
    snd->mixbuf.dirty = calloc(snd->mixbuf.num_clean, sizeof(sint_16 *));
    mixbuf            = calloc(snd_buf_size, sizeof(uint_32));

    if (!snd->mixbuf.buf || !snd->mixbuf.clean || !snd->mixbuf.dirty)
    {
        fprintf(stderr, "snd_init: Out of memory allocating mixbuf.\n");
        return -1;
    }

    for (i = 0; i < snd->mixbuf.num_clean; i++)
        snd->mixbuf.clean[i] = snd->mixbuf.buf + i * snd_buf_size;

    /* -------------------------------------------------------------------- */
    /*  If the user is dumping audio to a raw-audio file, open 'er up.      */
    /* -------------------------------------------------------------------- */
    if (raw_file)
    {
        snd->raw_file = fopen(raw_file, "wb");
        if (!snd->raw_file)
        {
            fprintf(stderr,"snd:  Error opening '%s' for writing.\n",raw_file);
            perror("fopen");
            return -1;
        }
        snd_wav_hdr[24] = (snd->rate >>  0) & 0xFF;
        snd_wav_hdr[25] = (snd->rate >>  8) & 0xFF;
        snd_wav_hdr[26] = (snd->rate >> 16) & 0xFF;
        snd_wav_hdr[27] = (snd->rate >> 24) & 0xFF;
        fwrite(snd_wav_hdr, 1, 44, snd->raw_file);
        return 0;
    }


    /* -------------------------------------------------------------------- */
    /*  Make sure the audio is paused.                                      */
    /* -------------------------------------------------------------------- */
    SDL_PauseAudio(1);

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


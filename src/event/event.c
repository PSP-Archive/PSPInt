/*
 * ============================================================================
 *  Title:    Event Handling Subsystem
 *  Author:   J. Zbiciak
 *  $Id: event.c,v 1.10 2001/02/03 02:34:21 im14u2c Exp $
 * ============================================================================
 *  The Event subsystem receives input from the keyboard and/or joystick.
 *  Currently, only keyboard input is being processed, as the joystick
 *  driver in SDL is not yet written.
 *
 *  EVENT notifies other subsystems of events by setting flags.  When
 *  an event comes in, the event is looked up in a table and bits in a
 *  uint_32 are set/cleared according to masks assigned to the event.
 *  The masks are specified as four uint_32's:  "AND" and "OR" masks
 *  for a "down" event, and "AND" and "OR" masks for an "up" event.
 *
 *  Event->mask mappings are registered with EVENT via calls to "event_map".
 *  Each event can only be mapped to ONE set of masks, and so the order
 *  in which events are mapped determines the final set of mappings.  (This
 *  allows a specialized config file to override a standard config.)  Mapping
 *  an event to a NULL pointer or to an empty set/clear mask disables the
 *  event.
 * ============================================================================
 *  The following event classes are handled by the EVENT subsystem:
 *
 *   -- Quit events       (eg. somebody hitting the little 'X' in the corner)
 *   -- Keyboard events   (key down / key up)
 *   -- Joystick events   (not yet implemented)
 *   -- Mouse events      (not yet implemented, may never be implemented)
 *   -- Activation events (hide / unhide window)
 *
 *  Event symbol names are assigned in 'event.h', and are stored as strings.
 *  This should simplify dynamic configuration from CFG files.  For 
 *  simplicity's sake, I will convert joystick and quit events to 
 *  keyboard-like events.
 *
 *  While this code currently relies on SDL's event delivery mechanism,
 *  I'm taking great pains to keep it largely independent of SDL.  Here's
 *  hoping I succeed.
 * ============================================================================
 *  EVENT_INIT       -- Initializes the Event Subsystem
 *  EVENT_TICK       -- Processes currently pending events in the event queue
 *  EVENT_MAP        -- Map an event to a set/clear mask and a word pointer.
 * ============================================================================
 */

static const char rcs_id[]="$Id: event.c,v 1.10 2001/02/03 02:34:21 im14u2c Exp $";

#include "../config.h"
#include "periph/periph.h"
#include "pads/pads.h"
#include <SDL/SDL.h>
#include <SDL/SDL_events.h>
#include "joy/joy.h"
#include "event.h"
#include "event_tbl.h"

uint_32 event_count = 0;
static const char *idx_to_name[EVENT_LAST];

/*
 * ============================================================================
 *  EVENT_INIT       -- Initializes the Event subsystem.
 * ============================================================================
 */
int 
event_init(event_t *event)
{
    SDL_Event dummy;
    int i;

    /* -------------------------------------------------------------------- */
    /*  The event 'peripheral' is ticked every so often in order to         */
    /*  drain input events from the event queue and post inputs to the      */
    /*  emulator.                                                           */
    /* -------------------------------------------------------------------- */
    event->periph.read      = NULL;
    event->periph.write     = NULL;
    event->periph.peek      = NULL;
    event->periph.poke      = NULL;
    event->periph.tick      = event_tick;
    event->periph.min_tick  = 3579545 / 480;    /* 120Hz */
    event->periph.max_tick  = 3579545 / 480;    /* 120Hz */
    event->periph.addr_base = ~0U;
    event->periph.addr_mask = 0;

    /* -------------------------------------------------------------------- */
    /*  Allocate the event lookup table (this will be huge!).               */
    /* -------------------------------------------------------------------- */
    event->mask_tbl[0]      = calloc (sizeof(event_mask_t), EVENT_LAST * 4);
    event->max_event        = EVENT_LAST;

    if (!event->mask_tbl[0])
    {
        fprintf(stderr, "event_init: Unable to allocate event mask table\n");
        return -1;
    }
    event->mask_tbl[1]      = event->mask_tbl[0] + EVENT_LAST;
    event->mask_tbl[2]      = event->mask_tbl[1] + EVENT_LAST;
    event->mask_tbl[3]      = event->mask_tbl[2] + EVENT_LAST;

    for (i = 0; i < EVENT_LAST; i++)
        idx_to_name[i] = "bad";
    for (i = 0; i < event_name_count; i++)
        idx_to_name[event_names[i].event_num] = event_names[i].name;

# if 0 //LUDO:
    /* -------------------------------------------------------------------- */
    /*  Set up SDL to filter out the events we're NOT interested in...      */
    /* -------------------------------------------------------------------- */
    SDL_EventState(SDL_MOUSEMOTION,         SDL_IGNORE);
    SDL_EventState(SDL_MOUSEBUTTONDOWN,     SDL_IGNORE);
    SDL_EventState(SDL_MOUSEBUTTONUP,       SDL_IGNORE);
    SDL_EventState(SDL_SYSWMEVENT,          SDL_IGNORE);

    /* -------------------------------------------------------------------- */
    /*  ...and leave us only with the events we ARE interested in.          */
    /* -------------------------------------------------------------------- */
    SDL_EventState(SDL_ACTIVEEVENT,         SDL_ENABLE);
    SDL_EventState(SDL_KEYDOWN,             SDL_ENABLE);
    SDL_EventState(SDL_KEYUP,               SDL_ENABLE);
    SDL_EventState(SDL_QUIT,                SDL_ENABLE);
    SDL_EventState(SDL_JOYAXISMOTION,       SDL_ENABLE);
    SDL_EventState(SDL_JOYHATMOTION,        SDL_ENABLE);
    SDL_EventState(SDL_JOYBUTTONDOWN,       SDL_ENABLE);
    SDL_EventState(SDL_JOYBUTTONUP,         SDL_ENABLE);
    SDL_JoystickEventState(SDL_ENABLE);

    /* -------------------------------------------------------------------- */
    /*  Drain the event queue right now to clear any initial events.        */
    /* -------------------------------------------------------------------- */
    while (SDL_PollEvent(&dummy))
        ;
# endif

    /* -------------------------------------------------------------------- */
    /*  Done!                                                               */
    /* -------------------------------------------------------------------- */
    return 0;
}

/*
 * ============================================================================
 *  EVENT_TICK       -- Processes currently pending events in the event queue
 * ============================================================================
 */
uint_32 
event_tick(periph_p p, uint_32 len)
{
  psp_update_keys();
# if 0 //LUDO:
    /* -------------------------------------------------------------------- */
    /*  First, pump the event loop and gather some events.                  */
    /* -------------------------------------------------------------------- */
    SDL_PumpEvents();

    /* -------------------------------------------------------------------- */
    /*  Now, process all pending events.                                    */
    /* -------------------------------------------------------------------- */
    while (SDL_PollEvent(&ev))
    {
        ev_num = EVENT_IGNORE;
        ex_num = EVENT_IGNORE;

        switch (ev.type)
        {
            /* ------------------------------------------------------------ */
            /*  Activation events:  Only look at whether we're iconified    */
            /*  or not, and convert it to an up/down event on EVENT_HIDE.   */
            /* ------------------------------------------------------------ */
            case SDL_ACTIVEEVENT:
            {
                if (ev.active.state & SDL_APPACTIVE)
                {
                    ev_num  = EVENT_HIDE;
                    ev_updn = ev.active.gain ? UP : DOWN;
                }
                break;
            }

            /* ------------------------------------------------------------ */
            /*  Handle keypresses by grabbing the keysym value as event #   */
            /* ------------------------------------------------------------ */
            case SDL_KEYDOWN:
            {
                ev_updn = DOWN;
                ev_num  = (uint_32)ev.key.keysym.sym;
                break;
            }

            /* ------------------------------------------------------------ */
            /*  Key releases are almost the same as keypresses...           */
            /* ------------------------------------------------------------ */
            case SDL_KEYUP:
            {
                ev_updn = UP;
                ev_num  = (uint_32)ev.key.keysym.sym;
                break;
            }

            /* ------------------------------------------------------------ */
            /*  Outsource all the joystick event decoding...                */
            /* ------------------------------------------------------------ */
            case SDL_JOYAXISMOTION: 
            case SDL_JOYHATMOTION:  
            case SDL_JOYBUTTONDOWN: 
            case SDL_JOYBUTTONUP:   
            {
                joy_decode_event(&ev, &ev_updn, &ev_num, &ex_updn, &ex_num);
                break;
            }

            /* ------------------------------------------------------------ */
            /*  And finally, handle the QUIT event.                         */
            /* ------------------------------------------------------------ */
            case SDL_QUIT:
            {
                ev_updn = DOWN;
                ev_num  = EVENT_QUIT;
                break;
            }

            default:
            {
                break;
            }
        }

        /* ---------------------------------------------------------------- */
        /*  Sanity check the event number, to make sure it's within our     */
        /*  valid range of events.  If the event number appears to be for   */
        /*  an upper-case letter, convert it to lower-case.                 */
        /* ---------------------------------------------------------------- */
again:
        if (ev_num > event->max_event || ev_num == EVENT_IGNORE)
            goto skip;

        if (ev_num >= 'A' && ev_num <= 'Z')
            ev_num = tolower(ev_num);


        /* ---------------------------------------------------------------- */
        /*  Process the event.  If event->mask_tbl[ev_num].word == NULL,    */
        /*  then we aren't interested in this event.                        */
        /* ---------------------------------------------------------------- */
        mask_tbl = &event->mask_tbl[event->cur_kbd][ev_num];
        if (mask_tbl->word == NULL)
            continue;

        /* ---------------------------------------------------------------- */
        /*  Apply the appropriate AND and OR masks to the word.             */
        /* ---------------------------------------------------------------- */
        *mask_tbl->word &= mask_tbl->and_mask[ev_updn];
        *mask_tbl->word |= mask_tbl->or_mask [ev_updn];

skip:
        /* ---------------------------------------------------------------- */
        /*  Joysticks can give an extra event to clear a previous joystick  */
        /*  position.                                                       */
        /* ---------------------------------------------------------------- */
        if (ex_num  != EVENT_IGNORE)
        {
            ev_updn = ex_updn;
            ev_num  = ex_num;
            ex_num  = EVENT_IGNORE;
            goto again;
        }

        /* ---------------------------------------------------------------- */
        /*  Increment the global event counter.                             */
        /* ---------------------------------------------------------------- */
        event_count++;
    }
# endif

    /* -------------------------------------------------------------------- */
    /*  Done!  Return elapsed time.                                         */
    /* -------------------------------------------------------------------- */
    return len;
}

/*
 * ============================================================================
 *  EVENT_MAP        -- Maps an event to a particular AND/OR mask set
 *
 *  NOTE:  This implementation currently uses a gawdawful slow linear search
 *  to look up event names.  Eventually, I'll sort the event name list and
 *  use a binary search, which should be many times faster.  I can justify
 *  this slow lookup for now since this event mapping only occurs at startup.
 * ============================================================================
 */
int event_map
(
    event_t     *event,         /* Event_t structure being set up.          */
    const char  *name,          /* Name of event to map.                    */
    int         map,            /* Keyboard map number to map within.       */
    v_uint_32   *word,          /* Word modified by event, (NULL to ignore) */
    uint_32     and_mask[2],    /* AND masks for event up/down.             */
    uint_32     or_mask[2]      /* OR masks for event up/down.              */
)
{
# if 0 //LUDO: 
    const char *s1;
    char buf[64], *s2;
    int  i, j, num;

    /* -------------------------------------------------------------------- */
    /*  Copy the event name to our local buffer and capitalize it.          */
    /* -------------------------------------------------------------------- */
    i  = 63;
    s1 = name;
    s2 = buf;
    while (i-->0 && *s1)
        *s2++ = toupper(*s1++);
    *s2 = 0;

    /* -------------------------------------------------------------------- */
    /*  Step through the event names and try to find a match.               */
    /* -------------------------------------------------------------------- */
    for (i = 0; i < event_name_count; i++)
        if (!strcmp(buf, event_names[i].name))
            break;

    if (i == event_name_count)
    {
        fprintf(stderr, "event_map:  Invalid event name '%s'\n", buf);
        return -1;
    }

    /* -------------------------------------------------------------------- */
    /*  Sanity check.                                                       */
    /* -------------------------------------------------------------------- */
    if (event_names[i].event_num > event->max_event)
    {
        fprintf(stderr, "event_map:  Event number %d is too large (max %d)\n",
                event_names[i].event_num, event->max_event);
        return -1;
    }

    num = event_names[i].event_num;

    /* -------------------------------------------------------------------- */
    /*  Register ourselves with this event.                                 */
    /* -------------------------------------------------------------------- */
    event->mask_tbl[map][num].word = word;

    for (j = 0; j < 2; j++)
    {
        event->mask_tbl[map][num].and_mask[j] = and_mask[j];
        event->mask_tbl[map][num].or_mask [j] = or_mask [j];
    }

    /* -------------------------------------------------------------------- */
    /*  Done:  Return success.                                              */
    /* -------------------------------------------------------------------- */
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
/*                 Copyright (c) 1998-2000, Joseph Zbiciak                  */
/* ======================================================================== */

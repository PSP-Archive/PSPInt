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

#include <pspctrl.h>
#include <pspkernel.h>
#include <pspdebug.h>

#include "global.h"
#include "config.h"

#include "macros.h"
#include "plat/plat.h"
#include "periph/periph.h"
#include "cp1600/cp1600.h"
#include "mem/mem.h"
#include "icart/icart.h"
#include "bincfg/bincfg.h"
#include "bincfg/legacy.h"
#include "pads/pads.h"
#include "pads/pads_cgc.h"
#include "pads/pads_intv2pc.h"
#include "gfx/gfx.h"
#include "snd/snd.h"
#include "ay8910/ay8910.h"
#include "demo/demo.h"
#include "stic/stic.h"
#include "speed/speed.h"
#include "debug/debug_.h"
#include "event/event.h"
#include "file/file.h"
#include "ivoice/ivoice.h"
#include "cfg/cfg.h"

#include "psp_kbd.h"
#include "psp_menu.h"
#include "psp_danzeff.h"

# define KBD_MIN_ANALOG_TIME  150000
# define KBD_MIN_START_TIME   800000
# define KBD_MAX_EVENT_TIME   500000
# define KBD_MIN_PENDING_TIME 300000
# define KBD_MIN_DANZEFF_TIME 150000
# define KBD_MIN_COMMAND_TIME 100000
# define KBD_MIN_BATTCHECK_TIME 90000000 

 static SceCtrlData    loc_button_data;
 static unsigned int   loc_last_event_time = 0;
 static unsigned int   loc_last_analog_time = 0;
 static int            loc_analog_x_released = 0;
 static int            loc_analog_y_released = 0;
 static long           first_time_stamp = -1;
 static char           loc_button_press[ KBD_MAX_BUTTONS ]; 
 static char           loc_button_release[ KBD_MAX_BUTTONS ]; 
 static unsigned int   loc_button_mask[ KBD_MAX_BUTTONS ] =
 {
   PSP_CTRL_UP         , /*  KBD_UP         */
   PSP_CTRL_RIGHT      , /*  KBD_RIGHT      */
   PSP_CTRL_DOWN       , /*  KBD_DOWN       */
   PSP_CTRL_LEFT       , /*  KBD_LEFT       */
   PSP_CTRL_TRIANGLE   , /*  KBD_TRIANGLE   */
   PSP_CTRL_CIRCLE     , /*  KBD_CIRCLE     */
   PSP_CTRL_CROSS      , /*  KBD_CROSS      */
   PSP_CTRL_SQUARE     , /*  KBD_SQUARE     */
   PSP_CTRL_SELECT     , /*  KBD_SELECT     */
   PSP_CTRL_START      , /*  KBD_START      */
   PSP_CTRL_HOME       , /*  KBD_HOME       */
   PSP_CTRL_HOLD       , /*  KBD_HOLD       */
   PSP_CTRL_LTRIGGER   , /*  KBD_LTRIGGER   */
   PSP_CTRL_RTRIGGER   , /*  KBD_RTRIGGER   */
 };

 static char loc_button_name[ KBD_ALL_BUTTONS ][10] =
 {
   "UP",
   "RIGHT",
   "DOWN",
   "LEFT",
   "TRIANGLE",
   "CIRCLE",
   "CROSS",
   "SQUARE",
   "SELECT",
   "START",
   "HOME",
   "HOLD",
   "LTRIGGER",
   "RTRIGGER",
   "JOY_UP",
   "JOY_RIGHT",
   "JOY_DOWN",
   "JOY_LEFT"
 };

  extern cfg_t top_intv;

#define V(v) (&top_intv.v)

  intel_key_trans psp_intel_key_to_name[INTEL_MAX_KEY] =
  {
    { "RESET",      V(do_reset          ),  { 0,   ~0U },   { 0,   ~0U } },    /* INTEL_RESET,     */
    { "PD0L_KP1",   V(pad0.l[1]         ),  { 0,    ~0U  }, { 0,    0x81 } },  /* INTEL_PD0L_KP1,  */
    { "PD0L_KP2",   V(pad0.l[2]         ),  { 0,    ~0U  }, { 0,    0x41 } },  /* INTEL_PD0L_KP2,  */
    { "PD0L_KP3",   V(pad0.l[3]         ),  { 0,    ~0U  }, { 0,    0x21 } },  /* INTEL_PD0L_KP3,  */
    { "PD0L_KP4",   V(pad0.l[4]         ),  { 0,    ~0U  }, { 0,    0x82 } },  /* INTEL_PD0L_KP4,  */
    { "PD0L_KP5",   V(pad0.l[5]         ),  { 0,    ~0U  }, { 0,    0x42 } },  /* INTEL_PD0L_KP5,  */
    { "PD0L_KP6",   V(pad0.l[6]         ),  { 0,    ~0U  }, { 0,    0x22 } },  /* INTEL_PD0L_KP6,  */
    { "PD0L_KP7",   V(pad0.l[7]         ),  { 0,    ~0U  }, { 0,    0x84 } },  /* INTEL_PD0L_KP7,  */
    { "PD0L_KP8",   V(pad0.l[8]         ),  { 0,    ~0U  }, { 0,    0x44 } },  /* INTEL_PD0L_KP8,  */
    { "PD0L_KP9",   V(pad0.l[9]         ),  { 0,    ~0U  }, { 0,    0x24 } },  /* INTEL_PD0L_KP9,  */
    { "PD0L_KPC",   V(pad0.l[10]        ),  { 0,    ~0U  }, { 0,    0x88 } },  /* INTEL_PD0L_KPC,  */
    { "PD0L_KP0",   V(pad0.l[0]         ),  { 0,    ~0U  }, { 0,    0x48 } },  /* INTEL_PD0L_KP0,  */
    { "PD0L_KPE",   V(pad0.l[11]        ),  { 0,    ~0U  }, { 0,    0x28 } },  /* INTEL_PD0L_KPE,  */
    { "PD0L_A_T",   V(pad0.l[12]        ), { 0,    ~0U  }, { 0,    0xA0 } },   /* INTEL_PD0L_A_T,  */
    { "PD0L_A_L",   V(pad0.l[13]        ), { 0,    ~0U  }, { 0,    0x60 } },   /* INTEL_PD0L_A_L,  */
    { "PD0L_A_R",   V(pad0.l[14]        ), { 0,    ~0U  }, { 0,    0xC0 } },   /* INTEL_PD0L_A_R,  */
    { "PD0L_D_E",   V(pad0.l[15]        ), { ~1,   ~0U  }, { 0,    1  } },     /* INTEL_PD0L_D_E,  */
    { "PD0L_D_ENE", V(pad0.l[15]        ), { ~3,   ~0U  }, { 0,    3  } },     /* INTEL_PD0L_D_ENE,*/
    { "PD0L_D_NE",  V(pad0.l[15]        ), { ~2,   ~0U  }, { 0,    2  } },     /* INTEL_PD0L_D_NE, */
    { "PD0L_D_NNE", V(pad0.l[15]        ), { ~6,   ~0U  }, { 0,    6  } },     /* INTEL_PD0L_D_NNE,*/
    { "PD0L_D_N",   V(pad0.l[15]        ), { ~4,   ~0U  }, { 0,    4  } },     /* INTEL_PD0L_D_N,  */
    { "PD0L_D_NNW", V(pad0.l[15]        ), { ~12,  ~0U  }, { 0,    12 } },     /* INTEL_PD0L_D_NNW,*/
    { "PD0L_D_NW",  V(pad0.l[15]        ), { ~8,   ~0U  }, { 0,    8  } },     /* INTEL_PD0L_D_NW, */
    { "PD0L_D_WNW", V(pad0.l[15]        ), { ~24,  ~0U  }, { 0,    24 } },     /* INTEL_PD0L_D_WNW,*/
    { "PD0L_D_W",   V(pad0.l[15]        ), { ~16,  ~0U  }, { 0,    16 } },     /* INTEL_PD0L_D_W,  */
    { "PD0L_D_WSW", V(pad0.l[15]        ), { ~48,  ~0U  }, { 0,    48 } },     /* INTEL_PD0L_D_WSW,*/
    { "PD0L_D_SW",  V(pad0.l[15]        ), { ~32,  ~0U  }, { 0,    32 } },     /* INTEL_PD0L_D_SW, */
    { "PD0L_D_SSW", V(pad0.l[15]        ), { ~96,  ~0U  }, { 0,    96 } },     /* INTEL_PD0L_D_SSW,*/
    { "PD0L_D_S",   V(pad0.l[15]        ), { ~64,  ~0U  }, { 0,    64 } },     /* INTEL_PD0L_D_S,  */
    { "PD0L_D_SSE", V(pad0.l[15]        ), { ~192, ~0U  }, { 0,    192} },     /* INTEL_PD0L_D_SSE,*/
    { "PD0L_D_SE",  V(pad0.l[15]        ), { ~128, ~0U  }, { 0,    128} },     /* INTEL_PD0L_D_SE, */
    { "PD0L_J_E",   V(pad0.l[16]        ), { 0,    0    }, { 0,    1   }},     /* INTEL_PD0L_J_E,  */
    { "PD0L_J_ENE", V(pad0.l[16]        ), { 0,    0    }, { 0,    3   }},     /* INTEL_PD0L_J_ENE,*/
    { "PD0L_J_NE",  V(pad0.l[16]        ), { 0,    0    }, { 0,    2   }},     /* INTEL_PD0L_J_NE, */
    { "PD0L_J_NNE", V(pad0.l[16]        ), { 0,    0    }, { 0,    6   }},     /* INTEL_PD0L_J_NNE,*/
    { "PD0L_J_N",   V(pad0.l[16]        ), { 0,    0    }, { 0,    4   }},     /* INTEL_PD0L_J_N,  */
    { "PD0L_J_NNW", V(pad0.l[16]        ), { 0,    0    }, { 0,    12  }},     /* INTEL_PD0L_J_NNW,*/
    { "PD0L_J_NW",  V(pad0.l[16]        ), { 0,    0    }, { 0,    8   }},     /* INTEL_PD0L_J_NW, */
    { "PD0L_J_WNW", V(pad0.l[16]        ), { 0,    0    }, { 0,    24  }},     /* INTEL_PD0L_J_WNW,*/
    { "PD0L_J_W",   V(pad0.l[16]        ), { 0,    0    }, { 0,    16  }},     /* INTEL_PD0L_J_W,  */
    { "PD0L_J_WSW", V(pad0.l[16]        ), { 0,    0    }, { 0,    48  }},     /* INTEL_PD0L_J_WSW,*/
    { "PD0L_J_SW",  V(pad0.l[16]        ), { 0,    0    }, { 0,    32  }},     /* INTEL_PD0L_J_SW, */
    { "PD0L_J_SSW", V(pad0.l[16]        ), { 0,    0    }, { 0,    96  }},     /* INTEL_PD0L_J_SSW,*/
    { "PD0L_J_S",   V(pad0.l[16]        ), { 0,    0    }, { 0,    64  }},     /* INTEL_PD0L_J_S,  */
    { "PD0L_J_SSE", V(pad0.l[16]        ), { 0,    0    }, { 0,    192 }},     /* INTEL_PD0L_J_SSE,*/
    { "PD0L_J_SE",  V(pad0.l[16]        ), { 0,    0    }, { 0,    128 }},     /* INTEL_PD0L_J_SE, */
    { "PD0L_J_ESE", V(pad0.l[16]        ), { 0,    0    }, { 0,    129 }},     /* INTEL_PD0L_J_ESE,*/
    { "PD0R_KP1",   V(pad0.r[1]         ),  { 0,    ~0U  }, { 0,    0x81 } },  /* INTEL_PD0R_KP1,  */
    { "PD0R_KP2",   V(pad0.r[2]         ),  { 0,    ~0U  }, { 0,    0x41 } },  /* INTEL_PD0R_KP2,  */
    { "PD0R_KP3",   V(pad0.r[3]         ),  { 0,    ~0U  }, { 0,    0x21 } },  /* INTEL_PD0R_KP3,  */
    { "PD0R_KP4",   V(pad0.r[4]         ),  { 0,    ~0U  }, { 0,    0x82 } },  /* INTEL_PD0R_KP4,  */
    { "PD0R_KP5",   V(pad0.r[5]         ),  { 0,    ~0U  }, { 0,    0x42 } },  /* INTEL_PD0R_KP5,  */
    { "PD0R_KP6",   V(pad0.r[6]         ),  { 0,    ~0U  }, { 0,    0x22 } },  /* INTEL_PD0R_KP6,  */
    { "PD0R_KP7",   V(pad0.r[7]         ),  { 0,    ~0U  }, { 0,    0x84 } },  /* INTEL_PD0R_KP7,  */
    { "PD0R_KP8",   V(pad0.r[8]         ),  { 0,    ~0U  }, { 0,    0x44 } },  /* INTEL_PD0R_KP8,  */
    { "PD0R_KP9",   V(pad0.r[9]         ),  { 0,    ~0U  }, { 0,    0x24 } },  /* INTEL_PD0R_KP9,  */
    { "PD0R_KPC",   V(pad0.r[10]        ),  { 0,    ~0U  }, { 0,    0x88 } },  /* INTEL_PD0R_KPC,  */
    { "PD0R_KP0",   V(pad0.r[0]         ),  { 0,    ~0U  }, { 0,    0x48 } },  /* INTEL_PD0R_KP0,  */
    { "PD0R_KPE",   V(pad0.r[11]        ),  { 0,    ~0U  }, { 0,    0x28 } },  /* INTEL_PD0R_KPE,  */
    { "PD0R_A_T",   V(pad0.r[12]        ), { 0,    ~0U  }, { 0,    0xA0 } },   /* INTEL_PD0R_A_T,  */
    { "PD0R_A_L",   V(pad0.r[13]        ), { 0,    ~0U  }, { 0,    0x60 } },   /* INTEL_PD0R_A_L,  */
    { "PD0R_A_R",   V(pad0.r[14]        ), { 0,    ~0U  }, { 0,    0xC0 } },   /* INTEL_PD0R_A_R,  */
    { "PD0R_D_E",   V(pad0.r[15]        ), { ~1,   ~0U  }, { 0,    1  } },     /* INTEL_PD0R_D_E,  */
    { "PD0R_D_ENE", V(pad0.r[15]        ), { ~3,   ~0U  }, { 0,    3  } },     /* INTEL_PD0R_D_ENE,*/
    { "PD0R_D_NE",  V(pad0.r[15]        ), { ~2,   ~0U  }, { 0,    2  } },     /* INTEL_PD0R_D_NE, */
    { "PD0R_D_NNE", V(pad0.r[15]        ), { ~6,   ~0U  }, { 0,    6  } },     /* INTEL_PD0R_D_NNE,*/
    { "PD0R_D_N",   V(pad0.r[15]        ), { ~4,   ~0U  }, { 0,    4  } },     /* INTEL_PD0R_D_N,  */
    { "PD0R_D_NNW", V(pad0.r[15]        ), { ~12,  ~0U  }, { 0,    12 } },     /* INTEL_PD0R_D_NNW,*/
    { "PD0R_D_NW",  V(pad0.r[15]        ), { ~8,   ~0U  }, { 0,    8  } },     /* INTEL_PD0R_D_NW, */
    { "PD0R_D_WNW", V(pad0.r[15]        ), { ~24,  ~0U  }, { 0,    24 } },     /* INTEL_PD0R_D_WNW,*/
    { "PD0R_D_W",   V(pad0.r[15]        ), { ~16,  ~0U  }, { 0,    16 } },     /* INTEL_PD0R_D_W,  */
    { "PD0R_D_WSW", V(pad0.r[15]        ), { ~48,  ~0U  }, { 0,    48 } },     /* INTEL_PD0R_D_WSW,*/
    { "PD0R_D_SW",  V(pad0.r[15]        ), { ~32,  ~0U  }, { 0,    32 } },     /* INTEL_PD0R_D_SW, */
    { "PD0R_D_SSW", V(pad0.r[15]        ), { ~96,  ~0U  }, { 0,    96 } },     /* INTEL_PD0R_D_SSW,*/
    { "PD0R_D_S",   V(pad0.r[15]        ), { ~64,  ~0U  }, { 0,    64 } },     /* INTEL_PD0R_D_S,  */
    { "PD0R_D_SSE", V(pad0.r[15]        ), { ~192, ~0U  }, { 0,    192} },     /* INTEL_PD0R_D_SSE,*/
    { "PD0R_D_SE",  V(pad0.r[15]        ), { ~128, ~0U  }, { 0,    128} },     /* INTEL_PD0R_D_SE, */
    { "PD0R_J_E",   V(pad0.r[16]        ), { 0,    0    }, { 0,    1   }},     /* INTEL_PD0R_J_E,  */
    { "PD0R_J_ENE", V(pad0.r[16]        ), { 0,    0    }, { 0,    3   }},     /* INTEL_PD0R_J_ENE,*/
    { "PD0R_J_NE",  V(pad0.r[16]        ), { 0,    0    }, { 0,    2   }},     /* INTEL_PD0R_J_NE, */
    { "PD0R_J_NNE", V(pad0.r[16]        ), { 0,    0    }, { 0,    6   }},     /* INTEL_PD0R_J_NNE,*/
    { "PD0R_J_N",   V(pad0.r[16]        ), { 0,    0    }, { 0,    4   }},     /* INTEL_PD0R_J_N,  */
    { "PD0R_J_NNW", V(pad0.r[16]        ), { 0,    0    }, { 0,    12  }},     /* INTEL_PD0R_J_NNW,*/
    { "PD0R_J_NW",  V(pad0.r[16]        ), { 0,    0    }, { 0,    8   }},     /* INTEL_PD0R_J_NW, */
    { "PD0R_J_WNW", V(pad0.r[16]        ), { 0,    0    }, { 0,    24  }},     /* INTEL_PD0R_J_WNW,*/
    { "PD0R_J_W",   V(pad0.r[16]        ), { 0,    0    }, { 0,    16  }},     /* INTEL_PD0R_J_W,  */
    { "PD0R_J_WSW", V(pad0.r[16]        ), { 0,    0    }, { 0,    48  }},     /* INTEL_PD0R_J_WSW,*/
    { "PD0R_J_SW",  V(pad0.r[16]        ), { 0,    0    }, { 0,    32  }},     /* INTEL_PD0R_J_SW, */
    { "PD0R_J_SSW", V(pad0.r[16]        ), { 0,    0    }, { 0,    96  }},     /* INTEL_PD0R_J_SSW,*/
    { "PD0R_J_S",   V(pad0.r[16]        ), { 0,    0    }, { 0,    64  }},     /* INTEL_PD0R_J_S,  */
    { "PD0R_J_SSE", V(pad0.r[16]        ), { 0,    0    }, { 0,    192 }},     /* INTEL_PD0R_J_SSE,*/
    { "PD0R_J_SE",  V(pad0.r[16]        ), { 0,    0    }, { 0,    128 }},     /* INTEL_PD0R_J_SE, */
    { "PD0R_J_ESE", V(pad0.r[16]        ), { 0,    0    }, { 0,    129 }}      /* INTEL_PD0R_J_ESE,*/
 };

 static int loc_default_mapping[ KBD_ALL_BUTTONS ] = {
   INTEL_PD0L_D_N     , /*  KBD_UP         */
   INTEL_PD0L_D_E     , /*  KBD_RIGHT      */
   INTEL_PD0L_D_S     , /*  KBD_DOWN       */
   INTEL_PD0L_D_W     , /*  KBD_LEFT       */
   INTEL_PD0L_A_T     , /*  KBD_TRIANGLE   */
   INTEL_PD0L_A_R     , /*  KBD_CIRCLE     */
   INTEL_PD0L_A_L     , /*  KBD_CROSS      */
   INTEL_PD0L_KP1     , /*  KBD_SQUARE     */
   -1                 , /*  KBD_SELECT     */
   -1                 , /*  KBD_START      */
   -1                 , /*  KBD_HOME       */
   -1                 , /*  KBD_HOLD       */
   INTEL_RESET        , /*  KBD_LTRIGGER   */
   INTEL_PD0L_KPE     , /*  KBD_RTRIGGER   */
   INTEL_PD0L_J_N     , /*  KBD_JOY_UP     */
   INTEL_PD0L_J_E     , /*  KBD_JOY_RIGHT  */
   INTEL_PD0L_J_S     , /*  KBD_JOY_DOWN   */
   INTEL_PD0L_J_W       /*  KBD_JOY_LEFT   */
  };

# define KBD_MAX_ENTRIES   93

  int kbd_layout[KBD_MAX_ENTRIES][2] = {
    /* Key            Ascii */
    { INTEL_RESET,       DANZEFF_ESC  },
    { INTEL_PD0L_KP1,    '1'  },
    { INTEL_PD0L_KP2,    '2'  },
    { INTEL_PD0L_KP3,    '3'  },
    { INTEL_PD0L_KP4,    '4'  },
    { INTEL_PD0L_KP5,    '5'  },
    { INTEL_PD0L_KP6,    '6'  },
    { INTEL_PD0L_KP7,    '7'  },
    { INTEL_PD0L_KP8,    '8'  },
    { INTEL_PD0L_KP9,    '9'  },
    { INTEL_PD0L_KPC,    'C'  },
    { INTEL_PD0L_KP0,    '0'  },
    { INTEL_PD0L_KPE,    'E'  },
    { INTEL_PD0L_A_T,    'T' },
    { INTEL_PD0L_A_L,    'L' },
    { INTEL_PD0L_A_R,    'R' },
    { INTEL_PD0L_D_E,    -1  },
    { INTEL_PD0L_D_ENE,  -1  },
    { INTEL_PD0L_D_NE,   -1  },
    { INTEL_PD0L_D_NNE,  -1  },
    { INTEL_PD0L_D_N,    -1  },
    { INTEL_PD0L_D_NNW,  -1  },
    { INTEL_PD0L_D_NW,   -1  },
    { INTEL_PD0L_D_WNW,  -1  },
    { INTEL_PD0L_D_W,    -1  },
    { INTEL_PD0L_D_WSW,  -1  },
    { INTEL_PD0L_D_SW,   -1  },
    { INTEL_PD0L_D_SSW,  -1  },
    { INTEL_PD0L_D_S,    -1  },
    { INTEL_PD0L_D_SSE,  -1  },
    { INTEL_PD0L_D_SE,   -1  },
    { INTEL_PD0L_J_E,    -1  },
    { INTEL_PD0L_J_ENE,  -1  },
    { INTEL_PD0L_J_NE,   -1  },
    { INTEL_PD0L_J_NNE,  -1  },
    { INTEL_PD0L_J_N,    -1  },
    { INTEL_PD0L_J_NNW,  -1  },
    { INTEL_PD0L_J_NW,   -1  },
    { INTEL_PD0L_J_WNW   -1  },
    { INTEL_PD0L_J_W,    -1  },
    { INTEL_PD0L_J_WSW,  -1  },
    { INTEL_PD0L_J_SW,   -1  },
    { INTEL_PD0L_J_SSW   -1  },
    { INTEL_PD0L_J_S,    -1  },
    { INTEL_PD0L_J_SSE,  -1  },
    { INTEL_PD0L_J_SE,   -1  },
    { INTEL_PD0L_J_ESE,  -1  },
    { INTEL_PD0R_KP1,    -1  },
    { INTEL_PD0R_KP2,    -1  },
    { INTEL_PD0R_KP3,    -1  },
    { INTEL_PD0R_KP4,    -1  },
    { INTEL_PD0R_KP5,    -1  },
    { INTEL_PD0R_KP6,    -1  },
    { INTEL_PD0R_KP7,    -1  },
    { INTEL_PD0R_KP8,    -1  },
    { INTEL_PD0R_KP9,    -1  },
    { INTEL_PD0R_KPC,    -1  },
    { INTEL_PD0R_KP0,    -1  },
    { INTEL_PD0R_KPE,    -1  },
    { INTEL_PD0R_A_T,    -1  },
    { INTEL_PD0R_A_L,    -1  },
    { INTEL_PD0R_A_R,    -1  },
    { INTEL_PD0R_D_E,    -1  },
    { INTEL_PD0R_D_ENE,  -1  },
    { INTEL_PD0R_D_NE,   -1  },
    { INTEL_PD0R_D_NNE,  -1  },
    { INTEL_PD0R_D_N,    -1  },
    { INTEL_PD0R_D_NNW,  -1  },
    { INTEL_PD0R_D_NW,   -1  },
    { INTEL_PD0R_D_WNW,  -1  },
    { INTEL_PD0R_D_W,    -1  },
    { INTEL_PD0R_D_WSW,  -1  },
    { INTEL_PD0R_D_SW,   -1  },
    { INTEL_PD0R_D_SSW,  -1  },
    { INTEL_PD0R_D_S,    -1  },
    { INTEL_PD0R_D_SSE,  -1  },
    { INTEL_PD0R_D_SE,   -1  },
    { INTEL_PD0R_J_E,    -1  },
    { INTEL_PD0R_J_ENE,  -1  },
    { INTEL_PD0R_J_NE,   -1  },
    { INTEL_PD0R_J_NNE,  -1  },
    { INTEL_PD0R_J_N,    -1  },
    { INTEL_PD0R_J_NNW,  -1  },
    { INTEL_PD0R_J_NW,   -1  },
    { INTEL_PD0R_J_WNW   -1  },
    { INTEL_PD0R_J_W,    -1  },
    { INTEL_PD0R_J_WSW,  -1  },
    { INTEL_PD0R_J_SW,   -1  },
    { INTEL_PD0R_J_SSW   -1  },
    { INTEL_PD0R_J_S,    -1  },
    { INTEL_PD0R_J_SSE,  -1  },
    { INTEL_PD0R_J_SE,   -1  },
    { INTEL_PD0R_J_ESE,  -1  } 
  };

        int psp_kbd_mapping[ KBD_ALL_BUTTONS ];

 static int danzeff_intel_key     = 0;
 static int danzeff_intel_pending = 0;
 static int danzeff_mode        = 0;


       char command_keys[ 128 ];
 static int command_mode        = 0;
 static int command_index       = 0;
 static int command_size        = 0;
 static int command_intel_pending = 0;
 static int command_intel_key     = 0;

  extern uint_32 event_count;

void
psp_kbd_run_command(char *Command)
{
  strncpy(command_keys, Command, 128);
  command_size  = strlen(Command);
  command_index = 0;

  command_intel_key     = 0;
  command_intel_pending = 0;
  command_mode        = 1;
}

void
psp_kbd_run_command_reset()
{
  command_keys[0] = DANZEFF_ESC;
  command_keys[1] = 0;
  command_size  = 1;
  command_index = 0;

  command_intel_key     = 0;
  command_intel_pending = 0;
  command_mode        = 1;
}

int
intel_key_event(int intel_id, int button_pressed)
{
  uint_32 *pvalue;

  if (INTEL.psp_active_joystick) {
    if ((intel_id >= INTEL_PD0L_KP1) && (intel_id <= INTEL_PD0L_J_ESE)) {
      intel_id += INTEL_PD0R_KP1 - INTEL_PD0L_KP1;
    }
  }

  if ((intel_id >= 0) && (intel_id < INTEL_MAX_KEY)) {
    pvalue = psp_intel_key_to_name[intel_id].pvalue;
    if (pvalue != NULL) {
      *pvalue &= psp_intel_key_to_name[intel_id].mask_and[button_pressed];
      *pvalue |= psp_intel_key_to_name[intel_id].mask_or[button_pressed];
       event_count++;
    }
  }
  return 0;
}
int 
intel_kbd_reset()
{
  uint_32 *pvalue;
  int      intel_id;

  for (intel_id = INTEL_PD0L_KP1; intel_id < INTEL_MAX_KEY; intel_id++) {
    pvalue = psp_intel_key_to_name[intel_id].pvalue;
    if (pvalue != NULL) {
      *pvalue &= psp_intel_key_to_name[intel_id].mask_and[0];
      *pvalue |= psp_intel_key_to_name[intel_id].mask_or[0];
    }
  }
  return 0;
}

int
intel_get_key_from_ascii(int key_ascii)
{
  int index;
  for (index = 0; index < KBD_MAX_ENTRIES; index++) {
   if (kbd_layout[index][1] == key_ascii) return kbd_layout[index][0];
  }
  return -1;
}

int
psp_kbd_reset_mapping(void)
{
  memcpy(psp_kbd_mapping, loc_default_mapping, sizeof(loc_default_mapping));
  return 0;
}

int
psp_kbd_load_mapping(char *kbd_filename)
{
  char     Buffer[512];
  FILE    *KbdFile;
  char    *Scan;
  int      tmp_mapping[KBD_ALL_BUTTONS];
  int      intel_key_id = 0;
  int      kbd_id = 0;
  int      error = 0;

  memcpy(tmp_mapping, loc_default_mapping, sizeof(loc_default_mapping));

  KbdFile = fopen(kbd_filename, "r");
  error   = 1;

  if (KbdFile != (FILE*)0) {

    while (fgets(Buffer,512,KbdFile) != (char *)0) {

      Scan = strchr(Buffer,'\n');
      if (Scan) *Scan = '\0';
      /* For this #@$% of windows ! */
      Scan = strchr(Buffer,'\r');
      if (Scan) *Scan = '\0';
      if (Buffer[0] == '#') continue;

      Scan = strchr(Buffer,'=');
      if (! Scan) continue;

      *Scan = '\0';
      intel_key_id = atoi(Scan + 1);

      for (kbd_id = 0; kbd_id < KBD_ALL_BUTTONS; kbd_id++) {
        if (!strcasecmp(Buffer,loc_button_name[kbd_id])) {
          tmp_mapping[kbd_id] = intel_key_id;
          break;
        }
      }
    }

    error = 0;
    fclose(KbdFile);
  }

  memcpy(psp_kbd_mapping, tmp_mapping, sizeof(psp_kbd_mapping));

  return error;
}

int
psp_kbd_save_mapping(char *kbd_filename)
{
  FILE    *KbdFile;
  int      kbd_id = 0;
  int      error = 0;

  KbdFile = fopen(kbd_filename, "w");
  error   = 1;

  if (KbdFile != (FILE*)0) {

    for (kbd_id = 0; kbd_id < KBD_ALL_BUTTONS; kbd_id++)
    {
      fprintf(KbdFile, "%s=%d\n", loc_button_name[kbd_id], psp_kbd_mapping[kbd_id]);
    }
    error = 0;
    fclose(KbdFile);
  }

  return error;
}

int
psp_kbd_enter_command()
{
  SceCtrlData  c;

  unsigned int command_key = 0;
  int          intel_key     = 0;
  int          key_event   = 0;

  sceCtrlPeekBufferPositive(&c, 1);

  if (command_intel_pending) 
  {
    if ((c.TimeStamp - loc_last_event_time) > KBD_MIN_COMMAND_TIME) {
      loc_last_event_time = c.TimeStamp;
      command_intel_pending = 0;
      intel_key_event(command_intel_key, 0);
    }

    return 0;
  }

  if ((c.TimeStamp - loc_last_event_time) > KBD_MIN_COMMAND_TIME) {
    loc_last_event_time = c.TimeStamp;

    if (command_index >= command_size) {

      command_mode  = 0;
      command_index = 0;
      command_size  = 0;

      command_intel_pending = 0;
      command_intel_key     = 0;

      return 0;
    }
  
    command_key = command_keys[command_index++];
    intel_key = intel_get_key_from_ascii(command_key);

    if (intel_key != -1) {
      command_intel_key     = intel_key;
      command_intel_pending = 1;
      intel_key_event(command_intel_key, 1);
    }

    return 1;
  }

  return 0;
}

int 
psp_kbd_is_danzeff_mode()
{
  return danzeff_mode;
}

int
psp_kbd_enter_danzeff()
{
  unsigned int danzeff_key = 0;
  int          intel_key     = 0;
  int          key_event   = 0;
  SceCtrlData  c;

  if (! danzeff_mode) {
    psp_init_keyboard();
    danzeff_mode = 1;
  }

  sceCtrlPeekBufferPositive(&c, 1);

  if (danzeff_intel_pending) 
  {
    if ((c.TimeStamp - loc_last_event_time) > KBD_MIN_PENDING_TIME) {
      loc_last_event_time = c.TimeStamp;
      danzeff_intel_pending = 0;
      intel_key_event(danzeff_intel_key, 0);
    }

    return 0;
  }

  if ((c.TimeStamp - loc_last_event_time) > KBD_MIN_DANZEFF_TIME) {
    loc_last_event_time = c.TimeStamp;
  
    sceCtrlPeekBufferPositive(&c, 1);
    danzeff_key = danzeff_readInput(c);
  }

  if (danzeff_key > DANZEFF_START) {
    intel_key = intel_get_key_from_ascii(danzeff_key);

    if (intel_key != -1) {
      danzeff_intel_key     = intel_key;
      danzeff_intel_pending = 1;
      intel_key_event(danzeff_intel_key, 1);
    }

    return 1;

  } else if (danzeff_key == DANZEFF_START) {
    danzeff_mode       = 0;
    danzeff_intel_pending = 0;
    danzeff_intel_key     = 0;

    psp_kbd_wait_no_button();

  } else if (danzeff_key == DANZEFF_SELECT) {
    danzeff_mode       = 0;
    danzeff_intel_pending = 0;
    danzeff_intel_key     = 0;
    psp_main_menu();
    psp_init_keyboard();

    psp_kbd_wait_no_button();
  }

  return 0;
}

int
intel_decode_key(int psp_b, int button_pressed)
{
  int wake = 0;
  int reverse_analog = INTEL.psp_reverse_analog;

  if (reverse_analog) {
    if ((psp_b >= KBD_JOY_UP  ) &&
        (psp_b <= KBD_JOY_LEFT)) {
      psp_b = psp_b - KBD_JOY_UP + KBD_UP;
    } else
    if ((psp_b >= KBD_UP  ) &&
        (psp_b <= KBD_LEFT)) {
      psp_b = psp_b - KBD_UP + KBD_JOY_UP;
    }
  }

  if (psp_b == KBD_START) {
     if (button_pressed) psp_kbd_enter_danzeff();
  } else
  if (psp_b == KBD_SELECT) {
    if (button_pressed) {
      psp_main_menu();
      psp_init_keyboard();
    }
  } else {
 
    if (psp_kbd_mapping[psp_b] != -1) {
      wake = 1;
      intel_key_event(psp_kbd_mapping[psp_b], button_pressed);
    }
  }
  return 0;
}

# define ANALOG_THRESHOLD 60

void 
kbd_get_analog_direction(int Analog_x, int Analog_y, int *x, int *y)
{
  int DeltaX = 255;
  int DeltaY = 255;
  int DirX   = 0;
  int DirY   = 0;

  *x = 0;
  *y = 0;

  if (Analog_x <=        ANALOG_THRESHOLD)  { DeltaX = Analog_x; DirX = -1; }
  else 
  if (Analog_x >= (255 - ANALOG_THRESHOLD)) { DeltaX = 255 - Analog_x; DirX = 1; }

  if (Analog_y <=        ANALOG_THRESHOLD)  { DeltaY = Analog_y; DirY = -1; }
  else 
  if (Analog_y >= (255 - ANALOG_THRESHOLD)) { DeltaY = 255 - Analog_y; DirY = 1; }

  *x = DirX;
  *y = DirY;
}

static int 
kbd_reset_button_status(void)
{
  int b = 0;
  /* Reset Button status */
  for (b = 0; b < KBD_MAX_BUTTONS; b++) {
    loc_button_press[b]   = 0;
    loc_button_release[b] = 0;
  }
  psp_init_keyboard();
  return 0;
}

int
kbd_scan_keyboard(void)
{
  SceCtrlData c;
  long        delta_stamp;
  int         event;
  int         b;

  int new_Lx;
  int new_Ly;
  int old_Lx;
  int old_Ly;

  event = 0;
  sceCtrlPeekBufferPositive( &c, 1 );

  if ((c.Buttons & (PSP_CTRL_LTRIGGER|PSP_CTRL_RTRIGGER|PSP_CTRL_START)) ==
      (PSP_CTRL_LTRIGGER|PSP_CTRL_RTRIGGER|PSP_CTRL_START)) {
    /* Exit ! */
    psp_sdl_exit(0);
  }

  delta_stamp = c.TimeStamp - first_time_stamp;
  if ((delta_stamp < 0) || (delta_stamp > KBD_MIN_BATTCHECK_TIME)) {
    first_time_stamp = c.TimeStamp;
    if (psp_is_low_battery()) {
      psp_main_menu();
      psp_init_keyboard();
      return 0;
    }
  }

  /* Check Analog Device */
  kbd_get_analog_direction(loc_button_data.Lx,loc_button_data.Ly,&old_Lx,&old_Ly);
  kbd_get_analog_direction( c.Lx, c.Ly, &new_Lx, &new_Ly);

  /* Analog device has moved */
  if (new_Lx > 0) {
    if (old_Lx  > 0) intel_decode_key(KBD_JOY_LEFT , 0);
    intel_decode_key(KBD_JOY_RIGHT, 1);
    loc_analog_x_released = 0;

  } else 
  if (new_Lx < 0) {
    if (old_Lx  < 0) intel_decode_key(KBD_JOY_RIGHT, 0);
    intel_decode_key(KBD_JOY_LEFT , 1);
    loc_analog_x_released = 0;

  } else {
    if (old_Lx  > 0) intel_decode_key(KBD_JOY_LEFT , 0);
    else
    if (old_Lx  < 0) intel_decode_key(KBD_JOY_RIGHT, 0);
    else {
      intel_decode_key(KBD_JOY_LEFT  , 0);
      intel_decode_key(KBD_JOY_RIGHT , 0);
      loc_analog_x_released = 1;
    }
  }

  if (new_Ly < 0) {
    if (old_Ly  > 0) intel_decode_key(KBD_JOY_DOWN , 0);
    intel_decode_key(KBD_JOY_UP   , 1);
    loc_analog_y_released = 0;

  } else 
  if (new_Ly > 0) {
    if (old_Ly  < 0) intel_decode_key(KBD_JOY_UP   , 0);
    intel_decode_key(KBD_JOY_DOWN , 1);
    loc_analog_y_released = 0;

  } else {
    if (old_Ly  > 0) intel_decode_key(KBD_JOY_DOWN , 0);
    else
    if (old_Ly  < 0) intel_decode_key(KBD_JOY_UP   , 0);
    else {
      intel_decode_key(KBD_JOY_DOWN , 0);
      intel_decode_key(KBD_JOY_UP   , 0);
      loc_analog_y_released = 1;
    }
  }

  for (b = 0; b < KBD_MAX_BUTTONS; b++) 
  {
    if (c.Buttons & loc_button_mask[b]) {
      if (!(loc_button_data.Buttons & loc_button_mask[b])) {
        loc_button_press[b] = 1;
        event = 1;
      }
    } else {
      if (loc_button_data.Buttons & loc_button_mask[b]) {
        loc_button_release[b] = 1;
        event = 1;
      }
    }
  }
  memcpy(&loc_button_data,&c,sizeof(SceCtrlData));

  return event;
}

void
kbd_wait_start(void)
{
  while (1)
  {
    SceCtrlData c;
    sceCtrlReadBufferPositive(&c, 1);
    if (c.Buttons & PSP_CTRL_START) break;
  }
}

void
psp_init_keyboard(void)
{
  intel_kbd_reset();
}

void
psp_kbd_wait_no_button(void)
{
  SceCtrlData c;

  do {
   sceCtrlPeekBufferPositive(&c, 1);
  } while (c.Buttons != 0);
} 

int
psp_update_keys(void)
{
  int         b;

  static char first_time = 1;
  static int release_pending = 0;

  if (first_time) {

    memcpy(psp_kbd_mapping, loc_default_mapping, sizeof(loc_default_mapping));

    SceCtrlData c;
    sceCtrlPeekBufferPositive(&c, 1);

    if (first_time_stamp == -1) first_time_stamp = c.TimeStamp;
    if ((! c.Buttons) && ((c.TimeStamp - first_time_stamp) < KBD_MIN_START_TIME)) return 0;

    first_time      = 0;
    release_pending = 0;

    for (b = 0; b < KBD_MAX_BUTTONS; b++) {
      loc_button_release[b] = 0;
      loc_button_press[b] = 0;
    }
    sceCtrlPeekBufferPositive(&loc_button_data, 1);

    psp_main_menu();
    psp_init_keyboard();

    return 0;
  }

  if (command_mode) {
    return psp_kbd_enter_command();
  }

  if (danzeff_mode) {
    return psp_kbd_enter_danzeff();
  }

  if (release_pending)
  {
    release_pending = 0;
    for (b = 0; b < KBD_MAX_BUTTONS; b++) {
      if (loc_button_release[b]) {
        loc_button_release[b] = 0;
        loc_button_press[b] = 0;
        intel_decode_key(b, 0);
      }
    }
  }

  kbd_scan_keyboard();

  /* check release event */
  for (b = 0; b < KBD_MAX_BUTTONS; b++) {
    if (loc_button_release[b]) {
      release_pending = 1;
      break;
    } 
  }
  /* check press event */
  for (b = 0; b < KBD_MAX_BUTTONS; b++) {
    if (loc_button_press[b]) {
      loc_button_press[b] = 0;
      release_pending     = 0;
      intel_decode_key(b, 1);
    }
  }

  return 0;
}

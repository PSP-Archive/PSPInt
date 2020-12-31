/*
 * ============================================================================
 *  Title:    Configuration Manager
 *  Author:   J. Zbiciak
 *  $Id: cfg.c,v 1.11 2001/11/02 02:00:03 im14u2c Exp $
 * ============================================================================
 *  This module manages the machine configuration.  It does commandline
 *  parsing and processes the configuration elements that were read in 
 *  via the config-file parser.
 *
 *  CFG owns the entire machine -- it is encapsulated in a cfg_t.
 * ============================================================================
 *  CFG_INIT     -- Parse command line and get started
 *  CFG_FILE     -- Parse a config file and extend the state of the machine.
 * ============================================================================
 */

# if 1 //LUDO: 
#include <stdlib.h>
#include <string.h>
# endif

#include "../config.h"
#include "macros.h"
#include "periph/periph.h"
#include "cp1600/cp1600.h"
#include "cp1600/emu_link.h"
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
#include "ivoice/ivoice.h"
#include "speed/speed.h"
#include "debug/debug_.h"
#include "event/event.h"
#include "file/file.h"
#include "joy/joy.h"
#include "serializer/serializer.h"
#include "cfg.h"

#include <errno.h>


void cfg_default(event_t *event);

/* ------------------------------------------------------------------------ */
/*  This, my dear friends, is the Intellivision.                            */
/* ------------------------------------------------------------------------ */
cfg_t top_intv;


#define V(v) ((v_uint_32*)&top_intv.v)

/* ------------------------------------------------------------------------ */
/*  jzIntv internal event name table.  Keyboard and joystick inputs may be  */
/*  bound to any of these event names.  This table also ties the event      */
/*  names to the actual bits that the event fiddles with.                   */
/*                                                                          */
/*  Notes on mnemonics:                                                     */
/*      PD0L        Left controller pad on base unit.                       */
/*      PD0R        Right controller pad on base unit.                      */
/*      PD1L        Left controller pad on ECS unit.                        */
/*      PD1R        Right controller pad on ECS unit.                       */
/*      PDxx_KP     Left Key Pad                                            */
/*      PDxx_A      Left Action Button  ([T]op, [L]eft, [R]ight)            */
/*      PDxx_D      Right Disc                                              */
/*      N, NE, etc  Compass directions.                                     */
/*                                                                          */
/*  The bit patterns at the right are AND and OR masks.  The first two      */
/*  are the AND masks for release/press.  The second two are OR masks       */
/*  for release/press.                                                      */
/* ------------------------------------------------------------------------ */
# if 0 //LUDO:
cfg_evt_t  cfg_event[] =
{
    /* -------------------------------------------------------------------- */
    /*  Miscellaneous.                                                      */
    /* -------------------------------------------------------------------- */
    { "QUIT",       V(do_exit           ),  { ~0U, ~0U },   { ~0U, ~0U } },
    { "RESET",      V(do_reset          ),  { 0,   ~0U },   { 0,   ~0U } },
    { "PAUSE",      V(do_pause          ),  { ~0U, 0   },   { 0,   5   } },
    { "MOVIE",      V(gfx.scrshot       ),  { ~0U, ~0U },   { GFX_MVTOG, 0} },
    { "SHOT",       V(gfx.scrshot       ),  { ~0U, ~0U },   { GFX_SHOT,  0} },
    { "HIDE",       V(gfx.hidden        ),  { 0,   1   },   { 0,   1   } },
    { "BREAK",      V(debug.step_count  ),  { ~0U, 0   },   { 0,   0   } },
    { "KBD0",       V(event.change_kbd  ),  { ~0U, 0   },   { 0,   1   } },
    { "KBD1",       V(event.change_kbd  ),  { ~0U, 0   },   { 0,   2   } },
    { "KBD2",       V(event.change_kbd  ),  { ~0U, 0   },   { 0,   3   } },
    { "KBD3",       V(event.change_kbd  ),  { ~0U, 0   },   { 0,   4   } },
    { "KBDn",       V(event.change_kbd  ),  { ~0U, 0   },   { 0,   5   } },
    { "KBDp",       V(event.change_kbd  ),  { ~0U, 0   },   { 0,   6   } },
    { "NA",         NULL,                   { 0,   0   },   { 0,   0   } },

    /* -------------------------------------------------------------------- */
    /*  PAD0: Left-hand controller keypad                                   */
    /* -------------------------------------------------------------------- */
    { "PD0L_KP1",   V(pad0.l[1]         ),  { 0,    ~0U  }, { 0,    0x81 } },
    { "PD0L_KP2",   V(pad0.l[2]         ),  { 0,    ~0U  }, { 0,    0x41 } },
    { "PD0L_KP3",   V(pad0.l[3]         ),  { 0,    ~0U  }, { 0,    0x21 } },
    { "PD0L_KP4",   V(pad0.l[4]         ),  { 0,    ~0U  }, { 0,    0x82 } },
    { "PD0L_KP5",   V(pad0.l[5]         ),  { 0,    ~0U  }, { 0,    0x42 } },
    { "PD0L_KP6",   V(pad0.l[6]         ),  { 0,    ~0U  }, { 0,    0x22 } },
    { "PD0L_KP7",   V(pad0.l[7]         ),  { 0,    ~0U  }, { 0,    0x84 } },
    { "PD0L_KP8",   V(pad0.l[8]         ),  { 0,    ~0U  }, { 0,    0x44 } },
    { "PD0L_KP9",   V(pad0.l[9]         ),  { 0,    ~0U  }, { 0,    0x24 } },
    { "PD0L_KPC",   V(pad0.l[10]        ),  { 0,    ~0U  }, { 0,    0x88 } },
    { "PD0L_KP0",   V(pad0.l[0]         ),  { 0,    ~0U  }, { 0,    0x48 } },
    { "PD0L_KPE",   V(pad0.l[11]        ),  { 0,    ~0U  }, { 0,    0x28 } },

    /* -------------------------------------------------------------------- */
    /*  PAD0: Right-hand controller keypad                                  */
    /* -------------------------------------------------------------------- */
    { "PD0R_KP1",   V(pad0.r[1]         ),  { 0,    ~0U  }, { 0,    0x81 } },
    { "PD0R_KP2",   V(pad0.r[2]         ),  { 0,    ~0U  }, { 0,    0x41 } },
    { "PD0R_KP3",   V(pad0.r[3]         ),  { 0,    ~0U  }, { 0,    0x21 } },
    { "PD0R_KP4",   V(pad0.r[4]         ),  { 0,    ~0U  }, { 0,    0x82 } },
    { "PD0R_KP5",   V(pad0.r[5]         ),  { 0,    ~0U  }, { 0,    0x42 } },
    { "PD0R_KP6",   V(pad0.r[6]         ),  { 0,    ~0U  }, { 0,    0x22 } },
    { "PD0R_KP7",   V(pad0.r[7]         ),  { 0,    ~0U  }, { 0,    0x84 } },
    { "PD0R_KP8",   V(pad0.r[8]         ),  { 0,    ~0U  }, { 0,    0x44 } },
    { "PD0R_KP9",   V(pad0.r[9]         ),  { 0,    ~0U  }, { 0,    0x24 } },
    { "PD0R_KPC",   V(pad0.r[10]        ),  { 0,    ~0U  }, { 0,    0x88 } },
    { "PD0R_KP0",   V(pad0.r[0]         ),  { 0,    ~0U  }, { 0,    0x48 } },
    { "PD0R_KPE",   V(pad0.r[11]        ),  { 0,    ~0U  }, { 0,    0x28 } },

    /* -------------------------------------------------------------------- */
    /*  PAD0: Action buttons.                                               */
    /* -------------------------------------------------------------------- */
    { "PD0L_A_T",   V(pad0.l[12]        ), { 0,    ~0U  }, { 0,    0xA0 } },
    { "PD0L_A_L",   V(pad0.l[13]        ), { 0,    ~0U  }, { 0,    0x60 } },
    { "PD0L_A_R",   V(pad0.l[14]        ), { 0,    ~0U  }, { 0,    0xC0 } },

    { "PD0R_A_T",   V(pad0.r[12]        ), { 0,    ~0U  }, { 0,    0xA0 } },
    { "PD0R_A_L",   V(pad0.r[13]        ), { 0,    ~0U  }, { 0,    0x60 } },
    { "PD0R_A_R",   V(pad0.r[14]        ), { 0,    ~0U  }, { 0,    0xC0 } },

    /* -------------------------------------------------------------------- */
    /*  PAD0: The Controller DISC via Keyboard etc.                         */
    /* -------------------------------------------------------------------- */
    { "PD0L_D_E",   V(pad0.l[15]        ), { ~1,   ~0U  }, { 0,    1  } },
    { "PD0L_D_ENE", V(pad0.l[15]        ), { ~3,   ~0U  }, { 0,    3  } },
    { "PD0L_D_NE",  V(pad0.l[15]        ), { ~2,   ~0U  }, { 0,    2  } },
    { "PD0L_D_NNE", V(pad0.l[15]        ), { ~6,   ~0U  }, { 0,    6  } },
    { "PD0L_D_N",   V(pad0.l[15]        ), { ~4,   ~0U  }, { 0,    4  } },
    { "PD0L_D_NNW", V(pad0.l[15]        ), { ~12,  ~0U  }, { 0,    12 } },
    { "PD0L_D_NW",  V(pad0.l[15]        ), { ~8,   ~0U  }, { 0,    8  } },
    { "PD0L_D_WNW", V(pad0.l[15]        ), { ~24,  ~0U  }, { 0,    24 } },
    { "PD0L_D_W",   V(pad0.l[15]        ), { ~16,  ~0U  }, { 0,    16 } },
    { "PD0L_D_WSW", V(pad0.l[15]        ), { ~48,  ~0U  }, { 0,    48 } },
    { "PD0L_D_SW",  V(pad0.l[15]        ), { ~32,  ~0U  }, { 0,    32 } },
    { "PD0L_D_SSW", V(pad0.l[15]        ), { ~96,  ~0U  }, { 0,    96 } },
    { "PD0L_D_S",   V(pad0.l[15]        ), { ~64,  ~0U  }, { 0,    64 } },
    { "PD0L_D_SSE", V(pad0.l[15]        ), { ~192, ~0U  }, { 0,    192} },
    { "PD0L_D_SE",  V(pad0.l[15]        ), { ~128, ~0U  }, { 0,    128} },
                                                                      
    { "PD0R_D_E",   V(pad0.r[15]        ), { ~1,   ~0U  }, { 0,    1  } },
    { "PD0R_D_ENE", V(pad0.r[15]        ), { ~3,   ~0U  }, { 0,    3  } },
    { "PD0R_D_NE",  V(pad0.r[15]        ), { ~2,   ~0U  }, { 0,    2  } },
    { "PD0R_D_NNE", V(pad0.r[15]        ), { ~6,   ~0U  }, { 0,    6  } },
    { "PD0R_D_N",   V(pad0.r[15]        ), { ~4,   ~0U  }, { 0,    4  } },
    { "PD0R_D_NNW", V(pad0.r[15]        ), { ~12,  ~0U  }, { 0,    12 } },
    { "PD0R_D_NW",  V(pad0.r[15]        ), { ~8,   ~0U  }, { 0,    8  } },
    { "PD0R_D_WNW", V(pad0.r[15]        ), { ~24,  ~0U  }, { 0,    24 } },
    { "PD0R_D_W",   V(pad0.r[15]        ), { ~16,  ~0U  }, { 0,    16 } },
    { "PD0R_D_WSW", V(pad0.r[15]        ), { ~48,  ~0U  }, { 0,    48 } },
    { "PD0R_D_SW",  V(pad0.r[15]        ), { ~32,  ~0U  }, { 0,    32 } },
    { "PD0R_D_SSW", V(pad0.r[15]        ), { ~96,  ~0U  }, { 0,    96 } },
    { "PD0R_D_S",   V(pad0.r[15]        ), { ~64,  ~0U  }, { 0,    64 } },
    { "PD0R_D_SSE", V(pad0.r[15]        ), { ~192, ~0U  }, { 0,    192} },
    { "PD0R_D_SE",  V(pad0.r[15]        ), { ~128, ~0U  }, { 0,    128} },

    /* -------------------------------------------------------------------- */
    /*  PAD0: The Controller DISC via Joystick                              */
    /* -------------------------------------------------------------------- */
    { "PD0L_J_E",   V(pad0.l[16]        ), { 0,    0    }, { 0,    1   }},
    { "PD0L_J_ENE", V(pad0.l[16]        ), { 0,    0    }, { 0,    3   }},
    { "PD0L_J_NE",  V(pad0.l[16]        ), { 0,    0    }, { 0,    2   }},
    { "PD0L_J_NNE", V(pad0.l[16]        ), { 0,    0    }, { 0,    6   }},
    { "PD0L_J_N",   V(pad0.l[16]        ), { 0,    0    }, { 0,    4   }},
    { "PD0L_J_NNW", V(pad0.l[16]        ), { 0,    0    }, { 0,    12  }},
    { "PD0L_J_NW",  V(pad0.l[16]        ), { 0,    0    }, { 0,    8   }},
    { "PD0L_J_WNW", V(pad0.l[16]        ), { 0,    0    }, { 0,    24  }},
    { "PD0L_J_W",   V(pad0.l[16]        ), { 0,    0    }, { 0,    16  }},
    { "PD0L_J_WSW", V(pad0.l[16]        ), { 0,    0    }, { 0,    48  }},
    { "PD0L_J_SW",  V(pad0.l[16]        ), { 0,    0    }, { 0,    32  }},
    { "PD0L_J_SSW", V(pad0.l[16]        ), { 0,    0    }, { 0,    96  }},
    { "PD0L_J_S",   V(pad0.l[16]        ), { 0,    0    }, { 0,    64  }},
    { "PD0L_J_SSE", V(pad0.l[16]        ), { 0,    0    }, { 0,    192 }},
    { "PD0L_J_SE",  V(pad0.l[16]        ), { 0,    0    }, { 0,    128 }},
    { "PD0L_J_ESE", V(pad0.l[16]        ), { 0,    0    }, { 0,    129 }},
                                                                       
    { "PD0R_J_E",   V(pad0.r[16]        ), { 0,    0    }, { 0,    1   }},
    { "PD0R_J_ENE", V(pad0.r[16]        ), { 0,    0    }, { 0,    3   }},
    { "PD0R_J_NE",  V(pad0.r[16]        ), { 0,    0    }, { 0,    2   }},
    { "PD0R_J_NNE", V(pad0.r[16]        ), { 0,    0    }, { 0,    6   }},
    { "PD0R_J_N",   V(pad0.r[16]        ), { 0,    0    }, { 0,    4   }},
    { "PD0R_J_NNW", V(pad0.r[16]        ), { 0,    0    }, { 0,    12  }},
    { "PD0R_J_NW",  V(pad0.r[16]        ), { 0,    0    }, { 0,    8   }},
    { "PD0R_J_WNW", V(pad0.r[16]        ), { 0,    0    }, { 0,    24  }},
    { "PD0R_J_W",   V(pad0.r[16]        ), { 0,    0    }, { 0,    16  }},
    { "PD0R_J_WSW", V(pad0.r[16]        ), { 0,    0    }, { 0,    48  }},
    { "PD0R_J_SW",  V(pad0.r[16]        ), { 0,    0    }, { 0,    32  }},
    { "PD0R_J_SSW", V(pad0.r[16]        ), { 0,    0    }, { 0,    96  }},
    { "PD0R_J_S",   V(pad0.r[16]        ), { 0,    0    }, { 0,    64  }},
    { "PD0R_J_SSE", V(pad0.r[16]        ), { 0,    0    }, { 0,    192 }},
    { "PD0R_J_SE",  V(pad0.r[16]        ), { 0,    0    }, { 0,    128 }},
    { "PD0R_J_ESE", V(pad0.r[16]        ), { 0,    0    }, { 0,    129 }},


    /* -------------------------------------------------------------------- */
    /*  PAD1: Left-hand controller keypad                                   */
    /* -------------------------------------------------------------------- */
    { "PD1L_KP1",   V(pad1.l[1]         ),  { 0,    ~0U  }, { 0,    0x81 } },
    { "PD1L_KP2",   V(pad1.l[2]         ),  { 0,    ~0U  }, { 0,    0x41 } },
    { "PD1L_KP3",   V(pad1.l[3]         ),  { 0,    ~0U  }, { 0,    0x21 } },
    { "PD1L_KP4",   V(pad1.l[4]         ),  { 0,    ~0U  }, { 0,    0x82 } },
    { "PD1L_KP5",   V(pad1.l[5]         ),  { 0,    ~0U  }, { 0,    0x42 } },
    { "PD1L_KP6",   V(pad1.l[6]         ),  { 0,    ~0U  }, { 0,    0x22 } },
    { "PD1L_KP7",   V(pad1.l[7]         ),  { 0,    ~0U  }, { 0,    0x84 } },
    { "PD1L_KP8",   V(pad1.l[8]         ),  { 0,    ~0U  }, { 0,    0x44 } },
    { "PD1L_KP9",   V(pad1.l[9]         ),  { 0,    ~0U  }, { 0,    0x24 } },
    { "PD1L_KPC",   V(pad1.l[10]        ),  { 0,    ~0U  }, { 0,    0x88 } },
    { "PD1L_KP0",   V(pad1.l[0]         ),  { 0,    ~0U  }, { 0,    0x48 } },
    { "PD1L_KPE",   V(pad1.l[11]        ),  { 0,    ~0U  }, { 0,    0x28 } },

    /* -------------------------------------------------------------------- */
    /*  PAD1: Right-hand controller keypad                                  */
    /* -------------------------------------------------------------------- */
    { "PD1R_KP1",   V(pad1.r[1]         ),  { 0,    ~0U  }, { 0,    0x81 } },
    { "PD1R_KP2",   V(pad1.r[2]         ),  { 0,    ~0U  }, { 0,    0x41 } },
    { "PD1R_KP3",   V(pad1.r[3]         ),  { 0,    ~0U  }, { 0,    0x21 } },
    { "PD1R_KP4",   V(pad1.r[4]         ),  { 0,    ~0U  }, { 0,    0x82 } },
    { "PD1R_KP5",   V(pad1.r[5]         ),  { 0,    ~0U  }, { 0,    0x42 } },
    { "PD1R_KP6",   V(pad1.r[6]         ),  { 0,    ~0U  }, { 0,    0x22 } },
    { "PD1R_KP7",   V(pad1.r[7]         ),  { 0,    ~0U  }, { 0,    0x84 } },
    { "PD1R_KP8",   V(pad1.r[8]         ),  { 0,    ~0U  }, { 0,    0x44 } },
    { "PD1R_KP9",   V(pad1.r[9]         ),  { 0,    ~0U  }, { 0,    0x24 } },
    { "PD1R_KPC",   V(pad1.r[10]        ),  { 0,    ~0U  }, { 0,    0x88 } },
    { "PD1R_KP0",   V(pad1.r[0]         ),  { 0,    ~0U  }, { 0,    0x48 } },
    { "PD1R_KPE",   V(pad1.r[11]        ),  { 0,    ~0U  }, { 0,    0x28 } },

    /* -------------------------------------------------------------------- */
    /*  PAD1: Action buttons.                                               */
    /* -------------------------------------------------------------------- */
    { "PD1L_A_T",   V(pad1.l[12]        ), { 0,    ~0U  }, { 0,    0xA0 } },
    { "PD1L_A_L",   V(pad1.l[13]        ), { 0,    ~0U  }, { 0,    0x60 } },
    { "PD1L_A_R",   V(pad1.l[14]        ), { 0,    ~0U  }, { 0,    0xC0 } },

    { "PD1R_A_T",   V(pad1.r[12]        ), { 0,    ~0U  }, { 0,    0xA0 } },
    { "PD1R_A_L",   V(pad1.r[13]        ), { 0,    ~0U  }, { 0,    0x60 } },
    { "PD1R_A_R",   V(pad1.r[14]        ), { 0,    ~0U  }, { 0,    0xC0 } },

    /* -------------------------------------------------------------------- */
    /*  PAD1: The Controller DISC.                                          */
    /* -------------------------------------------------------------------- */
    { "PD1L_D_E",   V(pad1.l[15]        ), { ~1,   ~0U  }, { 0,    1  } },
    { "PD1L_D_NE",  V(pad1.l[15]        ), { ~2,   ~0U  }, { 0,    2  } },
    { "PD1L_D_N",   V(pad1.l[15]        ), { ~4,   ~0U  }, { 0,    4  } },
    { "PD1L_D_NW",  V(pad1.l[15]        ), { ~8,   ~0U  }, { 0,    8  } },
    { "PD1L_D_W",   V(pad1.l[15]        ), { ~16,  ~0U  }, { 0,    16 } },
    { "PD1L_D_SW",  V(pad1.l[15]        ), { ~32,  ~0U  }, { 0,    32 } },
    { "PD1L_D_S",   V(pad1.l[15]        ), { ~64,  ~0U  }, { 0,    64 } },
    { "PD1L_D_SE",  V(pad1.l[15]        ), { ~128, ~0U  }, { 0,    128} },

    { "PD1R_D_E",   V(pad1.r[15]        ), { ~1,   ~0U  }, { 0,    1  } },
    { "PD1R_D_NE",  V(pad1.r[15]        ), { ~2,   ~0U  }, { 0,    2  } },
    { "PD1R_D_N",   V(pad1.r[15]        ), { ~4,   ~0U  }, { 0,    4  } },
    { "PD1R_D_NW",  V(pad1.r[15]        ), { ~8,   ~0U  }, { 0,    8  } },
    { "PD1R_D_W",   V(pad1.r[15]        ), { ~16,  ~0U  }, { 0,    16 } },
    { "PD1R_D_SW",  V(pad1.r[15]        ), { ~32,  ~0U  }, { 0,    32 } },
    { "PD1R_D_S",   V(pad1.r[15]        ), { ~64,  ~0U  }, { 0,    64 } },
    { "PD1R_D_SE",  V(pad1.r[15]        ), { ~128, ~0U  }, { 0,    128} },

    /*
00FEh|                  00FFh bits
bits |   0       1    2  3  4    5        6      7
-----+------------------------------------------------
  0  | left,   comma, n, v, x, space,   [n/a], [n/a]
  1  | period, m,     b, c, z, down,    [n/a], [n/a]
  2  | scolon, k,     h, f, s, up,      [n/a], [n/a]
  3  | p,      i,     y, r, w, q,       [n/a], [n/a]
  4  | esc,    9,     7, 5, 3, 1,       [n/a], [n/a]
  5  | 0,      8,     6, 4, 2, right,   [n/a], [n/a]
  6  | enter,  o,     u, t, e, ctl,     [n/a], [n/a]
  7  | [n/a],  l,     j, g, d, a,       shift, [n/a]
  */
    
    /* -------------------------------------------------------------------- */
    /*  ECS Keyboard                                                        */
    /* -------------------------------------------------------------------- */

    /* bit 0 */
    { "KEYB_LEFT",  V(pad1.k[ 0]        ), { ~  1, ~0U  }, { 0,      1} },
    { "KEYB_PERIOD",V(pad1.k[ 0]        ), { ~  2, ~0U  }, { 0,      2} },
    { "KEYB_SEMI",  V(pad1.k[ 0]        ), { ~  4, ~0U  }, { 0,      4} },
    { "KEYB_P",     V(pad1.k[ 0]        ), { ~  8, ~0U  }, { 0,      8} },
    { "KEYB_ESC",   V(pad1.k[ 0]        ), { ~ 16, ~0U  }, { 0,     16} },
    { "KEYB_0",     V(pad1.k[ 0]        ), { ~ 32, ~0U  }, { 0,     32} },
    { "KEYB_ENTER", V(pad1.k[ 0]        ), { ~ 64, ~0U  }, { 0,     64} },

    /* bit 1 */                                                       
    { "KEYB_COMMA", V(pad1.k[ 1]        ), { ~  1, ~0U  }, { 0,      1} },
    { "KEYB_M",     V(pad1.k[ 1]        ), { ~  2, ~0U  }, { 0,      2} },
    { "KEYB_K",     V(pad1.k[ 1]        ), { ~  4, ~0U  }, { 0,      4} },
    { "KEYB_I",     V(pad1.k[ 1]        ), { ~  8, ~0U  }, { 0,      8} },
    { "KEYB_9",     V(pad1.k[ 1]        ), { ~ 16, ~0U  }, { 0,     16} },
    { "KEYB_8",     V(pad1.k[ 1]        ), { ~ 32, ~0U  }, { 0,     32} },
    { "KEYB_O",     V(pad1.k[ 1]        ), { ~ 64, ~0U  }, { 0,     64} },
    { "KEYB_L",     V(pad1.k[ 1]        ), { ~128, ~0U  }, { 0,    128} },
                                                                      
    /* bit 2 */                                                       
    { "KEYB_N",     V(pad1.k[ 2]        ), { ~  1, ~0U  }, { 0,      1} },
    { "KEYB_B",     V(pad1.k[ 2]        ), { ~  2, ~0U  }, { 0,      2} },
    { "KEYB_H",     V(pad1.k[ 2]        ), { ~  4, ~0U  }, { 0,      4} },
    { "KEYB_Y",     V(pad1.k[ 2]        ), { ~  8, ~0U  }, { 0,      8} },
    { "KEYB_7",     V(pad1.k[ 2]        ), { ~ 16, ~0U  }, { 0,     16} },
    { "KEYB_6",     V(pad1.k[ 2]        ), { ~ 32, ~0U  }, { 0,     32} },
    { "KEYB_U",     V(pad1.k[ 2]        ), { ~ 64, ~0U  }, { 0,     64} },
    { "KEYB_J",     V(pad1.k[ 2]        ), { ~128, ~0U  }, { 0,    128} },
                                                                      
    /* bit 3 */                                                       
    { "KEYB_V",     V(pad1.k[ 3]        ), { ~  1, ~0U  }, { 0,      1} },
    { "KEYB_C",     V(pad1.k[ 3]        ), { ~  2, ~0U  }, { 0,      2} },
    { "KEYB_F",     V(pad1.k[ 3]        ), { ~  4, ~0U  }, { 0,      4} },
    { "KEYB_R",     V(pad1.k[ 3]        ), { ~  8, ~0U  }, { 0,      8} },
    { "KEYB_5",     V(pad1.k[ 3]        ), { ~ 16, ~0U  }, { 0,     16} },
    { "KEYB_4",     V(pad1.k[ 3]        ), { ~ 32, ~0U  }, { 0,     32} },
    { "KEYB_T",     V(pad1.k[ 3]        ), { ~ 64, ~0U  }, { 0,     64} },
    { "KEYB_G",     V(pad1.k[ 3]        ), { ~128, ~0U  }, { 0,    128} },
                                                                      
    /* bit 4 */                                                       
    { "KEYB_X",     V(pad1.k[ 4]        ), { ~  1, ~0U  }, { 0,      1} },
    { "KEYB_Z",     V(pad1.k[ 4]        ), { ~  2, ~0U  }, { 0,      2} },
    { "KEYB_S",     V(pad1.k[ 4]        ), { ~  4, ~0U  }, { 0,      4} },
    { "KEYB_W",     V(pad1.k[ 4]        ), { ~  8, ~0U  }, { 0,      8} },
    { "KEYB_3",     V(pad1.k[ 4]        ), { ~ 16, ~0U  }, { 0,     16} },
    { "KEYB_2",     V(pad1.k[ 4]        ), { ~ 32, ~0U  }, { 0,     32} },
    { "KEYB_E",     V(pad1.k[ 4]        ), { ~ 64, ~0U  }, { 0,     64} },
    { "KEYB_D",     V(pad1.k[ 4]        ), { ~128, ~0U  }, { 0,    128} },
                                                                      
    /* bit 5 */                                                       
    { "KEYB_SPACE", V(pad1.k[ 5]        ), { ~  1, ~0U  }, { 0,      1} },
    { "KEYB_DOWN",  V(pad1.k[ 5]        ), { ~  2, ~0U  }, { 0,      2} },
    { "KEYB_UP",    V(pad1.k[ 5]        ), { ~  4, ~0U  }, { 0,      4} },
    { "KEYB_Q",     V(pad1.k[ 5]        ), { ~  8, ~0U  }, { 0,      8} },
    { "KEYB_1",     V(pad1.k[ 5]        ), { ~ 16, ~0U  }, { 0,     16} },
    { "KEYB_RIGHT", V(pad1.k[ 5]        ), { ~ 32, ~0U  }, { 0,     32} },
    { "KEYB_CTRL",  V(pad1.k[ 5]        ), { ~ 64, ~0U  }, { 0,     64} },
    { "KEYB_A",     V(pad1.k[ 5]        ), { ~128, ~0U  }, { 0,    128} },
                                                                      
    /* bit 6 */                                                       
    { "KEYB_SHIFT", V(pad1.k[ 6]        ), { ~128, ~0U  }, { 0,    128} },

    /* -------------------------------------------------------------------- */
    /*  ECS Keyboard "Shifted" Keys.                                        */
    /* -------------------------------------------------------------------- */
    { "KEYB_EQUAL", V(pad1.k[ 5]), { ~( 16 << 8), ~0U  }, { 0, (16 << 8)} },
    { "KEYB_QUOTE", V(pad1.k[ 4]), { ~( 32 << 8), ~0U  }, { 0, (32 << 8)} },
    { "KEYB_HASH",  V(pad1.k[ 4]), { ~( 16 << 8), ~0U  }, { 0, (16 << 8)} },
    { "KEYB_DOLLAR",V(pad1.k[ 3]), { ~( 32 << 8), ~0U  }, { 0, (32 << 8)} },
    { "KEYB_PLUS",  V(pad1.k[ 3]), { ~( 16 << 8), ~0U  }, { 0, (16 << 8)} },
    { "KEYB_MINUS", V(pad1.k[ 2]), { ~( 32 << 8), ~0U  }, { 0, (32 << 8)} },
    { "KEYB_SLASH", V(pad1.k[ 2]), { ~( 16 << 8), ~0U  }, { 0, (16 << 8)} },
    { "KEYB_STAR",  V(pad1.k[ 1]), { ~( 32 << 8), ~0U  }, { 0, (32 << 8)} },
    { "KEYB_LPAREN",V(pad1.k[ 1]), { ~( 16 << 8), ~0U  }, { 0, (16 << 8)} },
    { "KEYB_RPAREN",V(pad1.k[ 0]), { ~( 32 << 8), ~0U  }, { 0, (32 << 8)} },
    { "KEYB_CARET", V(pad1.k[ 5]), { ~(  4 << 8), ~0U  }, { 0, ( 4 << 8)} },
    { "KEYB_PCT",   V(pad1.k[ 5]), { ~(  2 << 8), ~0U  }, { 0, ( 2 << 8)} },
    { "KEYB_SQUOTE",V(pad1.k[ 0]), { ~(  1 << 8), ~0U  }, { 0, ( 1 << 8)} },
    { "KEYB_QUEST", V(pad1.k[ 5]), { ~( 32 << 8), ~0U  }, { 0, (32 << 8)} },
};
# endif


# if 0
/* ------------------------------------------------------------------------ */
/*  Default key bindings table.                                             */
/*                                                                          */
/*  I really need to make sure there are rows in here for all possible      */
/*  key inputs, so that when I process a config file, I can just update     */
/*  this table directly.  Otherwise, I need to duplicate this table in      */
/*  order to change it.                                                     */
/*                                                                          */
/*  Column 1 is the default.                                                */
/*  Column 2 is the ECS Keyboard setup.                                     */
/*  Column 3 is the ECS Piano setup.                                        */
/*  Column 4 is the ??? setup.                                              */
/* ------------------------------------------------------------------------ */
cfg_kbd_t  cfg_key_bind[] =
{
/* ------------------------------------------------------------------------ */
/*  Miscellaneous.                                                          */
/* ------------------------------------------------------------------------ */
{ "QUIT",   {   "QUIT",         "QUIT",         "QUIT",         "QUIT"      }},
{ "F1",     {   "QUIT",         "QUIT",         "QUIT",         "QUIT"      }},
{ "ESCAPE", {   "NA",           "NA",           "KEYB_ESC",     "NA",       }},
{ "F10",    {   "MOVIE",        "MOVIE",        "MOVIE",        "MOVIE"     }},
{ "F11",    {   "SHOT",         "SHOT",         "SHOT",         "SHOT"      }},
{ "F12",    {   "RESET",        "RESET",        "RESET",        "RESET"     }},
{ "HIDE",   {   "HIDE",         "HIDE",         "HIDE",         "HIDE"      }},
{ "F2",     {   "HIDE",         "HIDE",         "HIDE",         "HIDE"      }},
{ "F4",     {   "BREAK",        "BREAK",        "BREAK",        "BREAK"     }},
{ "F5",     {   "KBD0",         "KBD0",         "KBD0",         "KBD0",     }},
{ "F6",     {   "KBD1",         "KBD1",         "KBD1",         "KBD1",     }},
{ "F7",     {   "KBD2",         "KBD2",         "KBD2",         "KBD2",     }},
{ "F8",     {   "KBD3",         "KBD3",         "KBD3",         "KBD3",     }},
{ "PAUSE",  {   "PAUSE",        "PAUSE",        "PAUSE",        "PAUSE"     }},

/* ------------------------------------------------------------------------ */
/*  The numeric keypad.                                                     */
/* ------------------------------------------------------------------------ */
{ "KP7",    {   "PD0L_KP1",     "PD0L_KP1",     "KEYB_1",       "PD0R_KP1"  }},
{ "KP8",    {   "PD0L_KP2",     "PD0L_KP2",     "KEYB_2",       "PD0R_KP2"  }},
{ "KP9",    {   "PD0L_KP3",     "PD0L_KP3",     "KEYB_3",       "PD0R_KP3"  }},
{ "KP4",    {   "PD0L_KP4",     "PD0L_KP4",     "KEYB_4",       "PD0R_KP4"  }},
{ "KP5",    {   "PD0L_KP5",     "PD0L_KP5",     "KEYB_5",       "PD0R_KP5"  }},
{ "KP6",    {   "PD0L_KP6",     "PD0L_KP6",     "KEYB_6",       "PD0R_KP6"  }},
{ "KP1",    {   "PD0L_KP7",     "PD0L_KP7",     "KEYB_7",       "PD0R_KP7"  }},
{ "KP2",    {   "PD0L_KP8",     "PD0L_KP8",     "KEYB_8",       "PD0R_KP8"  }},
{ "KP3",    {   "PD0L_KP9",     "PD0L_KP9",     "KEYB_9",       "PD0R_KP9"  }},
{ "KP0",    {   "PD0L_KPC",     "PD0L_KPC",     "KEYB_0",       "PD0R_KPC"  }},
{ "KP_PERIOD",{ "PD0L_KP0",     "PD0L_KP0",     "KEYB_PERIOD",  "PD0R_KP0"  }},
{ "KP_ENTER", { "PD0L_KPE",     "PD0L_KPE",     "KEYB_ENTER",   "PD0R_KPE"  }},
                                                                
/* ------------------------------------------------------------------------ */
/*  The number keys.                                                        */
/* ------------------------------------------------------------------------ */
{ "1",      {   "PD0R_KP1",     "PD0R_KP1",     "KEYB_1",       "PD0L_KP1"  }},
{ "2",      {   "PD0R_KP2",     "PD0R_KP2",     "KEYB_2",       "PD0L_KP2"  }},
{ "3",      {   "PD0R_KP3",     "PD0R_KP3",     "KEYB_3",       "PD0L_KP3"  }},
{ "4",      {   "PD0R_KP4",     "PD0R_KP4",     "KEYB_4",       "PD0L_KP4"  }},
{ "5",      {   "PD0R_KP5",     "PD0R_KP5",     "KEYB_5",       "PD0L_KP5"  }},
{ "6",      {   "PD0R_KP6",     "PD0R_KP6",     "KEYB_6",       "PD0L_KP6"  }},
{ "7",      {   "PD0R_KP7",     "PD0R_KP7",     "KEYB_7",       "PD0L_KP7"  }},
{ "8",      {   "PD0R_KP8",     "PD0R_KP8",     "KEYB_8",       "PD0L_KP8"  }},
{ "9",      {   "PD0R_KP9",     "PD0R_KP9",     "KEYB_9",       "PD0L_KP9"  }},
{ "-",      {   "PD0R_KPC",     "PD0R_KPC",     "KEYB_MINUS",   "PD0L_KPC"  }},
{ "0",      {   "PD0R_KP0",     "PD0R_KP0",     "KEYB_0",       "PD0L_KP0"  }},
{ "=",      {   "PD0R_KPE",     "PD0R_KPE",     "KEYB_EQUAL",   "PD0L_KPE"  }},
                                                                
/* ------------------------------------------------------------------------ */
/*  Action buttons.                                                         */
/* ------------------------------------------------------------------------ */
{ "RSHIFT", {   "PD0L_A_T",     "PD0L_A_T",     "KEYB_SHIFT",   "PD0R_A_T"  }},
{ "RALT",   {   "PD0L_A_L",     "PD0L_A_L",     "NA",           "PD0R_A_L"  }},
{ "RCTRL",  {   "PD0L_A_R",     "PD0L_A_R",     "KEYB_CTRL",    "PD0R_A_R"  }},
                                                                
{ "LSHIFT", {   "PD0R_A_T",     "PD0R_A_T",     "KEYB_SHIFT",   "PD0L_A_T"  }},
{ "LALT",   {   "PD0R_A_L",     "PD0R_A_L",     "NA",           "PD0L_A_L"  }},
{ "LCTRL",  {   "PD0R_A_R",     "PD0R_A_R",     "KEYB_CTRL",    "PD0L_A_R"  }},
                                                                
/* ------------------------------------------------------------------------ */
/*  Movement keys.                                                          */
/* ------------------------------------------------------------------------ */
{ "RIGHT",  {   "PD0L_D_E",     "PD0L_D_E",     "KEYB_RIGHT",   "PD0R_D_E"  }},
{ "UP",     {   "PD0L_D_N",     "PD0L_D_N",     "KEYB_UP",      "PD0R_D_N"  }},
{ "LEFT",   {   "PD0L_D_W",     "PD0L_D_W",     "KEYB_LEFT",    "PD0R_D_W"  }},
{ "DOWN",   {   "PD0L_D_S",     "PD0L_D_S",     "KEYB_DOWN",    "PD0R_D_S"  }},
                                                                
{ "K",      {   "PD0L_D_E",     "PD0L_D_E",     "KEYB_K",       "PD0R_D_E"  }},
{ "O",      {   "PD0L_D_NE",    "PD0L_D_NE",    "KEYB_O",       "PD0R_D_NE" }},
{ "I",      {   "PD0L_D_N",     "PD0L_D_N",     "KEYB_I",       "PD0R_D_N"  }},
{ "U",      {   "PD0L_D_NW",    "PD0L_D_NW",    "KEYB_U",       "PD0R_D_NW" }},
{ "J",      {   "PD0L_D_W",     "PD0L_D_W",     "KEYB_J",       "PD0R_D_W"  }},
{ "N",      {   "PD0L_D_SW",    "PD0L_D_SW",    "KEYB_N",       "PD0R_D_SW" }},
{ "M",      {   "PD0L_D_S",     "PD0L_D_S",     "KEYB_M",       "PD0R_D_S"  }},
{ ",",      {   "PD0L_D_SE",    "PD0L_D_SE",    "KEYB_COMMA",   "PD0R_D_SE" }},
                                                                
{ "D",      {   "PD0R_D_E",     "PD0R_D_E",     "KEYB_D",       "PD0L_D_E"  }},
{ "R",      {   "PD0R_D_NE",    "PD0R_D_NE",    "KEYB_R",       "PD0L_D_NE" }},
{ "E",      {   "PD0R_D_N",     "PD0R_D_N",     "KEYB_E",       "PD0L_D_N"  }},
{ "W",      {   "PD0R_D_NW",    "PD0R_D_NW",    "KEYB_W",       "PD0L_D_NW" }},
{ "S",      {   "PD0R_D_W",     "PD0R_D_W",     "KEYB_S",       "PD0L_D_W"  }},
{ "Z",      {   "PD0R_D_SW",    "PD0R_D_SW",    "KEYB_Z",       "PD0L_D_SW" }},
{ "X",      {   "PD0R_D_S",     "PD0R_D_S",     "KEYB_X",       "PD0L_D_S"  }},
{ "C",      {   "PD0R_D_SE",    "PD0R_D_SE",    "KEYB_C",       "PD0L_D_SE" }},
                                                                
    /*                                                          
00FEh|                  00FFh bits                              
bits |   0       1    2  3  4    5        6      7              
-----+------------------------------------------------          
  0  | left,   comma, n, v, x, space,   [n/a], [n/a]            
  1  | period, m,     b, c, z, down,    [n/a], [n/a]            
  2  | scolon, k,     h, f, s, up,      [n/a], [n/a]            
  3  | p,      i,     y, r, w, q,       [n/a], [n/a]            
  4  | esc,    9,     7, 5, 3, 1,       [n/a], [n/a]            
  5  | 0,      8,     6, 4, 2, right,   [n/a], [n/a]            
  6  | enter,  o,     u, t, e, ctl,     [n/a], [n/a]            
  7  | [n/a],  l,     j, g, d, a,       shift, [n/a]            
  */                                                            
                                                                
/* ------------------------------------------------------------------------ */
/*  ECS Keyboard remaining keys.                                            */
/* ------------------------------------------------------------------------ */
{ "Q",      {   "NA",           "NA",           "KEYB_Q",       "NA"        }},
{ "T",      {   "NA",           "NA",           "KEYB_T",       "NA"        }},
{ "Y",      {   "NA",           "NA",           "KEYB_Y",       "NA"        }},
{ "P",      {   "NA",           "NA",           "KEYB_P",       "NA"        }},
                                                                
{ "A",      {   "NA",           "NA",           "KEYB_A",       "NA"        }},
{ "F",      {   "NA",           "NA",           "KEYB_F",       "NA"        }},
{ "G",      {   "NA",           "NA",           "KEYB_G",       "NA"        }},
{ "H",      {   "NA",           "NA",           "KEYB_H",       "NA"        }},
{ "L",      {   "NA",           "NA",           "KEYB_L",       "NA"        }},
                                                                
{ "V",      {   "NA",           "NA",           "KEYB_V",       "NA"        }},
{ "B",      {   "NA",           "NA",           "KEYB_B",       "NA"        }},
{ ".",      {   "NA",           "NA",           "KEYB_PERIOD",  "NA"        }},
{ ";",      {   "NA",           "NA",           "KEYB_SEMI",    "NA"        }},
                                                                
{ " ",      {   "NA",           "NA",           "KEYB_SPACE",   "NA"        }},
{ "RETURN", {   "NA",           "NA",           "KEYB_ENTER",   "NA"        }},
{"BACKSPACE",{  "NA",           "NA",           "KEYB_LEFT",    "NA"        }},

{ "QUOTEDBL",{  "NA",           "NA",           "KEYB_QUOTE",   "NA"        }},
{ "QUOTE",  {   "NA",           "NA",           "KEYB_QUOTE",   "NA"        }},
{ "CARET",  {   "NA",           "NA",           "KEYB_CARET",   "NA"        }},
{ "HASH",   {   "NA",           "NA",           "KEYB_HASH",    "NA"        }},
{ "PLUS",   {   "NA",           "NA",           "KEYB_PLUS",    "NA"        }},
{ "SLASH",  {   "NA",           "NA",           "KEYB_SLASH",   "NA"        }},
{ "DOLLAR", {   "NA",           "NA",           "KEYB_DOLLAR",  "NA"        }},
{ "ASTERISK",{  "NA",           "NA",           "KEYB_STAR",    "NA"        }},
{ "LEFTPAREN",{ "NA",           "NA",           "KEYB_LPAREN",  "NA"        }},
{ "RIGHTPAREN",{"NA",           "NA",           "KEYB_RPAREN",  "NA"        }},
{ "QUESTION",{  "NA",           "NA",           "KEYB_QUEST",   "NA"        }},

/* ------------------------------------------------------------------------ */
/*  Default Joystick 0 mapping.                                             */
/* ------------------------------------------------------------------------ */
{ "JS0_E",    { "PD0L_J_E",     "PD0L_J_E",     "PD0L_J_E",     "PD0L_J_E"  }},
{ "JS0_ENE",  { "PD0L_J_ENE",   "PD0L_J_ENE",   "PD0L_J_ENE",   "PD0L_J_ENE"}},
{ "JS0_NE",   { "PD0L_J_NE",    "PD0L_J_NE",    "PD0L_J_NE",    "PD0L_J_NE" }},
{ "JS0_NNE",  { "PD0L_J_NNE",   "PD0L_J_NNE",   "PD0L_J_NNE",   "PD0L_J_NNE"}},
{ "JS0_N",    { "PD0L_J_N",     "PD0L_J_N",     "PD0L_J_N",     "PD0L_J_N"  }},
{ "JS0_NNW",  { "PD0L_J_NNW",   "PD0L_J_NNW",   "PD0L_J_NNW",   "PD0L_J_NNW"}},
{ "JS0_NW",   { "PD0L_J_NW",    "PD0L_J_NW",    "PD0L_J_NW",    "PD0L_J_NW" }},
{ "JS0_WNW",  { "PD0L_J_WNW",   "PD0L_J_WNW",   "PD0L_J_WNW",   "PD0L_J_WNW"}},
{ "JS0_W",    { "PD0L_J_W",     "PD0L_J_W",     "PD0L_J_W",     "PD0L_J_W"  }},
{ "JS0_WSW",  { "PD0L_J_WSW",   "PD0L_J_WSW",   "PD0L_J_WSW",   "PD0L_J_WSW"}},
{ "JS0_SW",   { "PD0L_J_SW",    "PD0L_J_SW",    "PD0L_J_SW",    "PD0L_J_SW" }},
{ "JS0_SSW",  { "PD0L_J_SSW",   "PD0L_J_SSW",   "PD0L_J_SSW",   "PD0L_J_SSW"}},
{ "JS0_S",    { "PD0L_J_S",     "PD0L_J_S",     "PD0L_J_S",     "PD0L_J_S"  }},
{ "JS0_SSE",  { "PD0L_J_SSE",   "PD0L_J_SSE",   "PD0L_J_SSE",   "PD0L_J_SSE"}},
{ "JS0_SE",   { "PD0L_J_SE",    "PD0L_J_SE",    "PD0L_J_SE",    "PD0L_J_SE" }},
{ "JS0_ESE",  { "PD0L_J_ESE",   "PD0L_J_ESE",   "PD0L_J_ESE",   "PD0L_J_ESE"}},

{"JS0_BTN_00",{ "PD0L_A_T",     "PD0L_A_T",     "PD0L_A_T",     "PD0L_A_T"  }},
{"JS0_BTN_01",{ "PD0L_A_L",     "PD0L_A_L",     "PD0L_A_L",     "PD0L_A_L"  }},
{"JS0_BTN_02",{ "PD0L_A_R",     "PD0L_A_R",     "PD0L_A_R",     "PD0L_A_R"  }},
{"JS0_BTN_03",{ "PD0L_A_T",     "PD0L_A_T",     "PD0L_A_T",     "PD0L_A_T"  }},
{"JS0_BTN_04",{ "PD0L_A_L",     "PD0L_A_L",     "PD0L_A_L",     "PD0L_A_L"  }},
{"JS0_BTN_05",{ "PD0L_A_R",     "PD0L_A_R",     "PD0L_A_R",     "PD0L_A_R"  }},
{"JS0_BTN_06",{ "PD0L_A_T",     "PD0L_A_T",     "PD0L_A_T",     "PD0L_A_T"  }},
{"JS0_BTN_07",{ "PD0L_A_L",     "PD0L_A_L",     "PD0L_A_L",     "PD0L_A_L"  }},
{"JS0_BTN_08",{ "PD0L_A_R",     "PD0L_A_R",     "PD0L_A_R",     "PD0L_A_R"  }},
                                                                
{"JS0_HAT0_E", {"PD0R_KP6",     "PD0R_KP6",     "PD0R_KP6",     "PD0R_KP6"  }},
{"JS0_HAT0_NE",{"PD0R_KP3",     "PD0R_KP3",     "PD0R_KP3",     "PD0R_KP3"  }},
{"JS0_HAT0_N", {"PD0R_KP2",     "PD0R_KP2",     "PD0R_KP2",     "PD0R_KP2"  }},
{"JS0_HAT0_NW",{"PD0R_KP1",     "PD0R_KP1",     "PD0R_KP1",     "PD0R_KP1"  }},
{"JS0_HAT0_W", {"PD0R_KP4",     "PD0R_KP4",     "PD0R_KP4",     "PD0R_KP4"  }},
{"JS0_HAT0_SW",{"PD0R_KP7",     "PD0R_KP7",     "PD0R_KP7",     "PD0R_KP7"  }},
{"JS0_HAT0_S", {"PD0R_KP8",     "PD0R_KP8",     "PD0R_KP8",     "PD0R_KP8"  }},
{"JS0_HAT0_SE",{"PD0R_KP9",     "PD0R_KP9",     "PD0R_KP9",     "PD0R_KP9"  }},

{ NULL,     {   NULL,           NULL,           NULL,           NULL        }},

};
# endif

/* ======================================================================== */
/*  CFG_GET_EVT  -- Convert an event name into an index.  This is a         */
/*                  horrible linear search.  :-P  Hey, it works for now.    */
/* ======================================================================== */
# if 0
int cfg_get_evt(const char *event)
{
    int i;

    for (i = 0; i < (int)(sizeof(cfg_event)/sizeof(cfg_evt_t)); i++)
    {
        if (!strcmp(cfg_event[i].event, event))
            return i;
    }

    return -1;
}
# endif

/* ======================================================================== */
/*  CFG_SETBIND  -- Set all of the key-bindings for the Intellivision.      */
/* ======================================================================== */
# if 0 //LUDO:
void cfg_setbind(cfg_t *cfg, char *kbdhackfile)
{
    int i, j;
    int e;
    FILE *f;
    char buf[256];

    /* -------------------------------------------------------------------- */
    /*  Iterate over the bindings table.                                    */
    /* -------------------------------------------------------------------- */
    for (i = 0; cfg->binding[i].key != NULL; i++)
    {
        /* ---------------------------------------------------------------- */
        /*  Iterate over the four possible "event spaces" that the user     */
        /*  may have configured.  For instance, the user may have set up    */
        /*  "Normal", "Swapped", "Alpha-numeric", and one other.            */
        /* ---------------------------------------------------------------- */
        for (j = 0; j < 4; j++)
        {
            /* ------------------------------------------------------------ */
            /*  Skip empty event bindings.  These keys aren't bound.        */
            /* ------------------------------------------------------------ */
            if (!cfg->binding[i].event[j] || !cfg->binding[i].event[j][0]) 
                continue;

            /* ------------------------------------------------------------ */
            /*  Look up the event name, and skip if the name is invalid.    */
            /* ------------------------------------------------------------ */
            e = cfg_get_evt(cfg->binding[i].event[j]);
            if (e < 0)
            {
                fprintf(stderr, "cfg: Invalid event name '%s'\n", 
                        cfg->binding[i].event[j]);
                continue;
            }

            /* ------------------------------------------------------------ */
            /*  Map the key to the event.                                   */
            /* ------------------------------------------------------------ */
            event_map(&cfg->event, cfg->binding[i].key, j,
                      cfg_event[e].word, cfg_event[e].and_mask, 
                      cfg_event[e].or_mask);
        }
    }

    /* -------------------------------------------------------------------- */
    /*  HACK: If the user specified a keyboard mapping file, read that in.  */
    /* -------------------------------------------------------------------- */
    if (!kbdhackfile)
        return;

    if (!(f = fopen(kbdhackfile, "r")))
    {
        fprintf(stderr, "Couldn't open keyboard map file '%s'\n", kbdhackfile);
        exit(1);
    }

    j = 0;
    while (fgets(buf, 256, f) != NULL)
    {
        char *s1, *s2;
        char cmd[256], arg[256];

        if ((s1 = strrchr(buf, '\r'))) *s1 = 0;
        if ((s1 = strrchr(buf, '\n'))) *s1 = 0;

        cmd[0] = 0;
        arg[0] = 0;

        s1 = buf;
        while (*s1 && isspace(*s1)) s1++;
        s2 = cmd;
        while (*s1 && !isspace(*s1)) *s2++ = *s1++;
        *s2 = 0;
        while (*s1 && isspace(*s1)) s1++;
        s2 = arg;
        while (*s1 && !isspace(*s1)) *s2++ = *s1++;
        *s2 = 0;

        if (!stricmp(cmd, "map")) { j = atoi(arg); continue; }

        if (cmd[0] == 0 || arg[0] == 0)
        {
            if (cmd[0])
            {
                fprintf(stderr, "unknown command '%s' in %s\n",
                        cmd, kbdhackfile);
            }
            continue;
        }

        printf("binding %s to %s in map %d\n", cmd, arg, j);

        if ((e = cfg_get_evt(arg)) < 0)
        {
            fprintf(stderr, "cfg: Invalid event name '%s'\n", arg);
            continue;
        }

        event_map(&cfg->event, cmd, j, cfg_event[e].word, 
                cfg_event[e].and_mask, cfg_event[e].or_mask);
    }

    fclose(f);
}
# endif

/* ======================================================================== */
/*  CFG_LONGOPT  -- Long options for getopt_long                            */
/* ======================================================================== */
# if 0 //LUDO: 
struct option cfg_longopt[] =
{
    {   "ecsimg",       1,      NULL,       'E'     },
    {   "execimg",      1,      NULL,       'e'     },
    {   "gromimg",      1,      NULL,       'g'     },
    {   "ecs",          2,      NULL,       's'     },
    {   "fullscreen",   2,      NULL,       'f'     },
    {   "audiofile",    1,      NULL,       'F'     },
    {   "debugger",     0,      NULL,       'd'     },
    {   "ratecontrol",  2,      NULL,       'r'     },
    {   "fullscreen",   2,      NULL,       'x'     },
    {   "displaysize",  2,      NULL,       'z'     },
    {   "audio",        1,      NULL,       'a'     },
    {   "audiorate",    1,      NULL,       'a'     },
    {   "audiowindow",  1,      NULL,       'w'     },
    {   "audiobufsize", 1,      NULL,       'B'     },
    {   "audiobufcnt",  1,      NULL,       'C'     },
    {   "audiomintick", 1,      NULL,       'M'     },
    {   "voice",        2,      NULL,       'v'     },
    {   "voicewindow",  2,      NULL,       'W'     },
    {   "voicefiles",   2,      NULL,       'V'     },
    {   "i2pc0",        2,      NULL,       'i'     },
    {   "i2pc1",        2,      NULL,       'I'     },
    {   "intv2pc0",     2,      NULL,       'i'     },
    {   "intv2pc1",     2,      NULL,       'I'     },
#ifdef CGC_DLL
    {   "cgc0",         2,      NULL,       1       },
    {   "cgc1",         2,      NULL,       2       },
#endif
#ifdef CGC_THREAD
    {   "cgc0",         1,      NULL,       1       },
    {   "cgc1",         1,      NULL,       2       },
#endif
    {   "icartcache",   2,      NULL,       'c'     },
    {   "help",         0,      NULL,       'h'     },
    {   "license",      0,      NULL,       'l'     },
    {   "nobusywait",   0,      NULL,       '9'     },
    {   "kbdhackfile",  1,      NULL,       3       },
    {   "demofile",     1,      NULL,       'D'     },

    {   "js0",          2,      NULL,       4       },
    {   "js1",          2,      NULL,       5       },
    {   "js2",          2,      NULL,       6       },
    {   "js3",          2,      NULL,       7       },

    {   NULL,           0,      NULL,       0       }
};

LOCAL const char *optchars= "E:e:g:s::f::F:?dhr::x::z::a:w:B:C:M:"
                            "v::W::V::i::I::c:";

extern char *optarg;
extern int   optind, opterr, optopt;
# endif

/* ======================================================================== */
/*  Supported I/O addresses for INTV2PC.                                    */
/* ======================================================================== */
const uint_32 i2pc_ports[4] = { 0x0, 0x378, 0x278, 0x3BC };

/* ======================================================================== */
/*  Supported display resolutions.                                          */
/* ======================================================================== */
LOCAL const int res_x[] = { 320};
LOCAL const int res_y[] = { 200};
LOCAL const int res_d[] = { 16};
#define NUM_RES ((int)(sizeof(res_x) / sizeof(res_x[0])))
LOCAL char *joy_cfg[MAX_JOY];

/* ======================================================================== */
/*  CFG_INIT     -- Parse command line and get started                      */
/* ======================================================================== */
cfg_t *cfg_init(int argc, char * argv[])
{
# if 1 //LUDO: 
    char buffer[128];
    char tmp_filename[128];
# endif
    int c, option_idx = 0, rx, ry, rd;
    int exec_type = 0, legacy_rom = 0; 
    int value = 1, busywaits = 1;
    uint_32 cache_flags = IC_CACHE_DFLT;
    char *audiofile = NULL, *tmp;
    char *kbdhackfile = NULL;
    char *demofile = NULL;
    FILE *f;
    ser_hier_t *ser_cfg;

    /* -------------------------------------------------------------------- */
    /*  Set up the default state for everything.                            */
    /* -------------------------------------------------------------------- */
    memset(&top_intv, 0, sizeof(cfg_t));

    top_intv.audio_rate = 44100;        /* high quality.  :-)                   */
    top_intv.psg_window = -1;           /* Automatic window setting.            */
    top_intv.ecs_enable = -1;           /* Automatic (dflt: ECS off)            */
    top_intv.ivc_enable = -1;           /* Automatic (dflt: Intellivoice off.   */
    top_intv.ivc_window = -1;           /* Automatic window setting.            */
    top_intv.full_scr   = 0;            /* Windowed.                            */
    top_intv.i2pc0_port = 0;            /* No INTV2PC #0                        */
    top_intv.i2pc1_port = 0;            /* No INTV2PC #1                        */
    top_intv.cgc0_num   = -1;           /* No CGC #0                            */
    top_intv.cgc1_num   = -1;           /* No CGC #1                            */
    top_intv.cgc0_dev   = NULL;         /* No CGC #0                            */
    top_intv.cgc1_dev   = NULL;         /* No CGC #1                            */
    top_intv.debugging  = 0;            /* No debugger.                         */
    top_intv.rate_ctl   = 1;            /* Rate control enabled.                */
    top_intv.disp_res   = 0;            /* 320x200                              */
    top_intv.accutick   = 1;            /* fully accurate audio.                */
# if 0 //LUDO:
    top_intv.binding    = cfg_key_bind; /* default key bindings.                */
# endif

# if 0 //LUDO: 
    top_intv.fn_exec    = "exec.bin";   /* Look in current dir by default.      */
    top_intv.fn_grom    = "grom.bin";   /* Look in current dir by default.      */
    top_intv.fn_game    = "game.rom";   /* Look in current dir by default.      */
    top_intv.fn_ecs     = "ecs.bin";    /* Look in current dir by default.      */
# else
    getcwd(buffer, 128);
    strcpy(tmp_filename, buffer);
    strcat(tmp_filename, "/exec.bin");
    top_intv.fn_exec    = strdup(tmp_filename);
    
    strcpy(tmp_filename, buffer);
    strcat(tmp_filename, "/grom.bin");
    top_intv.fn_grom    = strdup(tmp_filename);

    strcpy(tmp_filename, buffer);
    strcat(tmp_filename, "/game.bin");
    top_intv.fn_game    = strdup(tmp_filename);

    strcpy(tmp_filename, buffer);
    strcat(tmp_filename, "/ecs.bin");
    top_intv.fn_ecs     = strdup(tmp_filename);
# endif


    /* -------------------------------------------------------------------- */
    /*  Register our config variables for serialization.                    */
    /* -------------------------------------------------------------------- */
#define SER_REG(x,t,l,f)\
    ser_register(ser_cfg, #x, &top_intv. x, t, l, f)

    ser_cfg = ser_new_hierarchy(NULL, "cfg");
    SER_REG(ecs_enable, ser_s32,    1,  SER_INIT|SER_MAND);
    SER_REG(ivc_enable, ser_s32,    1,  SER_INIT|SER_MAND);
    SER_REG(ivc_tname,  ser_string, 1,  SER_INIT|SER_MAND);


    if (snd_buf_size < 1) snd_buf_size = SND_BUF_SIZE_DEFAULT;
    if (snd_buf_cnt  < 1) snd_buf_size = SND_BUF_CNT_DEFAULT;

    /* -------------------------------------------------------------------- */
    /*  Sanity-check some of the flags.  Most get checked by peripherals.   */
    /* -------------------------------------------------------------------- */
    if (top_intv.disp_res >= NUM_RES)
    {
        int i;
        fprintf(stderr, "Display resolution # out of range.  "
                        "Valid resolutions:\n");

        for (i = 0; i < NUM_RES; i++)
        {
            fprintf(stderr, "    -z%d:  %dx%dx%x\n", 
                    i, res_x[i], res_y[i], res_d[i]);
        }
        exit(1);
    } else
    {
        rx = res_x[top_intv.disp_res];
        ry = res_y[top_intv.disp_res];
        rd = res_d[top_intv.disp_res];
    }

    /* -------------------------------------------------------------------- */
    /*  Look up INTV2PC port numbers, if any.                               */
    /* -------------------------------------------------------------------- */
    if (top_intv.i2pc0_port > 3 || top_intv.i2pc1_port > 3)
    {
# if 0 //LUDO:
        fprintf(stderr, "ERROR:  "
            "INTV2PC port number out of range.  Valid values are 1..3 for\n"
            "typical ports for LPT1: through LPT3:, and 0 to disable.\n"
            "\n"
            "The following port numbers are selected by 1 through 3:\n"
            "   1 selects 0x%.3X\n"
            "   2 selects 0x%.3X\n"
            "   3 selects 0x%.3X\n"
            "\n", i2pc_ports[1], i2pc_ports[2], i2pc_ports[3]);
# endif
        exit(1);
    }
    if (top_intv.i2pc0_port && top_intv.i2pc0_port == top_intv.i2pc1_port)
    {
# if 0 //LUDO:
        fprintf(stderr, "ERROR:  Cannot enable two INTV2PCs on same port #\n");
# endif
        exit(1);
    }
    top_intv.i2pc0_port = i2pc_ports[top_intv.i2pc0_port];
    top_intv.i2pc1_port = i2pc_ports[top_intv.i2pc1_port];

    /* -------------------------------------------------------------------- */
    /*  Create a new peripheral bus for the Intellivision main console.     */
    /* -------------------------------------------------------------------- */
    top_intv.intv = periph_new(16, 16, 4);
    strncpy(top_intv.intv->periph.name, "Master Component", 16);

    /* -------------------------------------------------------------------- */
    /*  Now, configure the Intellivision according to our flags.  Start     */
    /*  off by reading in the EXEC, GROM, and GAME images.                  */
    /* -------------------------------------------------------------------- */
    f = fopen(top_intv.fn_exec, "rb");  
//  if (errno) perror("errno value");
    exec_type = 0;
    if (!f || file_read_rom16(f, 4096, top_intv.exec_img) != 4096)
    {
        if (errno) perror("errno value");
        fprintf(stderr, "ERROR:  Could not read EXEC image '%s'\n",
                top_intv.fn_exec);
        exit(1);
    }
    fseek(f, 0, SEEK_END);
    if (ftell(f) == 2 * (4096 + 256))
    {
        exec_type = 1;
        fseek(f, 8192, SEEK_SET);
        if (file_read_rom16(f, 256, top_intv.exec_img + 4096) != 256)
        {
            if (errno) perror("errno value");
            fprintf(stderr, "ERROR:  Could not read EXEC2 image '%s'\n",
                    top_intv.fn_exec);
            exit(1);
        }
    }
    fclose(f);

    f = fopen(top_intv.fn_grom, "rb");
    if (!f || file_read_rom8 (f, 2048, top_intv.grom_img) != 2048)
    {
        if (errno) perror("errno value");
        fprintf(stderr, "ERROR:  Could not read GROM image '%s'\n",
                top_intv.fn_grom);
        exit(1);
    }
    fclose(f);

    tmp = legacy_bincfg(&(top_intv.legacy), top_intv.fn_game);
    if (tmp == NULL)
    {
        fprintf(stderr, "ERROR:  Failed to initialize legacy BIN+CFG game\n");
        exit(1);
    }
    if (tmp == top_intv.fn_game)
    {
        legacy_rom = 1;
    } else
    {
        top_intv.fn_game = tmp;
    }

    if (!legacy_rom)
    {
        f = fopen(top_intv.fn_game, "rb");
        if (errno) perror("errno value");

        /* ---------------------------------------------------------------- */
        /*  Process the Intellicart ROM itself.                             */
        /* ---------------------------------------------------------------- */
        if (icart_init(&top_intv.icart, f, NULL))
        {
            fprintf(stderr, "ERROR:  Failed to register Intellicart\n");
            exit(1);
        }

        /* ---------------------------------------------------------------- */
        /*  TODO:  Process meta-data tags on Intellicart image.             */
        /* ---------------------------------------------------------------- */
        fclose(f);
    }


    /* -------------------------------------------------------------------- */
    /*  Initialize the peripherals.                                         */
    /* -------------------------------------------------------------------- */
    printf("jzintv:  Initializing Master Component and peripherals...\n");

    if (emu_link_init())
    {
        fprintf(stderr, "ERROR:  Failed to initialize EMU_LINK\n");
        exit(1);
    }

    if (demofile &&
		demo_init(&top_intv.demo, demofile, &top_intv.psg0, 
		          top_intv.ecs_enable > 0 ? &top_intv.psg1 : 0))
    {
        fprintf(stderr, "ERROR:  Failed to initialize demo recorder\n");
        exit(1);
	}

    if (gfx_init(&top_intv.gfx, rx, ry, rd, top_intv.full_scr))
    {
        fprintf(stderr, "ERROR:  Failed to initialize graphics\n");
        exit(1);
    }

    if (top_intv.audio_rate && snd_init(&top_intv.snd, top_intv.audio_rate, audiofile))
    {
        fprintf(stderr, "WARNING:  Failed to initialize sound.  Disabled.\n");
        top_intv.audio_rate = 0;
    }

    if (cp1600_init(&top_intv.cp1600, 0x1000, 0x1004))
    {
        fprintf(stderr, "ERROR:  Failed to initialize CP-1610 CPU\n");
        exit(1);
    }

    if (mem_make_ram  (&top_intv.scr_ram,  8, 0x0100, 8) ||
        mem_make_ram  (&top_intv.sys_ram, 16, 0x0200, 9) ||
        mem_make_9600a(&top_intv.sys_ram2,    0x0300, 8)/* ||
        mem_make_ram  (&top_intv.gram,     8, 0x3800, 9)*/)
    {
        fprintf(stderr, "ERROR:  Failed to initialize RAMs\n");
        exit(1);
    }

    if (stic_init(&top_intv.stic, top_intv.grom_img, &top_intv.cp1600.req_bus, 
                  &top_intv.gfx, demofile ? &top_intv.demo : NULL))
    {
        fprintf(stderr, "ERROR:  Failed to initialize STIC\n");
        exit(1);
    }

    if (top_intv.ecs_enable > 0)
    {

        f = fopen(top_intv.fn_ecs, "rb");
        if (!f || file_read_rom16(f, 12*1024, top_intv.ecs_img) != 12*1024)
        {
            if (errno) perror("errno value");
            fprintf(stderr, "ERROR:  Could not read ECS ROM image '%s'\n",
                    top_intv.fn_ecs);
            exit(1);
        }
        fclose(f);

        if (mem_make_prom(&top_intv.ecs0, 16, 0x2000, 12, 1, top_intv.ecs_img     ) ||
            mem_make_prom(&top_intv.ecs1, 16, 0x7000, 12, 0, top_intv.ecs_img+4096) ||
            mem_make_prom(&top_intv.ecs2, 16, 0xE000, 12, 1, top_intv.ecs_img+8192))
        {
            fprintf(stderr, "ERROR:  Can't make Paged ROM from ECS image\n");
            exit(1);
        }
        if (mem_make_ram(&top_intv.ecs_ram, 8, 0x4000, 11))
        {
            fprintf(stderr, "ERROR:  Can't allocate ECS RAM\n");
            exit(1);
        }
    }


    if (ay8910_init(&top_intv.psg0, 0x1F0, &top_intv.snd, 
                    top_intv.audio_rate, top_intv.psg_window, top_intv.accutick))
    {
        fprintf(stderr, "ERROR:  Failed to initialize PSG#1 (AY8914)\n");
        exit(1);
    }

    if (top_intv.ecs_enable > 0 &&
        ay8910_init(&top_intv.psg1, 0x0F0, &top_intv.snd, 
                    top_intv.audio_rate, top_intv.psg_window, top_intv.accutick))
    {
        fprintf(stderr, "ERROR:  Failed to initialize PSG#2 (AY8914)\n");
        exit(1);
    }


    if (pad_init(&top_intv.pad0, 0x1F0, PAD_HAND))
    {
        fprintf(stderr, "ERROR:  Failed to initialize game pads\n");
        exit(1);
    }
    if (top_intv.ecs_enable > 0 &&
        pad_init(&top_intv.pad1, 0x0F0, PAD_KEYBOARD))
    {
        fprintf(stderr, "ERROR:  Failed to ECS input device\n");
        exit(1);
    }

    if (top_intv.i2pc0_port > 0 &&
        pad_intv2pc_init(&top_intv.i2pc0, 0x1F0, top_intv.i2pc0_port))
    {
        fprintf(stderr, "ERROR:  Failed to initialize INTV2PC #0 at 0x%.3X\n",
                top_intv.i2pc0_port);
        exit(1);
    }
    if (top_intv.ecs_enable > 0 &&
        top_intv.i2pc1_port &&
        pad_intv2pc_init(&top_intv.i2pc1, 0x0F0, top_intv.i2pc1_port))
    {
        fprintf(stderr, "ERROR:  Failed to initialize INTV2PC #1 at 0x%.3X\n",
                top_intv.i2pc1_port);
        exit(1);
    }

    if (top_intv.cgc0_num >= 0 &&
        pad_cgc_init(&top_intv.cgc0, 0x1F0, top_intv.cgc0_num, top_intv.cgc0_dev))
    {
        fprintf(stderr, "ERROR:  Failed to initialize CGC #%d as pad pair 0\n",
                top_intv.cgc0_num);
        exit(1);
    }

    if (top_intv.ecs_enable > 0 &&
        top_intv.cgc1_num >= 0 &&
        pad_cgc_init(&top_intv.cgc1, 0x0F0, top_intv.cgc1_num, top_intv.cgc0_dev))
    {
        fprintf(stderr, "ERROR:  Failed to initialize CGC #%d as pad pair 1\n",
                top_intv.cgc1_num);
        exit(1);
    }

    if (top_intv.rate_ctl && speed_init(&top_intv.speed, &top_intv.gfx, busywaits))
    {
        fprintf(stderr, "ERROR:  Failed to initialize rate control.\n");
        exit(1);
    }

    if (joy_init(1, joy_cfg))
    {
        fprintf(stderr, "ERROR:  Failed to initialize joystick subsystem.\n");
        exit(1);
    }

    if (event_init(&top_intv.event))
    {
        fprintf(stderr, "ERROR:  Failed to initialize event subsystem.\n");
        exit(1);
    }
    
# if 0 //LUDO:
    cfg_setbind(&top_intv, kbdhackfile);
# endif

    if (top_intv.ivc_enable > 0 && top_intv.audio_rate > 0 &&
        ivoice_init(&top_intv.ivoice, 0x80, &top_intv.snd, 
                    top_intv.audio_rate, top_intv.ivc_window, top_intv.ivc_tname))
    {
        fprintf(stderr, "ERROR:  Failed to initialize Intellivoice\n");
        exit(1);
    }

    /* -------------------------------------------------------------------- */
    /*  Note:  We handle the EXEC ROM specially, since it's weird on        */
    /*  the Intellivision 2.                                                */
    /* -------------------------------------------------------------------- */
    if (exec_type == 0)
    {
        if (mem_make_rom(&top_intv.exec,     10, 0x1000, 12, top_intv.exec_img))
        {
            fprintf(stderr, "ERROR:  Failed to initialize EXEC ROM\n");
            exit(1);
        }
    } else 
    {
        if (mem_make_rom(&top_intv.exec,     10, 0x1000, 12, top_intv.exec_img+256) ||
            mem_make_rom(&top_intv.exec2,    10, 0x0400,  8, top_intv.exec_img))
        {
            fprintf(stderr, "ERROR:  Failed to initialize EXEC2 ROM\n");
            exit(1);
        }
    }

    /* -------------------------------------------------------------------- */
    /*  Now register all the devices on the Intellivision's bus.            */
    /* -------------------------------------------------------------------- */
    #define P(x) top_intv.intv, ((periph_p)(&(top_intv.x)))

    periph_register    (P(cp1600         ),  0x0000, 0x0000, "CP-1610"     );

    periph_register    (P(psg0           ),  0x01F0, 0x01FF, "PSG0 AY8914" );
    if (top_intv.ecs_enable > 0)
        periph_register(P(psg1           ),  0x00F0, 0x00FF, "PSG1 AY8914" );

    if (top_intv.ivc_enable > 0 && top_intv.audio_rate)
        periph_register(P(ivoice         ),  0x0080, 0x0081, "Int. Voice"  );

    periph_register    (P(gfx            ),  0x0000, 0x0000, "[Graphics]"  );
    if (top_intv.audio_rate)               
        periph_register(P(snd            ),  0x0000, 0x0000, "[Sound]"     );
                                       
    periph_register    (P(scr_ram        ),  0x0100, 0x01EF, "Scratch RAM" );
    periph_register    (P(sys_ram        ),  0x0200, 0x035F, "System RAM"  );

    if (exec_type != 0)
        periph_register(P(sys_ram2       ),  0x0360, 0x03FF, "System RAM B");

    if (exec_type == 0)                
    {                                  
        periph_register(P(exec           ),  0x1000, 0x1FFF, "EXEC ROM"    );
    } else                             
    {                                  
        periph_register(P(exec           ),  0x1000, 0x1FFF, "EXEC2 main"  );
        periph_register(P(exec2          ),  0x0400, 0x04FF, "EXEC2 aux."  );
    }                                  

    if (top_intv.ecs_enable > 0)
    {
        periph_register(P(ecs_ram        ),  0x4000, 0x47FF, "ECS RAM" );
        periph_register(P(ecs0           ),  0x2000, 0x2FFF, "ECS ROM (2xxx)");
        periph_register(P(ecs1           ),  0x7000, 0x7FFF, "ECS ROM (7xxx)");
        periph_register(P(ecs2           ),  0xE000, 0xEFFF, "ECS ROM (Exxx)");
    }
                                       
/*  periph_register    (P(grom           ),  0x3000, 0x37FF, "GROM"        );*/
/*  periph_register    (P(gram           ),  0x3800, 0x3FFF, "GRAM"        );*/

    periph_register    (P(pad0           ),  0x01F0, 0x01FF, "Pad Pair 0"  );
    if (top_intv.ecs_enable > 0)
        periph_register(P(pad1           ),  0x00F0, 0x00FF, "Pad Pair 1"  );
    if (top_intv.i2pc0_port)
        periph_register(P(i2pc0          ),  0x01F0, 0x01FF, "INTV2PC #0"  );
    if (top_intv.i2pc1_port && top_intv.ecs_enable > 0)
        periph_register(P(i2pc1          ),  0x00F0, 0x00FF, "INTV2PC #1"  );
    if (top_intv.cgc0_num >= 0)
        periph_register(P(cgc0           ),  0x01F0, 0x01FF, "CGC #0"      );
    if (top_intv.cgc1_num >= 0 && top_intv.ecs_enable > 0)
        periph_register(P(cgc1           ),  0x00F0, 0x00FF, "CGC #1"      );
    periph_register    (P(stic.stic_cr   ),  0x0000, 0x007F, "STIC"        );
/*  periph_register    (P(stic.stic_cr   ),  0x4000, 0x403F, "STIC (alias)");*/
/*  periph_register    (P(stic.stic_cr   ),  0x8000, 0x803F, "STIC (alias)");*/
/*  periph_register    (P(stic.stic_cr   ),  0xC000, 0xC03F, "STIC (alias)");*/
    periph_register    (P(stic.snoop_btab),  0x0200, 0x02EF, "STIC (BTAB)" );
    periph_register    (P(stic.snoop_gram),  0x3000, 0x3FFF, "STIC (GRAM)" );

    periph_register    (P(event          ),  0x0000, 0x0000, "[Event]"     );

    if (top_intv.rate_ctl)
        periph_register(P(speed          ),  0x0000, 0x0000, "[Rate Ctrl]" );

    /* -------------------------------------------------------------------- */
    /*  Register the game ROMs, or the Intellicart, as the case may be.     */
    /* -------------------------------------------------------------------- */
    if (legacy_rom)
    {
        legacy_register(&top_intv.legacy, top_intv.intv);
    } else
    {
        icart_register(&top_intv.icart, top_intv.intv, &top_intv.cp1600, cache_flags);
    }

    /* -------------------------------------------------------------------- */
    /*  Mark the ROMs cacheable in the CPU.  Mark the 16-bit RAM as cache-  */
    /*  able, but in need of bus-snoop support.                             */
    /* -------------------------------------------------------------------- */
    cp1600_cacheable(&top_intv.cp1600, 0x0200, 0x035F, 1);
    cp1600_cacheable(&top_intv.cp1600, 0x1000, 0x1FFF, 0);
    cp1600_cacheable(&top_intv.cp1600, 0x3000, 0x37FF, 0);

    /* -------------------------------------------------------------------- */
    /*  Register the debugger.  This _must_ be done last.                   */
    /* -------------------------------------------------------------------- */
    if (top_intv.debugging)
        periph_register(P(debug          ),  0x0000, 0xFFFF, "[Debugger]"  );


#if 0
    {
        f = fopen("ser.txt", "w");
        if (f)
            ser_print_hierarchy(f, NULL, 0, 0);
    }
#endif

    return &top_intv;
}

extern bc_cfgfile_t *bc_parsed_cfg;
int
cfg_load_bin(char *filename)
{
  FILE *f;
  char *tmp;
  int legacy_rom = 0; 

  top_intv.fn_game = strdup(filename);
  bc_parsed_cfg = NULL;
  tmp = legacy_bincfg(&(top_intv.legacy), top_intv.fn_game);

  if (tmp == NULL) return 1;

  if (tmp == top_intv.fn_game)
  {
    legacy_rom = 1;
  }
  else
  {
    top_intv.fn_game = tmp;
  }

  if (!legacy_rom)
  {
    f = fopen(top_intv.fn_game, "rb");
    if (f == NULL) return 1;

    /* ---------------------------------------------------------------- */
    /*  Process the Intellicart ROM itself.                             */
    /* ---------------------------------------------------------------- */
    if (icart_init(&top_intv.icart, f, NULL))
    {
      fclose(f);
      return 1;
    }

    /* ---------------------------------------------------------------- */
    /*  TODO:  Process meta-data tags on Intellicart image.             */
    /* ---------------------------------------------------------------- */
    fclose(f);
  }

//LUDO:
    /* -------------------------------------------------------------------- */
    /*  Register the game ROMs, or the Intellicart, as the case may be.     */
    /* -------------------------------------------------------------------- */
    if (legacy_rom)
    {
        legacy_register(&top_intv.legacy, top_intv.intv);
    } else
    {
        icart_register(&top_intv.icart, top_intv.intv, &top_intv.cp1600, IC_CACHE_DFLT);
    }

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

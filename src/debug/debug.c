/*
 * ============================================================================
 *  Title:    Debugger
 *  Author:   J. Zbiciak
 *  $Id: debug.c,v 1.29 2001/11/02 02:00:03 im14u2c Exp $
 * ============================================================================
 *  
 * ============================================================================
 *  This module will implement the debugger as a combination of interactive
 *  interface and a peripheral which maps into the entire address space
 *  which allows capturing memory events.
 * ============================================================================
 */

static const char rcs_id[]="$Id: debug.c,v 1.29 2001/11/02 02:00:03 im14u2c Exp $";

#include "config.h"
#include "periph/periph.h"
#include "cp1600/cp1600.h"
#include "cp1600/op_decode.h"
#include "gfx/gfx.h"
#include "speed/speed.h"
#include "debug_.h"
#include "debug_dasm1600.h"

#define HISTSIZE (0x10000)
#define HISTMASK (HISTSIZE-1)
#define RH_RECSIZE (16)

void debug_write_memattr(char *fname);
uint_32 debug_peek(periph_t *p, periph_t *r, uint_32 a, uint_32 d);
void debug_poke(periph_t *p, periph_t *r, uint_32 a, uint_32 d);

LOCAL int dc_hits = 0, dc_miss = 0, dc_nocache = 0; 
LOCAL int dc_unhook_ok = 0, dc_unhook_odd = 0;

#define DEBUG_MA_DATA  (1 << 0)
#define DEBUG_MA_SDBD  (1 << 1)
#define DEBUG_MA_CODE  (1 << 2)
#define DEBUG_MA_WRITE (1 << 3)

LOCAL uint_16 *debug_reghist = NULL;
LOCAL uint_32 *debug_profile = NULL;
LOCAL uint_8  *debug_memattr = NULL;
LOCAL uint_16 *debug_mempc   = NULL;
LOCAL int      debug_rh_ptr = -1;
LOCAL int      debug_histinit = 0;
LOCAL void debug_write_reghist(char *, periph_p, cp1600_t *);

LOCAL uint_32 debug_watch[0x10000 >> 5];
#define WATCHING(x) ((int)((debug_watch[(x) >> 5] >> ((x) & 31)) & 1))
#define WATCHTOG(x) ((debug_watch[(x) >> 5] ^= 1 << ((x) & 31)))

int debug_fault_detected = 0;

/*
 * ============================================================================
 *  DEBUG_RD         -- Capture/print a read event.
 *  DEBUG_WR         -- Capture/print a write event.
 *  DEBUG_TK         -- Debugger 'tick' function.  This where the user command
 *                      line called from.
 * ============================================================================
 */
uint_32 debug_rd(periph_t *p, periph_t *r, uint_32 a, uint_32 d)
{
    debug_t *debug = (debug_t*)p;
    cp1600_t *cp = debug->cp1600;

    if ((debug->show_rd/* || WATCHING(a)*/) && p != r->req)
    {
        printf(" RD a=%.4X d=%.4X %-16s (PC = $%.4X) t=%llu\n", 
                a, d, r==(periph_p)p->bus ? r->req->name:r->name, 
                cp->oldpc, cp->periph.now);
    }

    if (debug_memattr)
    {
        if (cp->r[7] == a) 
            debug_memattr[a] |= DEBUG_MA_CODE;
        else if (cp->D)
            debug_memattr[a] |= DEBUG_MA_SDBD;
        else
            debug_memattr[a] |= DEBUG_MA_DATA;

        debug_mempc[a]  = cp->r[7];
    }

    return ~0U;
}

void    debug_wr(periph_t *p, periph_t *r, uint_32 a, uint_32 d)
{
    debug_t *debug = (debug_t*)p;
    cp1600_t *cp = debug->cp1600;

    if ((debug->show_wr || WATCHING(a)) && p != r->req)
    {
        printf(" WR a=%.4X d=%.4X %-16s (PC = $%.4X) t=%llu\n", 
                a, d, r==(periph_p)p->bus ? r->req->name:r->name, 
                cp->oldpc, cp->periph.now);
    }

    if (debug_memattr)
    {
        debug_memattr[a] |= DEBUG_MA_DATA | DEBUG_MA_WRITE;
        debug_mempc  [a]  = cp->r[7];
    }
}

uint_32 debug_peek(periph_t *p, periph_t *r, uint_32 a, uint_32 d)
{
    (void)p;
    (void)r;
    (void)a;
    (void)d;
    return ~0U;
}

void debug_poke(periph_t *p, periph_t *r, uint_32 a, uint_32 d)
{
    (void)p;
    (void)r;
    (void)a;
    (void)d;
}


uint_32 debug_tk(periph_t *p, uint_32 len)
{
    debug_t *debug = (debug_t*)p;
    cp1600_t *cp = debug->cp1600;
    uint_32 pc = cp->r[7];
    char    buf[1024], *s;
    static  uint_16 next_hex_dump = 0x0000;
    static  int     prev_pc = ~0U;
    static  int     non_int = 0;
    sint_32 slen = (sint_32)len;
    uint_64 instrs = cp->tot_instr - debug->tot_instr;
	uint_64 now = cp->periph.now;
    uint_16 next_disassem = pc;
	int     intrq;
	//static req_bus_t old_req = { 0, 0, 0, 0, 0, 0, 0 };
	//static uint_64 old_now = 0;

    debug->tot_instr = cp->tot_instr;
	intrq = cp->req_bus.intrq & 3;
	/* dequalify existing intrq/busrq */
	if (now > cp->req_bus.intrq_until)  intrq &= 2;
	if (now > cp->req_bus.busrq_until)  intrq &= 1;
	/* prequalify pending intrq/busrq */
	if (now > cp->req_bus.next_intrq)   intrq |= 1;
	if (now > cp->req_bus.next_busrq)   intrq |= 2;


	if (now == cp->req_bus.intak)       intrq = 4;
	if (now >  cp->req_bus.busak &&
	    now == cp->req_bus.busrq_until) intrq = 5;

#if 0
	if (intrq > 3)
	{
	    printf("prev:  n=%-8llu ni=%-8llu nb=%-8llu\n"
	         "                  iu=%-8llu bu=%-8llu\n"
	         "                   i=%-8llu  b=%-8llu\n", 
	           old_now, 
	           old_req.next_intrq,
	           old_req.next_busrq,
	           old_req.intrq_until,
	           old_req.busrq_until,
	           old_req.intak,
	           old_req.busak);
	}
	old_req = cp->req_bus;
	old_now = now;
#endif


//printf("%15llu %15llu %15llu %d\n", now, cp->req_bus.intak, cp->req_bus.busak, intrq);
    /* -------------------------------------------------------------------- */
    /*  If we're keeping a trace history, update it now.                    */
    /* -------------------------------------------------------------------- */
    if (debug_rh_ptr >= 0)
    {
        int i;

        if (slen > 0 && prev_pc > 0 && cp->intr == 0) 
        { 
            non_int += len; 
        } else 
        { 
            /* FIXME:  Only print if happens during active display. */
            if (0 && non_int > 45)
                printf("NON_INT = %d at PC = %.4X\n", non_int, pc);
            
            non_int = 0;
        }

        memcpy(debug_reghist + debug_rh_ptr * RH_RECSIZE, cp->r, 16);


        debug_reghist[debug_rh_ptr * RH_RECSIZE + 8] = 1 +
                                              ((!!cp->S    ) << 1) + 
                                              ((!!cp->C    ) << 2) +
                                              ((!!cp->O    ) << 3) +
                                              ((!!cp->Z    ) << 4) +
                                              ((!!cp->I    ) << 5) +
                                              ((!!cp->D    ) << 6) +
                                              ((!!cp->intr ) << 7) +
                                              ((intrq      ) << 8);

        for (i = 0; i < 3; i++)
            debug_reghist[debug_rh_ptr * RH_RECSIZE + 9 + i] = 
                periph_peek((periph_t*)p->bus, p, pc + i, ~0);

        debug_reghist[debug_rh_ptr * RH_RECSIZE + 12] = (now    ) & 0xFFFF;
        debug_reghist[debug_rh_ptr * RH_RECSIZE + 13] = (now>>16) & 0xFFFF;
        debug_reghist[debug_rh_ptr * RH_RECSIZE + 14] = (now>>32) & 0xFFFF;
        debug_reghist[debug_rh_ptr * RH_RECSIZE + 15] = (now>>48) & 0xFFFF;

        debug_rh_ptr = (debug_rh_ptr + 1) & HISTMASK;

        if (slen > 0 && prev_pc > 0)
        {
            debug_profile[prev_pc] += len * 2;
            debug_profile[pc]      |= !!cp->D;
        }
    }
    prev_pc = pc;

    /* -------------------------------------------------------------------- */
    /*  If slen == -INT_MAX, we're crashing.                                */
    /* -------------------------------------------------------------------- */
    if (slen == -INT_MAX || debug_fault_detected)
    {   
        if (debug_rh_ptr >= 0)  debug_write_reghist("dump.hst", p, cp);
        if (debug_memattr)      debug_write_memattr("dump.atr");
        printf("CPU crashed!\n");
        debug_fault_detected = 0;
        slen = -INT_MAX;
    }

    /* -------------------------------------------------------------------- */
    /*  Short-circuit the debugger if we're not doing anything interesting  */
    /* -------------------------------------------------------------------- */
    if (debug->step_count < 0 && slen > 0 && !debug->show_ins)
        return len;

    /* -------------------------------------------------------------------- */
    /*  Decrement our CPU step count.                                       */
    /* -------------------------------------------------------------------- */
    if (debug->step_count > 0)
    {
        debug->step_count -= instrs;
        if (debug->step_count < 0)
            debug->step_count = 0;
    }

    /* -------------------------------------------------------------------- */
    /*  If this was a breakpoint, the cycle count will be negative.         */
    /*  Drop into debug mode on a breakpoint.                               */
    /* -------------------------------------------------------------------- */
    if (slen < 0)
        debug->step_count = 0;

    /* -------------------------------------------------------------------- */
    /*  If we're in sync with the CPU, then grab the current PC and words   */
    /*  at that location, disassemble and display.                          */
    /* -------------------------------------------------------------------- */
    if (debug->show_ins || !debug->step_count /*|| !len*/)
    {
        char *dis;

show_disassem:

        pc = cp->r[7];

        /* ---------------------------------------------------------------- */
        /*  Where possible, cache disassembled instructions for speed.      */
        /* ---------------------------------------------------------------- */
        dis = debug_disasm(p, cp, pc, NULL, cp->D);
        dis = dis ? dis + 22 : "(null)";

        /* ---------------------------------------------------------------- */
        /*  Print the state of the machine, along w/ disassembly.           */
        /* ---------------------------------------------------------------- */
        printf(" %.4X %.4X %.4X %.4X %.4X %.4X %.4x %.4X %c%c%c%c%c%c%c%c"
               "%-20.20s %8llu\n",
               cp->r[0], cp->r[1], cp->r[2], cp->r[3], 
               cp->r[4], cp->r[5], cp->r[6], cp->r[7],
               cp->S ? 'S' : '-',
               cp->C ? 'C' : '-',
               cp->O ? 'O' : '-',
               cp->Z ? 'Z' : '-',
               cp->I ? 'I' : '-',
               cp->D ? 'D' : '-',
               cp->intr ? 'i' : '-',
               intrq == 1 ? 'q' : 
               intrq == 2 ? 'b' : 
               intrq == 3 ? '?' :
               intrq == 4 ? 'Q' : 
               intrq == 5 ? 'B' : '-', dis,
               now);

        if (debug->speed) speed_resync(debug->speed);
    }

    /* -------------------------------------------------------------------- */
    /*  If we've exhausted our CPU step count, drop into command prompt.    */
    /* -------------------------------------------------------------------- */
    if (debug->step_count == 0 /*|| !len*/)
    {
        int cmd = 0, c, arg = -1, arg2;
        static int over = 0;
        extern void dump_state(void);

next_cmd:
        do
        {
            printf("> ");
            fflush(stdout);
            fgets(buf, 1023, stdin);
            s = buf;

            if (buf[0] == '\n')
            {
                cmd = arg = 1;
            }

            while (*s && isspace(*s)) s++;

            if (!*s) continue;

            c = toupper(*s);

            if (c == 'S') cmd = 1, over = 0;  /* step into */
            if (c == 'T') cmd = 2, over = 1;  /* trace over */
            if (c == 'R') cmd = 2, over = 0;  /* run  */
            if (c == 'D') cmd = 3;  /* dump/abort */
            if (c == 'Q') cmd = 4;  /* quit */
            if (c == 'B') cmd = 5;  /* breakpoint */
            if (c == 'M') cmd = 6;  /* mem dump */
            if (c == 'U') cmd = 7;  /* 'Un'assemble.            */
            if (c == 'C') cmd = 8;  /* Disassembly Cache stats. */
            if (c == 'G') cmd = 9;  /* Change re'G'ister value. */
            if (c == 'N') cmd = 10; /* uNset breakpoint */
            if (c == 'H') cmd = 11; /* toggle History */
            if (c == 'W') cmd = 12; /* toggle watch */
            if (c == 'A') cmd = 13; /* enable memory Attribute discovery */
            if (c == 'P') cmd = 14; /* enable memory Attribute discovery */
            if (c == 'E') cmd = 15; /* enable memory Attribute discovery */
            if (cmd < 3)
            {
                if (sscanf(s+1, "%d", &arg) != 1) 
                    arg=-1;
            } else if (cmd == 5 || cmd == 10) {
                if (sscanf(s+1, "%x", &arg) != 1) 
                    arg=-2;
            } else if (cmd == 6 || cmd == 7) {
                int args = sscanf(s+1, "%x %x", &arg, &arg2);

                if (args < 1) arg  = cmd == 6 ? next_hex_dump : next_disassem;
                if (args < 2) arg2 = cmd == 6 ? 0x40          : 0x10;

                if (cmd == 7)
                    next_disassem = arg;

                if (cmd == 6)
                    next_hex_dump = arg + arg2;
            } else if (cmd == 9) {
                int args = sscanf(s+1, "%x %x", &arg, &arg2);

                if (args != 2 || arg < 0 || arg > 7) arg = -2;
            } else if (cmd == 12) {
                int args = sscanf(s+1, "%x %x", &arg, &arg2);

                if (args == 1) arg2 = arg;
                if (arg2 < arg) { int tmp = arg2; arg2 = arg; arg = tmp; }
            } else if (cmd == 14 || cmd == 15) {
                int args = sscanf(s+1, "%x %x", &arg, &arg2);

                if (args != 2 || arg < 0x0000 || arg > 0xFFFF) arg = -2;
            } else 
                arg=0;
        } while (!cmd || arg < -1);

        switch (cmd)
        {   
            case 1: 
            case 2:
                if (over)
                {
                    uint_16 next;

                    /* Is this a JUMP family instruction? */
                    pc = cp->r[7];
                    next = periph_peek((periph_t*)p->bus, p, pc, ~0) & 0x3FF;
                    if (next == 0x0004)
                    {
                        cmd = 2; /* run until tracepoint*/
                        cp1600_set_tracept(cp, pc + 3);
                        arg = -1;
                    } else
                        cmd = arg = 1; /* step 1 */
                }

                debug->step_count = arg;
                debug->show_ins = debug->show_rd = debug->show_wr = cmd == 1;
                if (debug_rh_ptr < 0)
                    cp->instr_tick_per = cmd == 1 ? 1 : arg > 0 ? arg : 0;
                    
                break;
            case 3: 
                dump_state();
                if (debug_rh_ptr >= 0)
                    debug_write_reghist("dump.hst", p, cp);
                if (debug_memattr)
                    debug_write_memattr("dump.atr");
                break;
            case 4:
                exit(0);
                break;
            case 5:
            case 10:
            {
                int set = cmd == 5;
                if (set)
                    cp1600_set_breakpt(cp, arg);
                else
                    cp1600_clr_breakpt(cp, arg);
                printf("%s breakpoint at $%.4X\n", set ? "Set" : "Unset", arg);
                goto next_cmd;
                break;
            }
            case 6:
                debug_dispmem(p, arg, arg2);
                goto next_cmd;
                break;
            case 7:
                debug_disasm_mem(p, cp, &next_disassem, arg2);
                goto next_cmd;
                break;
            case 8:
                printf("dc hits: %6d  misses: %6d  nocache: %6d  "
                       "unhook: %6d vs %6d\n", dc_hits, dc_miss, dc_nocache,
                       dc_unhook_ok, dc_unhook_odd);
                goto next_cmd;
                break;
            case 9:
                cp->r[arg] = arg2;
                goto show_disassem;
                break;
            case 11:
                if (!debug_histinit)
                {
                    debug_reghist = calloc(HISTSIZE + 1, 
                                           RH_RECSIZE * sizeof(uint_16));
                    debug_profile = calloc(0x10000, sizeof(uint_32));
                    if (!debug_reghist || !debug_profile)
                    {
                        printf("Couldn't allocate register history\n");
                    } else
                    {
                        debug_histinit = 1;
                    }
                }
                if (debug_histinit)
                {
                    if (debug_rh_ptr < 0) debug_rh_ptr = 0;
                    else                  debug_rh_ptr = -1;
                    printf("Register History is %s\n", 
                            debug_rh_ptr ? "Off" : "On");
                    if (!debug_rh_ptr)
                    {
                        memset(debug_reghist, 0, 
                               RH_RECSIZE*sizeof(uint_16)*HISTSIZE);
                        memset(debug_profile, 0, 0x10000 * sizeof(uint_32));
                    }
                    cp->instr_tick_per = debug_rh_ptr ? 0 : 1;
                }
                fflush(stdout);
                goto next_cmd;
                break;
            case 12:
                {
                    int i, watch;

                    for (i = arg; i <= arg2; i++)
                    {
                        WATCHTOG(i);
                    }

                    for (i = arg, watch=-1; i <= arg2; i++)
                    {
                        if (WATCHING(i) != watch)
                        {
                            if (watch != -1) 
                                printf(" through $%.4X\n", i - 1);
                            watch = WATCHING(i);
                            printf("%s watching $%.4X", 
                                   watch ? "Now" : "No longer", i);
                        }
                    }
                    if (arg2 >= arg)
                    {
                        printf(arg == arg2?"\n":" through $%.4X\n", arg2);
                    }
                }
                goto next_cmd;
                break;
            case 13:
                if (!debug_memattr)
                {
                    debug_memattr = calloc(0x10000, sizeof(uint_8));
                    debug_mempc   = calloc(0x10000, sizeof(uint_16));
                    if (!debug_memattr || ! debug_mempc)
                    {
                        printf("Couldn't allocate memory attribute map\n");
                    } else
                    {
                        printf("Memory attribute map enabled.\n");
                    }
                }
                goto next_cmd;
            case 14: /* poke */
            {
                if (arg >= 0)
                    periph_poke((periph_t*)p->bus, p, arg, arg2);
                else
                    printf("invalid args.  P addr data\n");

                goto next_cmd;
            }
            case 15: /* enter */
            {
                if (arg >= 0)
                    periph_write((periph_t*)p->bus, p, arg, arg2);
                else
                    printf("invalid args.  E addr data\n");

                goto next_cmd;
            }
        }

        if (debug->speed) speed_resync(debug->speed);
    }

    return len;
}



/*
 * ============================================================================
 *  Instruction Disassembly Cache
 *
 *  This holds a cache of disassembled instructions.  The cache is maintained
 *  as an LRU list of entries.  We place an upper bound on the number of 
 *  entries just so we don't totally thrash through memory.
 * ============================================================================
 */
#define DISASM_CACHE (64)      /* Cache size. */

typedef struct disasm_cache_t
{
    char                    disasm[48];
    uint_32                 len;
    struct disasm_cache_t   *next,*prev;
    char                    **hook;
} disasm_cache_t;

disasm_cache_t *disasm_cache = NULL;


/*
 * ============================================================================
 *  DEBUG_DISASM      -- Disassembles one instruction, returning a pointer
 *                       to the disassembled text.  Uses the disassembly
 *                       cache if possible.
 * ============================================================================
 */
char * debug_disasm(periph_t *p, cp1600_t *cp, uint_16 addr, 
                    uint_32 *len, int dbd)
{
    static char buf[1024];
    uint_16 w1, w2, w3, pc = addr;
    disasm_cache_t *disasm = NULL;
    int instr_len;

#if 1
    /* -------------------------------------------------------------------- */
    /*  If we can't cache this disassembly, or if it's not disassembled     */
    /*  yet, go do the disassembly.  Also force disassembly if DBD is set   */
    /*  since we might have seen this instr before w/out DBD set.           */
    /* -------------------------------------------------------------------- */
    if (!cp->disasm[pc] || dbd)
    {
        w1 = periph_read((periph_t*)p->bus, p, pc    , ~0);
        w2 = periph_read((periph_t*)p->bus, p, pc + 1, ~0);
        w3 = periph_read((periph_t*)p->bus, p, pc + 2, ~0);

        instr_len = dasm1600(buf, pc, dbd, w1, w2, w3);

        dc_miss++;
        dc_hits--;            /* compensate for dc_hits++ in LRU update */

        /* ------------------------------------------------------------ */
        /*  If DBD is set, or if this instruction hasn't been decoded   */
        /*  yet, don't cache this instruction.                          */
        /* ------------------------------------------------------------ */
        if (dbd || !cp->instr[pc])
        {
            if (len) *len = instr_len;
            return buf + 17;
        }

        /* ------------------------------------------------------------ */
        /*  Allocate a new disassembly record by evicting the LRU.      */
        /* ------------------------------------------------------------ */
        disasm = disasm_cache->prev;        /* Tail of LRU list.        */

        if (disasm->hook &&                 /* Is this already hooked?  */
            *disasm->hook == (char*)disasm) /* Unhook it.               */
        {
            dc_unhook_ok++;
            *disasm->hook = NULL;
        } else
            dc_unhook_odd++;

        /* ------------------------------------------------------------ */
        /*  Hook this disassembly to the instruction record.            */
        /* ------------------------------------------------------------ */
        disasm->hook   = &cp->disasm[pc];
        cp->disasm[pc] = (char*) disasm;

        strncpy(disasm->disasm, buf + 17, sizeof(disasm->disasm));
        disasm->disasm[sizeof(disasm->disasm) - 1] = 0;
        disasm->len = instr_len;
    
    }

    /* -------------------------------------------------------------------- */
    /*  Grab the cached disassembly and update the LRU.                     */
    /* -------------------------------------------------------------------- */
    dc_hits++;
    disasm = (disasm_cache_t *)cp->disasm[pc];

    /* -------------------------------------------------------------------- */
    /*  Move this disasm record to head of the LRU.  Do this by first       */
    /*  unhooking it from the doubly-linked list, and then reinserting      */
    /*  it at the head of the doubly-linked list.                           */
    /* -------------------------------------------------------------------- */
    disasm->next->prev = disasm->prev;      /* Unhook 'next'.           */
    disasm->prev->next = disasm->next;      /* Unhook 'prev'.           */

    disasm->next = disasm_cache->next;      /* Hook us to new 'next'.   */
    disasm->prev = disasm_cache;            /* Hook us to new 'prev'.   */

    disasm->next->prev = disasm;            /* Hook new 'next' to us.   */
    disasm_cache->next = disasm;            /* Hook new 'prev' to us.   */


    /* -------------------------------------------------------------------- */
    /*  Returned cached disassembly.                                        */
    /* -------------------------------------------------------------------- */
    if (len) *len = disasm ? disasm->len : 0;
    return disasm ? disasm->disasm : NULL;
#else
    w1 = periph_read((periph_t*)p->bus, p, pc    , ~0);
    w2 = periph_read((periph_t*)p->bus, p, pc + 1, ~0);
    w3 = periph_read((periph_t*)p->bus, p, pc + 2, ~0);

    instr_len = dasm1600(buf, pc, cp->D, w1, w2, w3);

    if (len) *len = instr_len;

    return buf + 17;
#endif
}

/*
 * ============================================================================
 *  DEBUG_DISASM_MEM  -- Disassembles a range of memory locations.
 * ============================================================================
 */
void debug_disasm_mem(periph_t *p, cp1600_t *cp, uint_16 *paddr, uint_32 cnt)
{
    char * dis;
    uint_32 tot = 0, len = ~0, w0 = 0;
    uint_16 addr = *paddr;

    while (tot <= cnt && len > 0)
    {
        if (len == 1)
            w0 = periph_read((periph_t*)p->bus, p, addr-1, ~0);

        dis = debug_disasm(p, cp, addr, &len, len == 1 && w0 == 0x0001);

        printf("    $%.4X:   %s\n", addr, dis);

        addr += len;
        tot++;
    }

    *paddr = addr;
}

/*
 * ============================================================================
 *  DEBUG_DISPMEM     -- Displays ten lines of "hex dump" memory information.
 *                       The first arg is the address to start dumping at.
 *                       The second arg is the number of addresses to dump.
 * ============================================================================
 */
void debug_dispmem(periph_t *p, uint_16 addr, uint_16 len)
{
    int         i, j, k, l;
    uint_32     w[8];
    
    /* -------------------------------------------------------------------- */
    /*  Round our address down to the next lower multiple of 8.  Add this   */
    /*  difference to the length, and then round the length up to the       */
    /*  next multiple of 8.                                                 */
    /* -------------------------------------------------------------------- */
    k = addr & ~7;

    len = (len + 7 + addr - k) & ~7;
    
    /* -------------------------------------------------------------------- */
    /*  Iterate until we've dumped the entire block of memory.              */
    /* -------------------------------------------------------------------- */
    for (i = 0; i < len; i += 8)
    {
        l = k;
        
        /* ---------------------------------------------------------------- */
        /*  Read one row of 8 memory locations.                             */
        /* ---------------------------------------------------------------- */
        for (j = 0; j < 8; j++)
        {
            w[j] = periph_peek((periph_t*)p->bus, p, k++, ~0);
        }
        
        /* ---------------------------------------------------------------- */
        /*  Display them.                                                   */
        /* ---------------------------------------------------------------- */
        printf( "%.4X:  %.4X %.4X %.4X %.4X  %.4X %.4X %.4X %.4X   # ",
                    l, w[0], w[1], w[2], w[3], w[4], w[5], w[6], w[7] );
        
        /* ---------------------------------------------------------------- */
        /*  Display ASCII equivalents.                                      */
        /* ---------------------------------------------------------------- */
        for (j = 0; j < 8; j++)
        {
            char c, d;
        
            c = w[j] >> 8;
            d = w[j];
            
            if ( !isalnum(c) )
                c = '.';
            if ( !isalnum(d) )
                d = '.';
                
            printf( "%c%c", c, d );
        }
        
        printf( "\n" );
    }
}


/*
 * ============================================================================
 *  DEBUG_WRITE_REGHIST  -- Write CPU register history trace to file.
 * ============================================================================
 */
void debug_write_reghist(char *fname, periph_p p, cp1600_t *cp)
{
    FILE *f;
    int i, idx, S, C, O, Z, I, D, intr, irq, nirq;
    uint_16 pc, flags, r0, r1, r2, r3, r4, r5, r6, w1, w2, w3;
    char *dis;
    char buf[1024];
    double total = 0.0;
    uint_64 now, n0, n1, n2, n3;

    f = fopen(fname, "w");
    if (!f)
    {
        fprintf(stderr, "Could not open register history file '%s'\n", fname);
        return;
    }

    printf("Dumping register history to '%s'...  ", fname);
    fflush(stdout);
    for (i = 0; i < HISTSIZE; i++)
    {
        idx   = ((i + debug_rh_ptr) & HISTMASK) * RH_RECSIZE;
        n3    = debug_reghist[idx + 15];
        n2    = debug_reghist[idx + 14];
        n1    = debug_reghist[idx + 13];
        n0    = debug_reghist[idx + 12];
        now   = (n3 << 48) | (n2 << 32) | (n1 << 16) | n0;
        w3    = debug_reghist[idx + 11];
        w2    = debug_reghist[idx + 10];
        w1    = debug_reghist[idx +  9];
        flags = debug_reghist[idx +  8];
        pc    = debug_reghist[idx +  7];
        r6    = debug_reghist[idx +  6];
        r5    = debug_reghist[idx +  5];
        r4    = debug_reghist[idx +  4];
        r3    = debug_reghist[idx +  3];
        r2    = debug_reghist[idx +  2];
        r1    = debug_reghist[idx +  1];
        r0    = debug_reghist[idx +  0];

        if ((flags & 1) == 0) continue;

        S    = (flags >> 1) & 1;
        C    = (flags >> 2) & 1;
        O    = (flags >> 3) & 1;
        Z    = (flags >> 4) & 1;
        I    = (flags >> 5) & 1;
        D    = (flags >> 6) & 1;
        intr = (flags >> 7) & 1;
        irq  = (flags >> 8) & 7;

        if (i < HISTSIZE)
            nirq = (debug_reghist[idx + 8 + RH_RECSIZE] >> 8) & 7;
		else
		    nirq = 0;

        /* ---------------------------------------------------------------- */
        /*  Where possible, cache disassembled instructions for speed.      */
        /* ---------------------------------------------------------------- */
//      dis = debug_disasm(p, cp, pc, NULL, D);
        dasm1600(buf, pc, D, w1, w2, w3);
        dis = buf + 40;

        if (nirq == 4 || nirq == 5) /* did this one get stomped? */
        {
            dis = buf + 38;
            dis[0] = '>';
            dis[1] = '>';
		}

        /* ---------------------------------------------------------------- */
        /*  Print the state of the machine, along w/ disassembly.           */
        /* ---------------------------------------------------------------- */
        fprintf(f, "%.4X %.4X %.4X %.4X %.4X %.4X %.4x %.4X |%c%c%c%c%c%c%c%c|"
               "%-18.18s %8llu\n", 
               r0, r1, r2, r3, r4, r5, r6, pc,
               S ? 'S' : '-', C ? 'C' : '-', O ? 'O' : '-', Z ? 'Z' : '-',
               I ? 'I' : '-', D ? 'D' : '-', 
               intr ? 'i' : '-', 
               irq == 1 ? 'q' : 
               irq == 2 ? 'b' : 
               irq == 3 ? '?' :
               irq == 4 ? 'Q' :
               irq == 5 ? 'B' : '-', 
               dis, now);
    }
    printf("Done.\n");

    printf("Dumping profile data to '%s'...  ", fname);
    fflush(stdout);

    fprintf(f,"\n\nProfile\n");

    /* -------------------------------------------------------------------- */
    /*  First total up the cycles profiled.                                 */
    /* -------------------------------------------------------------------- */
    for (i = total = 0; i <= 0xFFFF; i++)
        total += (debug_profile[i] >> 1);

    /* -------------------------------------------------------------------- */
    /*  Then output the blow-by-blow.                                       */
    /* -------------------------------------------------------------------- */
    for (i = pc = 0; i <= 0xFFFF; i++, pc++)
    {
        double cycles;

        /* ---------------------------------------------------------------- */
        /*  Skip addresses that were never executed.                        */
        /* ---------------------------------------------------------------- */
        if (!debug_profile[pc]) continue;

        /* ---------------------------------------------------------------- */
        /*  Disassemble those that were.                                    */
        /* ---------------------------------------------------------------- */
        dis = debug_disasm(p, cp, pc, NULL, debug_profile[pc] & 1);
        dis = dis ? dis : "(null)";

        /* ---------------------------------------------------------------- */
        /*  Write the disassembly to the file.                              */
        /* ---------------------------------------------------------------- */
        cycles = debug_profile[pc] >> 1;
        fprintf(f, "%10d (%7.3f%%) | $%.4X:%s\n", 
                (uint_32)cycles, 100.0 * cycles / total, pc, dis);
    }
    printf("Done.\n");

    fflush(stdout);
    fclose(f);
}


/*
 * ============================================================================
 *  DEBUG_WRITE_MEMATTR  -- Write memory attribute map for disassembler
 * ============================================================================
 */
void debug_write_memattr(char *fname)
{
    FILE *f;
    int i, span_s, span_e;
    char *dir = NULL;

    f = fopen(fname, "w");
    if (!f)
    {
        fprintf(stderr, "Could not open memory attribute file '%s'\n", fname);
        return;
    }

    printf("Dumping memory attribute map to '%s'...  ", fname);
    fflush(stdout);
    for (i = 0; i < 0x10000; i = span_e + 1)
    {
        span_s = i;
        for (span_e = span_s; span_e < 0x10000 && 
             debug_memattr[span_e] == debug_memattr[span_s]/* &&
             debug_mempc  [span_e] == debug_mempc  [span_s]*/;
             span_e++)
             ;
        span_e--;

        if (debug_memattr[span_s] == 0)
            continue;

        dir = NULL;
        if        (debug_memattr[span_s] & DEBUG_MA_CODE)
        {
            dir = "code";
        } else if (debug_memattr[span_s] & DEBUG_MA_SDBD)
        {
            dir = "dbdata";
        } else if (debug_memattr[span_s] & DEBUG_MA_DATA)
        {
            dir = "data";
        }

        if (dir)
        {
            /*
            fprintf(f, "%-6s %.4X %.4X ; %.4X\n", dir, span_s, span_e, 
                    debug_mempc[span_s]);
            */
            fprintf(f, "%-6s %.4X %.4X\n", dir, span_s, span_e);
        }
    }

    printf("Done.\n");

    fflush(stdout);
    fclose(f);
}
    

/*
 * ============================================================================
 *  DEBUG_INIT       -- Initializes a debugger object and registers a CPU
 *                      pointer.
 * ============================================================================
 */
int     debug_init(debug_t *debug, cp1600_t *cp1600, speed_t *speed)
{
    /* -------------------------------------------------------------------- */
    /*  Set up the debugger's state.                                        */
    /* -------------------------------------------------------------------- */
    debug->periph.read     = debug_rd;
    debug->periph.write    = debug_wr;
    debug->periph.peek     = debug_peek;
    debug->periph.poke     = debug_poke;
    debug->periph.tick     = NULL;
    debug->periph.min_tick = ~0U;
    debug->periph.max_tick = ~0U;

    debug->periph.addr_base = 0;
    debug->periph.addr_mask = ~0U;

    debug->show_rd = 1;
    debug->show_wr = 1;
    debug->cp1600  = cp1600;
    debug->speed   = speed;

    /* -------------------------------------------------------------------- */
    /*  Register the debugger's tick function with the CPU.  We're ticked   */
    /*  by the CPU, not by the peripheral bus.                              */
    /* -------------------------------------------------------------------- */
    cp1600_instr_tick(cp1600, debug_tk, (periph_p)debug);

    /* -------------------------------------------------------------------- */
    /*  Set up the instruction disassembly cache.                           */
    /* -------------------------------------------------------------------- */
    if (!disasm_cache)
    {
        int i;

        disasm_cache = calloc(sizeof(disasm_cache_t), DISASM_CACHE);
        
        for (i = 0; i < DISASM_CACHE-1; i++)
        {
            disasm_cache[i  ].next = &disasm_cache[i+1];
            disasm_cache[i+1].prev = &disasm_cache[i  ];
        }
        disasm_cache[0].prev = &disasm_cache[DISASM_CACHE-1];
        disasm_cache[DISASM_CACHE-1].next = &disasm_cache[0];
    }

    /* -------------------------------------------------------------------- */
    /*  Clear watch array                                                   */
    /* -------------------------------------------------------------------- */
    memset(debug_watch, 0, sizeof(debug_watch));

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

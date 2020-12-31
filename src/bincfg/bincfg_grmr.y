/* Grammar for parsing CFG files */

/*%defines*/
/*%verbose*/
%token-table
/*%name-prefix="bc_"*/
/*%file-prefix="bincfg/bincfg_grmr"*/
%error-verbose

%{

#include "config.h"
#include "bincfg/bincfg.h"
#include "bincfg/bincfg_lex.h"


static bc_diag_t *bc_diag_list = NULL;
static char bc_errbuf[1024];
static char *bc_cursect = NULL;

static void yyerror(const char *msg);


static bc_diag_t bc_oom_error_struct =
{
    { NULL }, BC_DIAG_ERROR, 0, "<toplevel>", "out of memory!"
};

#define CALLOC(T,n) ((T *)calloc(sizeof(T), n))


/* ------------------------------------------------------------------------ */
/*  BC_OOM_ERROR -- Out of memory error handling.                           */
/*  BC_CHKOOM    -- Check for out-of-memory condition.                      */
/*  BC_DIAG      -- Queue up a diagnostic message.                          */
/*                                                                          */
/*  I hate making these macros, but YYABORT expands to a 'goto' within the  */
/*  yyparse() function.                                                     */
/* ------------------------------------------------------------------------ */
#define BC_OOM_ERROR                                                        \
        do {                                                                \
            bc_oom_error_struct.line = bc_line_no;                          \
            bc_diag_list = &bc_oom_error_struct;                            \
            YYABORT;                                                        \
        } while (0)

#define BC_CHKOOM(m) do { if (!(m)) BC_OOM_ERROR; } while (0)

#define BC_DIAG(diag, section, line_no, diagmsg)                            \
        do {                                                                \
            bc_diag_t *err, **p;                                            \
            int new_lineno = 1;                                             \
                                                                            \
            if (yychar == TOK_ERROR_OOM)                                    \
                BC_OOM_ERROR;                                               \
                                                                            \
            p = &bc_diag_list;                                              \
                                                                            \
            while (*p)                                                      \
            {                                                               \
                if ((*p)->line == line_no) new_lineno = 0;                  \
                p = (bc_diag_t**)&((*p)->l.next);                           \
            }                                                               \
                                                                            \
            if (new_lineno)                                                 \
            {                                                               \
                err = CALLOC(bc_diag_t, 1);                                 \
                BC_CHKOOM(err);                                             \
                                                                            \
                err->line = line_no;                                        \
                err->type = diag;                                           \
                err->sect = strdup(section);                                \
                err->msg  = strdup(diagmsg);                                \
                BC_CHKOOM(err->sect);                                       \
                BC_CHKOOM(err->msg);                                        \
                                                                            \
                *p = err;                                                   \
            }                                                               \
        } while (0)


static int bc_saved_tok = -1, bc_saved_lineno = 0, bc_dont_save = 0;
static char *bc_saved_sec = NULL;

/* Ugh:  bison magic.  Moo. */
#define BC_TOK (bc_saved_tok < 0 || bc_saved_tok == YYEMPTY ? "<unknown>" : \
                yytname[yytranslate[bc_saved_tok]])

#define BC_SEC (bc_saved_sec ? bc_saved_sec : \
                bc_cursect   ? bc_cursect   : "<toplevel>")

#define S(s) bc_cursect = s

%}

%union
{
    int                 intv;
    char                *strv;
    bc_varlike_t        varlike;
    bc_varlike_types_t  varlike_type;
    bc_var_t            *var_list;
    bc_strnum_t         strnum;
    bc_mac_watch_t      mac_watch;
    bc_macro_t          macro;
    bc_macro_t          *macro_list;
    bc_memspan_t        *memspan_list;
    bc_cfgfile_t        *cfgfile;
};

/* ------------------------------------------------------------------------ */
/*  These destructors should only end up getting called if we ignore a      */
/*  token in a grammar.  This could happen along an error path, for         */
/*  example.  The destructors are not strictly necessary in this app.       */
/*  DISABLED FOR NOW -- too much potential for mayhem.                      */
/* ------------------------------------------------------------------------ */
/*
%destructor { free($$); } string
%destructor { ll_free(&($$->vars.l));            } var_list var_rec;
%destructor { ll_free(&($$->vars.l)); free($$);  } varlike;
%destructor { if ($$.str_val) free($$->str_val);    } strnum;
%destructor { free($$.name); free($$.addr);         } watch_list watch_span watch_range watch_addr;
%destructor { bc_free_macro(&$$);                   } macro_line;
%destructor { bc_free_macro_list($$);               } macro_list macro_rec sec_macro;
%destructor { bc_free_memspan_list($$);             } memspan_list
*/


/* ------------------------------------------------------------------------ */
/*  Lots of tokens coming from the lexer.                                   */
/* ------------------------------------------------------------------------ */
%token TOK_SEC_BANKSWITCH
%token TOK_SEC_MAPPING
%token TOK_SEC_MEMATTR
%token TOK_SEC_PRELOAD
%token TOK_SEC_MACRO
%token TOK_SEC_VARS
%token TOK_SEC_JOYSTICK
%token TOK_SEC_KEYS
%token TOK_SEC_CAPSLOCK
%token TOK_SEC_NUMLOCK
%token TOK_SEC_SCROLLLOCK
%token TOK_SEC_DISASM
%token TOK_SEC_VOICES
%token TOK_SEC_UNKNOWN
%token TOK_RAM
%token TOK_ROM
%token TOK_WOM
%token TOK_PAGE
%token TOK_MAC_QUIET
%token TOK_MAC_REG
%token TOK_MAC_AHEAD
%token TOK_MAC_BLANK
%token TOK_MAC_INSPECT
%token TOK_MAC_LOAD
%token TOK_MAC_RUN
%token TOK_MAC_POKE
%token TOK_MAC_RUN
%token TOK_MAC_RUNTO
%token TOK_MAC_TRACE
%token TOK_MAC_VIEW
%token TOK_MAC_WATCH
%token TOK_DECONLY
%token TOK_DEC
%token TOK_HEX
%token TOK_NAME
%token TOK_ERROR_BAD
%token <strv> TOK_ERROR_OOM

%type <intv>            mac_reg hex_num dec_num memattr
%type <strv>            string
%type <varlike>         sec_varlike
%type <varlike_type>    varlike_head
%type <var_list>        var_list var_rec
%type <strnum>          strnum
%type <mac_watch>       watch_list watch_span watch_range watch_addr
%type <macro_list>      macro_list macro_rec sec_macro
%type <macro>           macro_line
%type <memspan_list>    sec_memspan
%type <memspan_list>    sec_mapping mapping_list mapping_rec
%type <memspan_list>    sec_memattr memattr_list memattr_rec
%type <memspan_list>    sec_preload preload_list preload_rec
%type <memspan_list>    sec_banksw  banksw_list  banksw_rec

%start config_file

%%

/* ------------------------------------------------------------------------ */
/*  top-level rule to clean up and grab any accumulated errors/warnings.    */
/* ------------------------------------------------------------------------ */
config_file :   config 
            {
                bc_parsed_cfg->diags = bc_diag_list;
                bc_diag_list  = NULL;
            }
            ;


/* ------------------------------------------------------------------------ */
/*  Config files are lists of sections, with possible lexical errors.       */
/* ------------------------------------------------------------------------ */
config      :   config sec_memspan
            {
                if ($2) 
                    ll_concat((ll_t**)&(bc_parsed_cfg->span), 
                                 &($2->l));
            }
            |   config sec_macro
            {
                if ($2) 
                    ll_concat((ll_t**)&(bc_parsed_cfg->macro), 
                                 &($2->l));
            }
            |   config sec_varlike
            {
                ll_t **list = NULL;

                if ($2.vars)
                {
                    switch ($2.type)
                    {
                        case BC_VL_VARS:
                        {
                            list = (ll_t**)&(bc_parsed_cfg->vars);
                            break;
                        }

                        case BC_VL_JOYSTICK:
                        {
                            list = (ll_t**)&(bc_parsed_cfg->joystick);
                            break;
                        }

                        case BC_VL_KEYS:
                        {
                            list = (ll_t**)&(bc_parsed_cfg->keys[0]);
                            break;
                        }

                        case BC_VL_CAPSLOCK:
                        {
                            list = (ll_t**)&(bc_parsed_cfg->keys[1]);
                            break;
                        }

                        case BC_VL_NUMLOCK:
                        {
                            list = (ll_t**)&(bc_parsed_cfg->keys[2]);
                            break;
                        }

                        case BC_VL_SCROLLLOCK:
                        {
                            list = (ll_t**)&(bc_parsed_cfg->keys[3]);
                            break;
                        }

                        default:
                            /* report the error? */
                            break;
                    }
                }

                if (list)
                {
                    ll_concat(list, &($2.vars->l));
                } else if ($2.vars && !list)
                {
                    /* error to report. */
                    BC_DIAG(BC_DIAG_ERROR, 
                            bc_cursect ? bc_cursect : "<toplevel>", 
                            bc_line_no, 
                            "Internal error processing variable lists");
                }
            }
            |   config sec_unsup  
            |   seed_config       
            |   error_tok
            {
                sprintf(bc_errbuf, "Unexpected token/state '%s'", BC_TOK);
                BC_DIAG(BC_DIAG_ERROR, BC_SEC,
                        bc_saved_lineno, bc_errbuf);
            }
            ;

/* ------------------------------------------------------------------------ */
/*  We support several different section types.                             */
/* ------------------------------------------------------------------------ */
seed_config :   /* empty */
            {
                /* Before the chicken or egg came the rooster */
                if (!bc_parsed_cfg)
                {
                    bc_parsed_cfg = CALLOC(bc_cfgfile_t, 1);
                    BC_CHKOOM(bc_parsed_cfg);
                    bc_cursect = NULL;
                }
            }
            |   eolns
            {
                if (!bc_parsed_cfg)
                {
                    bc_parsed_cfg = CALLOC(bc_cfgfile_t, 1);
                    BC_CHKOOM(bc_parsed_cfg);
                    bc_cursect = NULL;
                }
            }
            ;
            

/* ------------------------------------------------------------------------ */
/*  SEC_MEMSPAN:  All of the memory span-like sections aggregate here.      */
/* ------------------------------------------------------------------------ */
sec_memspan :   sec_banksw
            |   sec_mapping
            |   sec_memattr
            |   sec_preload
            ;

/* ------------------------------------------------------------------------ */
/*  SEC_BANKSW:  BANKSW sections only contain lists of bankswitched addrs.  */
/* ------------------------------------------------------------------------ */
bankswitch  :   TOK_SEC_BANKSWITCH { S("[bankswitch]"); } ;
sec_banksw  :   bankswitch eoln banksw_list
            {
                ll_t *l = &($3->l);
                ll_reverse(&l);
                $$ = (bc_memspan_t *)l;
            }
            ;

banksw_list :   banksw_list banksw_rec
            {
                $$ = (bc_memspan_t*)ll_insert(&($1->l), &($2->l));
            }
            |   banksw_rec         { $$ = $1;   }
            ;

banksw_rec  :   hex_num '-' hex_num eoln
            {
                $$ = CALLOC(bc_memspan_t, 1);
                BC_CHKOOM($$);
                $$->s_fofs = 0;
                $$->e_fofs = 0;
                $$->s_addr = $1;
                $$->e_addr = $3;
                $$->flags  = BC_SPAN_B | BC_SPAN_R;
                $$->width  = 16;
                $$->epage  = BC_SPAN_NOPAGE;
                $$->f_name = NULL;
            }
            |  eoln
            {
                $$ = NULL;
            }
            |   error_tok eoln
            {
                $$ = NULL;
                sprintf(bc_errbuf, "Unexpected token/state '%s'", BC_TOK);
                BC_DIAG(BC_DIAG_ERROR, "[bankswitch]", 
                        bc_saved_lineno, bc_errbuf);
            }
            ;

/* ------------------------------------------------------------------------ */
/*  SEC_MAPPING: MAPPING sections contain file ranges that map to address   */
/*               ranges in the Intellivision and Intellicart memory maps.   */
/* ------------------------------------------------------------------------ */
mapping     :   TOK_SEC_MAPPING { S("[mapping]"); } ;
sec_mapping :   mapping eoln mapping_list
            {
                ll_t *l = &($3->l);
                ll_reverse(&l);
                $$ = (bc_memspan_t *)l;
            }
            ;

mapping_list:   mapping_list mapping_rec
            {
                $$ = (bc_memspan_t*)ll_insert(&($1->l), &($2->l));
            }
            |   mapping_rec { $$ = $1; }
            ;


mapping_rec :   hex_num '-' hex_num '=' hex_num eoln
            {
                $$ = CALLOC(bc_memspan_t, 1);
                BC_CHKOOM($$);
                $$->s_fofs = $1;
                $$->e_fofs = $3;
                $$->s_addr = $5;
                $$->e_addr = $5 + $3 - $1;
                $$->flags  = BC_SPAN_PL | BC_SPAN_R;
                $$->width  = 16;
                $$->epage  = BC_SPAN_NOPAGE;
                $$->f_name = NULL;
            }
            |   hex_num '-' hex_num '=' hex_num TOK_PAGE hex_num eoln
            {
                $$ = CALLOC(bc_memspan_t, 1);
                BC_CHKOOM($$);
                $$->s_fofs = $1;
                $$->e_fofs = $3;
                $$->s_addr = $5;
                $$->e_addr = $5 + $3 - $1;
                $$->flags  = BC_SPAN_PL | BC_SPAN_R | BC_SPAN_EP;
                $$->width  = 16;
                $$->epage  = $7;
                $$->f_name = NULL;
            }
            |   eoln
            {
                $$ = NULL;
            }
            |   error_tok eoln
            {
                $$ = NULL;
                sprintf(bc_errbuf, "Unexpected token/state '%s'", BC_TOK);
                BC_DIAG(BC_DIAG_ERROR, "[mapping]", 
                        bc_saved_lineno, bc_errbuf);
            }
            ;


/* ------------------------------------------------------------------------ */
/*  SEC_MEMATTR: MEMATTR sections define regions of memory as RAM or WOM.   */
/* ------------------------------------------------------------------------ */
memattr     :   TOK_SEC_MEMATTR { S("[memattr]"); } ;
sec_memattr :   memattr eoln memattr_list
            {
                ll_t *l = &($3->l);
                ll_reverse(&l);
                $$ = (bc_memspan_t *)l;
            }
            ;


memattr_list:   memattr_list memattr_rec 
            {
                $$ = (bc_memspan_t*)ll_insert(&($1->l), &($2->l));
            }
            |   memattr_rec { $$ = $1; }
            ;

memattr_rec :   hex_num '-' hex_num '=' memattr dec_num eoln
            {
                $$ = CALLOC(bc_memspan_t, 1);
                BC_CHKOOM($$);
                $$->s_fofs = 0;
                $$->e_fofs = 0;
                $$->s_addr = $1;
                $$->e_addr = $3;
                $$->flags  = $5 | ($6 < 16 ? BC_SPAN_N : 0);
                $$->width  = $6;
                $$->epage  = BC_SPAN_NOPAGE;
                $$->f_name = NULL;
            }
            |   hex_num '-' hex_num eoln
            {
                $$ = CALLOC(bc_memspan_t, 1);
                BC_CHKOOM($$);
                $$->s_fofs = 0;
                $$->e_fofs = 0;
                $$->s_addr = $1;
                $$->e_addr = $3;
                $$->flags  = BC_SPAN_R;
                $$->width  = 16;
                $$->epage  = BC_SPAN_NOPAGE;
                $$->f_name = NULL;
            }
            |   eoln
            {   
                $$ = NULL;
            }
            |   error_tok eoln
            {
                $$ = NULL;
                sprintf(bc_errbuf, "Unexpected token/state '%s'\n", BC_TOK);
                BC_DIAG(BC_DIAG_ERROR, "[memattr]", 
                        bc_saved_lineno, bc_errbuf);
            }
            ;

/* ------------------------------------------------------------------------ */
/*  SEC_PRELOAD: PRELOAD sections initialize Intellicart address ranges     */
/*               without necessarily implying a mapping in Intellivision    */
/*               address space.                                             */
/* ------------------------------------------------------------------------ */
preload     :   TOK_SEC_PRELOAD { S("[preload]"); } ;
sec_preload :   preload eoln preload_list
            {
                ll_t *l = &($3->l);
                ll_reverse(&l);
                $$ = (bc_memspan_t *)l;
            }
            ;

preload_list:   preload_list preload_rec
            {
                $$ = (bc_memspan_t*)ll_insert(&($1->l), &($2->l));
            }
            |   preload_rec { $$ = $1; }
            ;

preload_rec :   hex_num '-' hex_num '=' hex_num eoln
            {
                $$ = CALLOC(bc_memspan_t, 1);
                BC_CHKOOM($$);
                $$->s_fofs = $1;
                $$->e_fofs = $3;
                $$->s_addr = $5;
                $$->e_addr = $5 + $3 - $1;
                $$->flags  = BC_SPAN_PL;
                $$->width  = 16;
                $$->epage  = BC_SPAN_NOPAGE;
                $$->f_name = NULL;
            }
            |   error_tok eoln
            {
                $$ = NULL;
                sprintf(bc_errbuf, "Unexpected token/state '%s'", BC_TOK);
                BC_DIAG(BC_DIAG_ERROR, "[preload]", 
                        bc_saved_lineno, bc_errbuf);
            }
            ;

/* ------------------------------------------------------------------------ */
/*  SEC_MACRO:  MACRO sections define key sequences that INTVPC's built-in  */
/*              debugger would execute.  We implement an ersatz interpreter */
/*              that is sufficient to apply ROM patches.                    */
/*                                                                          */
/*  Several different macro commands are parsed, even if we can't support   */
/*  all of them in the typical case.                                        */
/*                                                                          */
/*    [0-7] <value>             Set register <n> to <value>                 */
/*      A                       Shows the instructions 'A'head.             */
/*      B                       Run until next vertical 'B'lank.            */
/*      I <addr>                'I'nspect data/code at <addr>.              */
/*      L <file> <width> <addr> 'L'oad <file> of <width> at <addr>.         */
/*      P <addr> <data>         'P'oke <data> at <addr>.                    */
/*      R                       'R'un emulation.                            */
/*      T <addr>                'T'race to <addr>.                          */
/*      O <addr>                Run t'O' <addr>.                            */
/*      V                       'V'iew emulation display                    */
/*      W <name> <list>         'W'atch a set of values with label <name>.  */
/*                                                                          */
/* ------------------------------------------------------------------------ */
macro       :   TOK_SEC_MACRO { S("[macro]"); } ;
sec_macro   :   macro eoln macro_list
            {
                ll_t *l = &($3->l);
                ll_reverse(&l);
                $$ = (bc_macro_t *)l;
            }
            ;

macro_list  :   macro_list macro_rec
            {
                $$ = (bc_macro_t*)ll_insert(&($1->l), &($2->l));
            }
            |   macro_rec { $$ = $1; }
            ;



macro_rec   :   TOK_MAC_QUIET macro_line
            {
                if ($2.cmd == BC_MAC_ERROR)
                    $$ = NULL;
                else
                {
                    $$  = CALLOC(bc_macro_t, 1);
                    BC_CHKOOM($$);
                    *$$ = $2;
                    $$->quiet = 1;
                }
            }
            |   macro_line
            {
                if ($1.cmd == BC_MAC_ERROR)
                    $$ = NULL;
                else
                {
                    $$  = CALLOC(bc_macro_t, 1);
                    BC_CHKOOM($$);
                    *$$ = $1;
                    $$->quiet = 0;
                }
            }
            |   TOK_MAC_QUIET eoln
            {
                $$ = NULL;
            }
            |   eoln
            {
                $$ = NULL;
            }
            ;

macro_line  :   mac_reg hex_num eoln
            {
                $$.cmd            = BC_MAC_REG;
                $$.arg.reg.reg    = $1;
                $$.arg.reg.value  = $2;
            }
            |   TOK_MAC_LOAD string dec_num hex_num eoln
            {
                $$.cmd            = BC_MAC_LOAD;
                $$.arg.load.name  = $2;
                $$.arg.load.width = $3;
                $$.arg.load.addr  = $4;
            }
            |   TOK_MAC_WATCH string watch_list eoln
            {
                $$.cmd            = BC_MAC_WATCH;
                $$.arg.watch      = $3;
                $$.arg.watch.name = $2;
            }
            |   TOK_MAC_POKE hex_num hex_num eoln
            {
                $$.cmd = BC_MAC_POKE;
                $$.arg.poke.addr  = $2;
                $$.arg.poke.value = $3;
            }
            |   TOK_MAC_INSPECT hex_num eoln
            {
                $$.cmd = BC_MAC_INSPECT;
                $$.arg.inspect.addr = $2;
            }
            |   TOK_MAC_RUNTO hex_num eoln
            {
                $$.cmd = BC_MAC_RUNTO;
                $$.arg.runto.addr = $2;
            }
            |   TOK_MAC_TRACE hex_num eoln
            {
                $$.cmd = BC_MAC_TRACE;
                $$.arg.runto.addr = $2;
            }
            |   TOK_MAC_AHEAD eoln  {   $$.cmd  = BC_MAC_AHEAD;     }
            |   TOK_MAC_BLANK eoln  {   $$.cmd  = BC_MAC_BLANK;     }
            |   TOK_MAC_RUN   eoln  {   $$.cmd  = BC_MAC_RUN;       }
            |   TOK_MAC_VIEW  eoln  {   $$.cmd  = BC_MAC_VIEW;      }
            |   error_tok eoln
            {
                $$.cmd = BC_MAC_ERROR;
                sprintf(bc_errbuf, "Unexpected token/state '%s'", BC_TOK);
                BC_DIAG(BC_DIAG_WARNING, "[macro]", 
                        bc_saved_lineno, bc_errbuf);
            }
            ;

mac_reg     :   TOK_MAC_REG { $$ = bc_hex; } 
            ;

/* ------------------------------------------------------------------------ */
/*  WATCH_LIST:  These states assemble a watch list.  It's kinda messy.     */
/* ------------------------------------------------------------------------ */
watch_list  :   watch_list ',' watch_span
            {
                size_t new_size;
                int new_spans, i;

                new_spans = $1.spans + $3.spans;
                new_size  = new_spans*sizeof(uint_16)*2;

                $$.addr  = (uint_16 *)realloc($1.addr, new_size);
                $$.spans = new_spans;
                BC_CHKOOM($$.addr);

                for (i = 0; i < $3.spans*2; i++)
                    $$.addr[i + 2*$1.spans] = $3.addr[i];
            }
            |   watch_span  { $$ = $1; }
            ;

watch_span  :   watch_addr
            |   watch_range
            ;

watch_addr  :   hex_num
            {
                $$.spans   = 1;
                $$.addr    = CALLOC(uint_16, 2); BC_CHKOOM($$.addr);
                $$.addr[0] = $1;
                $$.addr[1] = $1;
            }
            ;

watch_range :   hex_num '-' hex_num
            {
                $$.spans   = 1;
                $$.addr    = CALLOC(uint_16, 2); BC_CHKOOM($$.addr);
                $$.addr[0] = $1;
                $$.addr[1] = $3;
            }
            ;

/* ------------------------------------------------------------------------ */
/*  SEC_VARLIKE:  Captures all the "foo = bar" sections.  This includes     */
/*                the variable definitions and the key mapping sections.    */
/*                                                                          */
/*  VARLIKE_HEAD tracks what section type this actually is.  The list that  */
/*  we produce in of the form of "name = value" tuples.  The names are      */
/*  always interpreted as strings.  The values are semi-lazily interpreted: */
/*  we keep track of their string interpretation, as well as their numeric  */
/*  values, if they have any.  It's up to the recipient of the data to      */
/*  decide if the right data types are being handed about.                  */
/* ------------------------------------------------------------------------ */
sec_varlike :   varlike_head eoln var_list
            {
                ll_t *l = (ll_t *)$3;
                ll_reverse(&(l));
                $$.type = $1;
                $$.vars = (bc_var_t*)l;
            }
            ;

varlike_head:   TOK_SEC_VARS       { $$=BC_VL_VARS;       S("[vars]"      ); }
            |   TOK_SEC_JOYSTICK   { $$=BC_VL_JOYSTICK;   S("[joystick]"  ); }
            |   TOK_SEC_KEYS       { $$=BC_VL_KEYS;       S("[keys]"      ); }
            |   TOK_SEC_CAPSLOCK   { $$=BC_VL_CAPSLOCK;   S("[capslock]"  ); }
            |   TOK_SEC_NUMLOCK    { $$=BC_VL_NUMLOCK;    S("[numlock]"   ); }
            |   TOK_SEC_SCROLLLOCK { $$=BC_VL_SCROLLLOCK; S("[scrolllock]"); }
            ;

var_list    :   var_list var_rec
            {
                $$ = (bc_var_t*)ll_insert(&$1->l, &$2->l);
            }
            |   var_rec { $$ = $1; }
            ;

var_rec     :   string '=' strnum eoln
            {
                $$ = CALLOC(bc_var_t, 1);
                BC_CHKOOM($$);

                $$->name = $1;
                $$->val  = $3;
            }
            |   eoln { $$ = NULL; }
            |   error_tok eoln
            {
                $$ = NULL;
                sprintf(bc_errbuf, "Unexpected token/state '%s'\n", BC_TOK);
                BC_DIAG(BC_DIAG_WARNING, "[vars]", 
                        bc_saved_lineno, bc_errbuf);
            }
            ;

/* ------------------------------------------------------------------------ */
/*  SEC_UNSUP:  Clean up the remaining section types that we might have     */
/*              heard of, but don't do anything valueable with.             */
/* ------------------------------------------------------------------------ */
sec_unsup   :   unknown_sec eoln ;

unknown_sec :   TOK_SEC_DISASM   { S("[disasm]"); }
            |   TOK_SEC_VOICES   { S("[voices]"); }
            |   TOK_SEC_UNKNOWN
            {
                static char bc_unknown[256];

                if (bc_txt)
                {
                    strncpy(bc_unknown, bc_txt, 255);
                    bc_unknown[255] = 0;
                }

                S(bc_unknown);
            }
            ;



/* ------------------------------------------------------------------------ */
/*  Helper states.  These are mostly here to deal with ambiguous tokens,    */
/*  or in the case of MEMATTR, to provide a wild card.                      */
/*                                                                          */
/*  STRNUM  :   Token that could be a string or a number -- we won't know   */
/*              until the emulator gets the value.                          */
/*                                                                          */
/*  DEC_NUM :   Token that (by context) is unambiguously a decimal number.  */
/*                                                                          */
/*  HEX_NUM :   Token that (by context) is unambiguously a hex number.      */
/*                                                                          */
/*  STRING  :   Token that (by context) is unambiguously a string.          */
/*                                                                          */
/*  MEMATTR :   RAM, ROM or WOM.                                            */
/*                                                                          */
/*  An example of an ambiguous token would be the text "100".  The lexer    */
/*  returns this as "TOK_DEC."  But, it could be the decimal value 100,     */
/*  the hex value $10, or the string "100", depending on where it is in an  */
/*  operand.  For instance, consider the following three config snippets:   */
/*                                                                          */
/*  [mapping]                                                               */
/*  100 - 0FFF = 5100  ; offset $100 thru $0FFF map to $5100 thru $5FFF.    */
/*                                                                          */
/*  [vars]                                                                  */
/*  handdelay = 100    ; delay for reading hand controllers on LPT.         */
/*  inp = 100          ; read ECS text input from file "100".               */
/* ------------------------------------------------------------------------ */
strnum      :   TOK_NAME
            {
                $$.flag    = BC_VAR_STRING;
                $$.str_val = strdup(bc_txt);  BC_CHKOOM($$.str_val);
                $$.dec_val = 0;
                $$.hex_val = 0;
            }
            |   TOK_HEX
            {
                $$.flag    = BC_VAR_STRING | BC_VAR_HEXNUM;
                $$.str_val = strdup(bc_txt);  BC_CHKOOM($$.str_val);
                $$.dec_val = 0;
                $$.hex_val = bc_hex;
            }
            |   TOK_DEC
            {
                $$.flag    = BC_VAR_STRING | BC_VAR_HEXNUM | BC_VAR_DECNUM;
                $$.str_val = strdup(bc_txt);  BC_CHKOOM($$.str_val);
                $$.dec_val = bc_dec;
                $$.hex_val = bc_hex;
            }
            |   TOK_DECONLY
            {
                $$.flag    = BC_VAR_STRING | BC_VAR_DECNUM;
                $$.str_val = strdup(bc_txt);  BC_CHKOOM($$.str_val);
                $$.dec_val = bc_dec;
                $$.hex_val = 0;
            }
            ;

dec_num     :   TOK_DEC     { $$ = bc_dec; }
            |   TOK_DECONLY { $$ = bc_dec; }
            ;

hex_num     :   TOK_HEX     { $$ = bc_hex; }
            |   TOK_DEC     { $$ = bc_hex; }
            ;

string      :   TOK_NAME    { $$ = strdup(bc_txt); BC_CHKOOM($$); }
            |   TOK_HEX     { $$ = strdup(bc_txt); BC_CHKOOM($$); }
            |   TOK_DEC     { $$ = strdup(bc_txt); BC_CHKOOM($$); }
            |   TOK_DECONLY { $$ = strdup(bc_txt); BC_CHKOOM($$); }
            ;

memattr     :   TOK_RAM     { $$ = BC_SPAN_RAM; }
            |   TOK_ROM     { $$ = BC_SPAN_ROM; }
            |   TOK_WOM     { $$ = BC_SPAN_WOM; }
            ;

eolns       :   eolns eoln
            |   eoln
            ;

eoln        :   '\n'        { bc_dont_save = 0; bc_saved_tok = YYEMPTY; }
            ;

error_tok   :   error
            {
                if (!bc_dont_save)
                {
                    if (yychar > 0 && yychar != YYEMPTY)
                    {
                        bc_dont_save    = 1;
                        bc_saved_tok    = yychar;
                        bc_saved_lineno = bc_line_no;
                        bc_saved_sec    = bc_cursect ? bc_cursect:"<toplevel>";
                    }
                }
            }
            ;
%%

/* ------------------------------------------------------------------------ */
/*  YYERROR -- required by Bison/YACC.  Note that this can leak memory in   */
/*             an OOM condition.  It's an acceptable irony, given we're     */
/*             about to die anyway.                                         */
/* ------------------------------------------------------------------------ */
static void yyerror(const char *diagmsg)
{
    bc_diag_t *err, **p;
    const char *cursect = bc_cursect ? bc_cursect : "<internal>";

    err = CALLOC(bc_diag_t, 1);
    if (!err) return;

    if (bc_txt && yychar == TOK_ERROR_BAD)
        sprintf(bc_errbuf, "%s.  Text at error: '%s'", diagmsg, bc_txt);
    else
        sprintf(bc_errbuf, "%s.", diagmsg);

    err->line = bc_line_no;
    err->type = BC_DIAG_ERROR;
    err->sect = strdup(cursect);    if (!err->sect) return;
    err->msg  = strdup(bc_errbuf);  if (!err->msg ) return;

    p = &bc_diag_list;

    while (*p)
        p = (bc_diag_t**)&((*p)->l.next);

    *p = err;
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
/*                 Copyright (c) 2003-+Inf, Joseph Zbiciak                  */
/* ======================================================================== */

/* A Bison parser, made by GNU Bison 2.0.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* Written by Richard Stallman by simplifying the original so called
   ``semantic'' parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Using locations.  */
#define YYLSP_NEEDED 0

/* Substitute the variable and function names.  */
#define yyparse bc_parse
#define yylex   bc_lex
#define yyerror bc_error
#define yylval  bc_lval
#define yychar  bc_char
#define yydebug bc_debug
#define yynerrs bc_nerrs


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     TOK_SEC_BANKSWITCH = 258,
     TOK_SEC_MAPPING = 259,
     TOK_SEC_MEMATTR = 260,
     TOK_SEC_PRELOAD = 261,
     TOK_SEC_MACRO = 262,
     TOK_SEC_VARS = 263,
     TOK_SEC_JOYSTICK = 264,
     TOK_SEC_KEYS = 265,
     TOK_SEC_CAPSLOCK = 266,
     TOK_SEC_NUMLOCK = 267,
     TOK_SEC_SCROLLLOCK = 268,
     TOK_SEC_DISASM = 269,
     TOK_SEC_VOICES = 270,
     TOK_SEC_UNKNOWN = 271,
     TOK_RAM = 272,
     TOK_ROM = 273,
     TOK_WOM = 274,
     TOK_PAGE = 275,
     TOK_MAC_QUIET = 276,
     TOK_MAC_REG = 277,
     TOK_MAC_AHEAD = 278,
     TOK_MAC_BLANK = 279,
     TOK_MAC_INSPECT = 280,
     TOK_MAC_LOAD = 281,
     TOK_MAC_RUN = 282,
     TOK_MAC_POKE = 283,
     TOK_MAC_RUNTO = 284,
     TOK_MAC_TRACE = 285,
     TOK_MAC_VIEW = 286,
     TOK_MAC_WATCH = 287,
     TOK_DECONLY = 288,
     TOK_DEC = 289,
     TOK_HEX = 290,
     TOK_NAME = 291,
     TOK_ERROR_BAD = 292,
     TOK_ERROR_OOM = 293
   };
#endif
#define TOK_SEC_BANKSWITCH 258
#define TOK_SEC_MAPPING 259
#define TOK_SEC_MEMATTR 260
#define TOK_SEC_PRELOAD 261
#define TOK_SEC_MACRO 262
#define TOK_SEC_VARS 263
#define TOK_SEC_JOYSTICK 264
#define TOK_SEC_KEYS 265
#define TOK_SEC_CAPSLOCK 266
#define TOK_SEC_NUMLOCK 267
#define TOK_SEC_SCROLLLOCK 268
#define TOK_SEC_DISASM 269
#define TOK_SEC_VOICES 270
#define TOK_SEC_UNKNOWN 271
#define TOK_RAM 272
#define TOK_ROM 273
#define TOK_WOM 274
#define TOK_PAGE 275
#define TOK_MAC_QUIET 276
#define TOK_MAC_REG 277
#define TOK_MAC_AHEAD 278
#define TOK_MAC_BLANK 279
#define TOK_MAC_INSPECT 280
#define TOK_MAC_LOAD 281
#define TOK_MAC_RUN 282
#define TOK_MAC_POKE 283
#define TOK_MAC_RUNTO 284
#define TOK_MAC_TRACE 285
#define TOK_MAC_VIEW 286
#define TOK_MAC_WATCH 287
#define TOK_DECONLY 288
#define TOK_DEC 289
#define TOK_HEX 290
#define TOK_NAME 291
#define TOK_ERROR_BAD 292
#define TOK_ERROR_OOM 293




/* Copy the first part of user declarations.  */
#line 10 "bincfg/bincfg_grmr.y"


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



/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 1
#endif

#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
#line 97 "bincfg/bincfg_grmr.y"
typedef union YYSTYPE {
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
} YYSTYPE;
/* Line 190 of yacc.c.  */
#line 260 "bincfg/bincfg_grmr.tab.c"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 213 of yacc.c.  */
#line 272 "bincfg/bincfg_grmr.tab.c"

#if ! defined (yyoverflow) || YYERROR_VERBOSE

# ifndef YYFREE
#  define YYFREE free
# endif
# ifndef YYMALLOC
#  define YYMALLOC malloc
# endif

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   else
#    define YYSTACK_ALLOC alloca
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning. */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
# else
#  if defined (__STDC__) || defined (__cplusplus)
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   define YYSIZE_T size_t
#  endif
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
# endif
#endif /* ! defined (yyoverflow) || YYERROR_VERBOSE */


#if (! defined (yyoverflow) \
     && (! defined (__cplusplus) \
	 || (defined (YYSTYPE_IS_TRIVIAL) && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  short int yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (short int) + sizeof (YYSTYPE))			\
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined (__GNUC__) && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  register YYSIZE_T yyi;		\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (0)
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (0)

#endif

#if defined (__STDC__) || defined (__cplusplus)
   typedef signed char yysigned_char;
#else
   typedef short int yysigned_char;
#endif

/* YYFINAL -- State number of the termination state. */
#define YYFINAL  9
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   485

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  43
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  44
/* YYNRULES -- Number of rules. */
#define YYNRULES  105
/* YYNRULES -- Number of states. */
#define YYNSTATES  174

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   293

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      42,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,    41,    39,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    40,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned short int yyprhs[] =
{
       0,     0,     3,     5,     8,    11,    14,    17,    19,    21,
      22,    24,    26,    28,    30,    32,    34,    38,    41,    43,
      48,    50,    53,    55,    59,    62,    64,    71,    80,    82,
      85,    87,    91,    94,    96,   104,   109,   111,   114,   116,
     120,   123,   125,   132,   135,   137,   141,   144,   146,   149,
     151,   154,   156,   160,   166,   171,   176,   180,   184,   188,
     191,   194,   197,   200,   203,   205,   209,   211,   213,   215,
     217,   221,   225,   227,   229,   231,   233,   235,   237,   240,
     242,   247,   249,   252,   255,   257,   259,   261,   263,   265,
     267,   269,   271,   273,   275,   277,   279,   281,   283,   285,
     287,   289,   291,   294,   296,   298
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const yysigned_char yyrhs[] =
{
      44,     0,    -1,    45,    -1,    45,    47,    -1,    45,    65,
      -1,    45,    74,    -1,    45,    78,    -1,    46,    -1,    86,
      -1,    -1,    84,    -1,    49,    -1,    53,    -1,    57,    -1,
      61,    -1,     3,    -1,    48,    85,    50,    -1,    50,    51,
      -1,    51,    -1,    82,    39,    82,    85,    -1,    85,    -1,
      86,    85,    -1,     4,    -1,    52,    85,    54,    -1,    54,
      55,    -1,    55,    -1,    82,    39,    82,    40,    82,    85,
      -1,    82,    39,    82,    40,    82,    20,    82,    85,    -1,
      85,    -1,    86,    85,    -1,     5,    -1,    56,    85,    58,
      -1,    58,    59,    -1,    59,    -1,    82,    39,    82,    40,
      56,    81,    85,    -1,    82,    39,    82,    85,    -1,    85,
      -1,    86,    85,    -1,     6,    -1,    60,    85,    62,    -1,
      62,    63,    -1,    63,    -1,    82,    39,    82,    40,    82,
      85,    -1,    86,    85,    -1,     7,    -1,    64,    85,    66,
      -1,    66,    67,    -1,    67,    -1,    21,    68,    -1,    68,
      -1,    21,    85,    -1,    85,    -1,    69,    82,    85,    -1,
      26,    83,    81,    82,    85,    -1,    32,    83,    70,    85,
      -1,    28,    82,    82,    85,    -1,    25,    82,    85,    -1,
      29,    82,    85,    -1,    30,    82,    85,    -1,    23,    85,
      -1,    24,    85,    -1,    27,    85,    -1,    31,    85,    -1,
      86,    85,    -1,    22,    -1,    70,    41,    71,    -1,    71,
      -1,    72,    -1,    73,    -1,    82,    -1,    82,    39,    82,
      -1,    75,    85,    76,    -1,     8,    -1,     9,    -1,    10,
      -1,    11,    -1,    12,    -1,    13,    -1,    76,    77,    -1,
      77,    -1,    83,    40,    80,    85,    -1,    85,    -1,    86,
      85,    -1,    79,    85,    -1,    14,    -1,    15,    -1,    16,
      -1,    36,    -1,    35,    -1,    34,    -1,    33,    -1,    34,
      -1,    33,    -1,    35,    -1,    34,    -1,    36,    -1,    35,
      -1,    34,    -1,    33,    -1,    17,    -1,    18,    -1,    19,
      -1,    84,    85,    -1,    85,    -1,    42,    -1,     1,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short int yyrline[] =
{
       0,   192,   192,   203,   209,   215,   277,   278,   279,   291,
     300,   315,   316,   317,   318,   324,   325,   333,   337,   340,
     353,   357,   370,   371,   379,   383,   387,   400,   413,   417,
     430,   431,   440,   444,   447,   460,   473,   477,   491,   492,
     500,   504,   507,   520,   550,   551,   559,   563,   568,   580,
     592,   596,   602,   608,   615,   621,   627,   632,   637,   642,
     643,   644,   645,   646,   655,   661,   676,   679,   680,   683,
     692,   712,   721,   722,   723,   724,   725,   726,   729,   733,
     736,   744,   745,   758,   760,   761,   762,   805,   812,   819,
     826,   835,   836,   839,   840,   843,   844,   845,   846,   849,
     850,   851,   854,   855,   858,   861
};
#endif

#if YYDEBUG || YYERROR_VERBOSE
/* YYTNME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals. */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "TOK_SEC_BANKSWITCH", "TOK_SEC_MAPPING",
  "TOK_SEC_MEMATTR", "TOK_SEC_PRELOAD", "TOK_SEC_MACRO", "TOK_SEC_VARS",
  "TOK_SEC_JOYSTICK", "TOK_SEC_KEYS", "TOK_SEC_CAPSLOCK",
  "TOK_SEC_NUMLOCK", "TOK_SEC_SCROLLLOCK", "TOK_SEC_DISASM",
  "TOK_SEC_VOICES", "TOK_SEC_UNKNOWN", "TOK_RAM", "TOK_ROM", "TOK_WOM",
  "TOK_PAGE", "TOK_MAC_QUIET", "TOK_MAC_REG", "TOK_MAC_AHEAD",
  "TOK_MAC_BLANK", "TOK_MAC_INSPECT", "TOK_MAC_LOAD", "TOK_MAC_RUN",
  "TOK_MAC_POKE", "TOK_MAC_RUNTO", "TOK_MAC_TRACE", "TOK_MAC_VIEW",
  "TOK_MAC_WATCH", "TOK_DECONLY", "TOK_DEC", "TOK_HEX", "TOK_NAME",
  "TOK_ERROR_BAD", "TOK_ERROR_OOM", "'-'", "'='", "','", "'\\n'",
  "$accept", "config_file", "config", "seed_config", "sec_memspan",
  "bankswitch", "sec_banksw", "banksw_list", "banksw_rec", "mapping",
  "sec_mapping", "mapping_list", "mapping_rec", "memattr", "sec_memattr",
  "memattr_list", "memattr_rec", "preload", "sec_preload", "preload_list",
  "preload_rec", "macro", "sec_macro", "macro_list", "macro_rec",
  "macro_line", "mac_reg", "watch_list", "watch_span", "watch_addr",
  "watch_range", "sec_varlike", "varlike_head", "var_list", "var_rec",
  "sec_unsup", "unknown_sec", "strnum", "dec_num", "hex_num", "string",
  "eolns", "eoln", "error_tok", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const unsigned short int yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,    45,
      61,    44,    10
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,    43,    44,    45,    45,    45,    45,    45,    45,    46,
      46,    47,    47,    47,    47,    48,    49,    50,    50,    51,
      51,    51,    52,    53,    54,    54,    55,    55,    55,    55,
      56,    57,    58,    58,    59,    59,    59,    59,    60,    61,
      62,    62,    63,    63,    64,    65,    66,    66,    67,    67,
      67,    67,    68,    68,    68,    68,    68,    68,    68,    68,
      68,    68,    68,    68,    69,    70,    70,    71,    71,    72,
      73,    74,    75,    75,    75,    75,    75,    75,    76,    76,
      77,    77,    77,    78,    79,    79,    79,    80,    80,    80,
      80,    81,    81,    82,    82,    83,    83,    83,    83,    56,
      56,    56,    84,    84,    85,    86
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     1,     2,     2,     2,     2,     1,     1,     0,
       1,     1,     1,     1,     1,     1,     3,     2,     1,     4,
       1,     2,     1,     3,     2,     1,     6,     8,     1,     2,
       1,     3,     2,     1,     7,     4,     1,     2,     1,     3,
       2,     1,     6,     2,     1,     3,     2,     1,     2,     1,
       2,     1,     3,     5,     4,     4,     3,     3,     3,     2,
       2,     2,     2,     2,     1,     3,     1,     1,     1,     1,
       3,     3,     1,     1,     1,     1,     1,     1,     2,     1,
       4,     1,     2,     2,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     2,     1,     1,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned char yydefact[] =
{
       0,   105,   104,     0,     2,     7,    10,   103,     8,     1,
      15,    22,    30,    38,    44,    72,    73,    74,    75,    76,
      77,    84,    85,    86,    99,   100,   101,     3,     0,    11,
       0,    12,     0,    13,     0,    14,     0,     4,     5,     0,
       6,     0,   102,     0,     0,     0,     0,     0,     0,    83,
      94,    93,     0,    18,     0,    20,     0,     0,    25,     0,
      28,     0,     0,    33,     0,    36,     0,     0,    41,     0,
       0,     0,    64,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    47,    49,     0,    51,     0,    98,
      97,    96,    95,     0,    79,     0,    81,     0,    17,     0,
      21,    24,     0,    29,    32,     0,    37,    40,     0,    43,
      48,    50,    59,    60,     0,     0,    61,     0,     0,     0,
      62,     0,    46,     0,    63,    78,     0,    82,     0,     0,
       0,     0,    56,    92,    91,     0,     0,    57,    58,     0,
      66,    67,    68,    69,    52,    90,    89,    88,    87,     0,
      19,     0,     0,    35,     0,     0,    55,     0,    54,     0,
      80,     0,     0,     0,    53,    65,    70,     0,    26,     0,
      42,     0,    34,    27
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short int yydefgoto[] =
{
      -1,     3,     4,     5,    27,    28,    29,    52,    53,    30,
      31,    57,    58,    32,    33,    62,    63,    34,    35,    67,
      68,    36,    37,    83,    84,    85,    86,   139,   140,   141,
     142,    38,    39,    93,    94,    40,    41,   149,   135,    54,
      95,     6,    55,    88
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -71
static const short int yypact[] =
{
     390,   -71,   -71,     2,   466,   -71,   -37,   -71,   -71,   -71,
     -71,   -71,   -71,   -71,   -71,   -71,   -71,   -71,   -71,   -71,
     -71,   -71,   -71,   -71,   -71,   -71,   -71,   -71,   -37,   -71,
     -37,   -71,   -37,   -71,   -37,   -71,   -37,   -71,   -71,   -37,
     -71,   -37,   -71,    30,    30,    30,     3,   183,    66,   -71,
     -71,   -71,     8,   -71,   -29,   -71,   -37,   310,   -71,    -6,
     -71,   -37,   350,   -71,    -4,   -71,   -37,   433,   -71,     7,
     -37,   388,   -71,   -37,   -37,    24,    51,   -37,    24,    24,
      24,   -37,    51,   230,   -71,   -71,    24,   -71,   -37,   -71,
     -71,   -71,   -71,   270,   -71,    11,   -71,   -37,   -71,    24,
     -71,   -71,    24,   -71,   -71,    24,   -71,   -71,    24,   -71,
     -71,   -71,   -71,   -71,   -37,    42,   -71,    24,   -37,   -37,
     -71,    24,   -71,   -37,   -71,   -71,    56,   -71,   -37,    20,
     -39,    23,   -71,   -71,   -71,    24,   -37,   -71,   -71,    27,
     -71,   -71,   -71,    39,   -71,   -71,   -71,   -71,   -71,   -37,
     -71,    24,    35,   -71,    24,   -37,   -71,    24,   -71,    24,
     -71,   -13,    42,   -37,   -71,   -71,   -71,    24,   -71,   -37,
     -71,   -37,   -71,   -71
};

/* YYPGOTO[NTERM-NUM].  */
static const short int yypgoto[] =
{
     -71,   -71,   -71,   -71,   -71,   -71,   -71,   -71,    28,   -71,
     -71,   -71,    22,   -70,   -71,   -71,    32,   -71,   -71,   -71,
      29,   -71,   -71,   -71,    12,    33,   -71,   -71,   -59,   -71,
     -71,   -71,   -71,   -71,    10,   -71,   -71,   -71,   -57,    65,
     -27,   -71,     0,   135
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -72
static const short int yytable[] =
{
       7,   152,     9,     2,     1,     2,    42,   167,   -16,     1,
      99,   -16,   -16,   -16,   -16,   -16,   -16,   -16,   -16,   -16,
     -16,   -16,   -16,   -16,   -16,   -16,   -16,   -16,    43,     2,
      44,     1,    45,   102,    46,   105,    47,    50,    51,    48,
      12,    49,    50,    51,    60,    65,   108,    87,    96,   115,
       2,   126,    24,    25,    26,   121,   100,    60,    50,    51,
     151,   103,    65,   154,    50,    51,   106,     1,   157,     2,
     109,   111,     2,   112,   113,   133,   134,   116,   159,   101,
      98,   120,   162,    87,    89,    90,    91,    92,   124,   145,
     146,   147,   148,    96,   104,   122,   107,   127,   165,    89,
      90,    91,    92,   125,   110,   169,     0,     0,     2,    59,
      64,    69,     0,     0,   132,     0,     0,     0,   137,   138,
       0,     0,    59,   144,     0,     0,     0,    64,   150,     0,
     153,     0,    69,     0,     0,     8,   156,     0,     0,   158,
     114,     0,     0,   117,   118,   119,     0,     0,     0,   160,
       0,   123,     0,     0,     0,   164,     0,     0,     0,     0,
       0,   168,     0,   170,   128,     0,     0,   129,     0,   172,
     130,   173,     0,   131,     0,     0,     0,     0,    56,    61,
      66,    70,   136,    97,     1,     0,   143,    56,     0,     0,
       0,     0,    61,     0,     0,     0,     0,    66,     0,     0,
     155,     0,    70,     0,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,   161,     0,     0,   163,
       0,     0,   143,     0,   166,     2,     0,     0,    97,     0,
     -45,     1,   171,   -45,   -45,   -45,   -45,   -45,   -45,   -45,
     -45,   -45,   -45,   -45,   -45,   -45,   -45,   -45,   -45,   -45,
       0,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,     0,     0,     0,     0,     0,     0,     0,
     -71,     1,     2,   -71,   -71,   -71,   -71,   -71,   -71,   -71,
     -71,   -71,   -71,   -71,   -71,   -71,   -71,   -71,   -71,   -71,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    89,    90,    91,    92,     0,     0,     0,
     -23,     1,     2,   -23,   -23,   -23,   -23,   -23,   -23,   -23,
     -23,   -23,   -23,   -23,   -23,   -23,   -23,   -23,   -23,   -23,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    50,    51,     0,     0,     0,     0,
     -31,     1,     2,   -31,   -31,   -31,   -31,   -31,   -31,   -31,
     -31,   -31,   -31,   -31,   -31,   -31,   -31,   -31,   -31,   -31,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    50,    51,     0,     0,     0,     1,
      -9,     1,     2,    -9,    -9,    -9,    -9,    -9,    -9,    -9,
      -9,    -9,    -9,    -9,    -9,    -9,    -9,    -9,    -9,    -9,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       2,     0,     2,   -39,     1,     0,   -39,   -39,   -39,   -39,
     -39,   -39,   -39,   -39,   -39,   -39,   -39,   -39,   -39,   -39,
     -39,   -39,   -39,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    50,    51,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26
};

static const short int yycheck[] =
{
       0,    40,     0,    42,     1,    42,     6,    20,     0,     1,
      39,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    28,    42,
      30,     1,    32,    39,    34,    39,    36,    34,    35,    39,
       5,    41,    34,    35,    44,    45,    39,    47,    48,    76,
      42,    40,    17,    18,    19,    82,    56,    57,    34,    35,
      40,    61,    62,    40,    34,    35,    66,     1,    41,    42,
      70,    71,    42,    73,    74,    33,    34,    77,    39,    57,
      52,    81,   152,    83,    33,    34,    35,    36,    88,    33,
      34,    35,    36,    93,    62,    83,    67,    97,   157,    33,
      34,    35,    36,    93,    71,   162,    -1,    -1,    42,    44,
      45,    46,    -1,    -1,   114,    -1,    -1,    -1,   118,   119,
      -1,    -1,    57,   123,    -1,    -1,    -1,    62,   128,    -1,
     130,    -1,    67,    -1,    -1,     0,   136,    -1,    -1,   139,
      75,    -1,    -1,    78,    79,    80,    -1,    -1,    -1,   149,
      -1,    86,    -1,    -1,    -1,   155,    -1,    -1,    -1,    -1,
      -1,   161,    -1,   163,    99,    -1,    -1,   102,    -1,   169,
     105,   171,    -1,   108,    -1,    -1,    -1,    -1,    43,    44,
      45,    46,   117,    48,     1,    -1,   121,    52,    -1,    -1,
      -1,    -1,    57,    -1,    -1,    -1,    -1,    62,    -1,    -1,
     135,    -1,    67,    -1,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,   151,    -1,    -1,   154,
      -1,    -1,   157,    -1,   159,    42,    -1,    -1,    93,    -1,
       0,     1,   167,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      -1,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       0,     1,    42,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    33,    34,    35,    36,    -1,    -1,    -1,
       0,     1,    42,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    34,    35,    -1,    -1,    -1,    -1,
       0,     1,    42,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    34,    35,    -1,    -1,    -1,     1,
       0,     1,    42,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      42,    -1,    42,     0,     1,    -1,     3,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    34,    35,     3,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,     1,    42,    44,    45,    46,    84,    85,    86,     0,
       3,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    47,    48,    49,
      52,    53,    56,    57,    60,    61,    64,    65,    74,    75,
      78,    79,    85,    85,    85,    85,    85,    85,    85,    85,
      34,    35,    50,    51,    82,    85,    86,    54,    55,    82,
      85,    86,    58,    59,    82,    85,    86,    62,    63,    82,
      86,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    66,    67,    68,    69,    85,    86,    33,
      34,    35,    36,    76,    77,    83,    85,    86,    51,    39,
      85,    55,    39,    85,    59,    39,    85,    63,    39,    85,
      68,    85,    85,    85,    82,    83,    85,    82,    82,    82,
      85,    83,    67,    82,    85,    77,    40,    85,    82,    82,
      82,    82,    85,    33,    34,    81,    82,    85,    85,    70,
      71,    72,    73,    82,    85,    33,    34,    35,    36,    80,
      85,    40,    40,    85,    40,    82,    85,    41,    85,    39,
      85,    82,    56,    82,    85,    71,    82,    20,    85,    81,
      85,    82,    85,    85
};

#if ! defined (YYSIZE_T) && defined (__SIZE_TYPE__)
# define YYSIZE_T __SIZE_TYPE__
#endif
#if ! defined (YYSIZE_T) && defined (size_t)
# define YYSIZE_T size_t
#endif
#if ! defined (YYSIZE_T)
# if defined (__STDC__) || defined (__cplusplus)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# endif
#endif
#if ! defined (YYSIZE_T)
# define YYSIZE_T unsigned int
#endif

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { 								\
      yyerror ("syntax error: cannot back up");\
      YYERROR;							\
    }								\
while (0)


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (N)								\
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (0)
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
              (Loc).first_line, (Loc).first_column,	\
              (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (0)

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)		\
do {								\
  if (yydebug)							\
    {								\
      YYFPRINTF (stderr, "%s ", Title);				\
      yysymprint (stderr, 					\
                  Type, Value);	\
      YYFPRINTF (stderr, "\n");					\
    }								\
} while (0)

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_stack_print (short int *bottom, short int *top)
#else
static void
yy_stack_print (bottom, top)
    short int *bottom;
    short int *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (/* Nothing. */; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_reduce_print (int yyrule)
#else
static void
yy_reduce_print (yyrule)
    int yyrule;
#endif
{
  int yyi;
  unsigned int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %u), ",
             yyrule - 1, yylno);
  /* Print the symbols being reduced, and their result.  */
  for (yyi = yyprhs[yyrule]; 0 <= yyrhs[yyi]; yyi++)
    YYFPRINTF (stderr, "%s ", yytname [yyrhs[yyi]]);
  YYFPRINTF (stderr, "-> %s\n", yytname [yyr1[yyrule]]);
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (Rule);		\
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   SIZE_MAX < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined (__GLIBC__) && defined (_STRING_H)
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
#   if defined (__STDC__) || defined (__cplusplus)
yystrlen (const char *yystr)
#   else
yystrlen (yystr)
     const char *yystr;
#   endif
{
  register const char *yys = yystr;

  while (*yys++ != '\0')
    continue;

  return yys - yystr - 1;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined (__GLIBC__) && defined (_STRING_H) && defined (_GNU_SOURCE)
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
#   if defined (__STDC__) || defined (__cplusplus)
yystpcpy (char *yydest, const char *yysrc)
#   else
yystpcpy (yydest, yysrc)
     char *yydest;
     const char *yysrc;
#   endif
{
  register char *yyd = yydest;
  register const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

#endif /* !YYERROR_VERBOSE */



#if YYDEBUG
/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yysymprint (FILE *yyoutput, int yytype, YYSTYPE *yyvaluep)
#else
static void
yysymprint (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);


# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
  switch (yytype)
    {
      default:
        break;
    }
  YYFPRINTF (yyoutput, ")");
}

#endif /* ! YYDEBUG */
/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
        break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM);
# else
int yyparse ();
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */



/* The look-ahead symbol.  */
int yychar;

/* The semantic value of the look-ahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;



/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM)
# else
int yyparse (YYPARSE_PARAM)
  void *YYPARSE_PARAM;
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
  
  register int yystate;
  register int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Look-ahead token as an internal (translated) token number.  */
  int yytoken = 0;

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  short int yyssa[YYINITDEPTH];
  short int *yyss = yyssa;
  register short int *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  register YYSTYPE *yyvsp;



#define YYPOPSTACK   (yyvsp--, yyssp--)

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


  /* When reducing, the number of symbols on the RHS of the reduced
     rule.  */
  int yylen;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;


  yyvsp[0] = yylval;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed. so pushing a state here evens the stacks.
     */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack. Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	short int *yyss1 = yyss;


	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow ("parser stack overflow",
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),

		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyoverflowlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyoverflowlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	short int *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyoverflowlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);

#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;


      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

/* Do appropriate processing given the current state.  */
/* Read a look-ahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to look-ahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a look-ahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid look-ahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the look-ahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;


  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  yystate = yyn;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:
#line 193 "bincfg/bincfg_grmr.y"
    {
                bc_parsed_cfg->diags = bc_diag_list;
                bc_diag_list  = NULL;
            ;}
    break;

  case 3:
#line 204 "bincfg/bincfg_grmr.y"
    {
                if ((yyvsp[0].memspan_list)) 
                    ll_concat((ll_t**)&(bc_parsed_cfg->span), 
                                 &((yyvsp[0].memspan_list)->l));
            ;}
    break;

  case 4:
#line 210 "bincfg/bincfg_grmr.y"
    {
                if ((yyvsp[0].macro_list)) 
                    ll_concat((ll_t**)&(bc_parsed_cfg->macro), 
                                 &((yyvsp[0].macro_list)->l));
            ;}
    break;

  case 5:
#line 216 "bincfg/bincfg_grmr.y"
    {
                ll_t **list = NULL;

                if ((yyvsp[0].varlike).vars)
                {
                    switch ((yyvsp[0].varlike).type)
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
                    ll_concat(list, &((yyvsp[0].varlike).vars->l));
                } else if ((yyvsp[0].varlike).vars && !list)
                {
                    /* error to report. */
                    BC_DIAG(BC_DIAG_ERROR, 
                            bc_cursect ? bc_cursect : "<toplevel>", 
                            bc_line_no, 
                            "Internal error processing variable lists");
                }
            ;}
    break;

  case 8:
#line 280 "bincfg/bincfg_grmr.y"
    {
                sprintf(bc_errbuf, "Unexpected token/state '%s'", BC_TOK);
                BC_DIAG(BC_DIAG_ERROR, BC_SEC,
                        bc_saved_lineno, bc_errbuf);
            ;}
    break;

  case 9:
#line 291 "bincfg/bincfg_grmr.y"
    {
                /* Before the chicken or egg came the rooster */
                if (!bc_parsed_cfg)
                {
                    bc_parsed_cfg = CALLOC(bc_cfgfile_t, 1);
                    BC_CHKOOM(bc_parsed_cfg);
                    bc_cursect = NULL;
                }
            ;}
    break;

  case 10:
#line 301 "bincfg/bincfg_grmr.y"
    {
                if (!bc_parsed_cfg)
                {
                    bc_parsed_cfg = CALLOC(bc_cfgfile_t, 1);
                    BC_CHKOOM(bc_parsed_cfg);
                    bc_cursect = NULL;
                }
            ;}
    break;

  case 15:
#line 324 "bincfg/bincfg_grmr.y"
    { S("[bankswitch]"); ;}
    break;

  case 16:
#line 326 "bincfg/bincfg_grmr.y"
    {
                ll_t *l = &((yyvsp[0].memspan_list)->l);
                ll_reverse(&l);
                (yyval.memspan_list) = (bc_memspan_t *)l;
            ;}
    break;

  case 17:
#line 334 "bincfg/bincfg_grmr.y"
    {
                (yyval.memspan_list) = (bc_memspan_t*)ll_insert(&((yyvsp[-1].memspan_list)->l), &((yyvsp[0].memspan_list)->l));
            ;}
    break;

  case 18:
#line 337 "bincfg/bincfg_grmr.y"
    { (yyval.memspan_list) = (yyvsp[0].memspan_list);   ;}
    break;

  case 19:
#line 341 "bincfg/bincfg_grmr.y"
    {
                (yyval.memspan_list) = CALLOC(bc_memspan_t, 1);
                BC_CHKOOM((yyval.memspan_list));
                (yyval.memspan_list)->s_fofs = 0;
                (yyval.memspan_list)->e_fofs = 0;
                (yyval.memspan_list)->s_addr = (yyvsp[-3].intv);
                (yyval.memspan_list)->e_addr = (yyvsp[-1].intv);
                (yyval.memspan_list)->flags  = BC_SPAN_B | BC_SPAN_R;
                (yyval.memspan_list)->width  = 16;
                (yyval.memspan_list)->epage  = BC_SPAN_NOPAGE;
                (yyval.memspan_list)->f_name = NULL;
            ;}
    break;

  case 20:
#line 354 "bincfg/bincfg_grmr.y"
    {
                (yyval.memspan_list) = NULL;
            ;}
    break;

  case 21:
#line 358 "bincfg/bincfg_grmr.y"
    {
                (yyval.memspan_list) = NULL;
                sprintf(bc_errbuf, "Unexpected token/state '%s'", BC_TOK);
                BC_DIAG(BC_DIAG_ERROR, "[bankswitch]", 
                        bc_saved_lineno, bc_errbuf);
            ;}
    break;

  case 22:
#line 370 "bincfg/bincfg_grmr.y"
    { S("[mapping]"); ;}
    break;

  case 23:
#line 372 "bincfg/bincfg_grmr.y"
    {
                ll_t *l = &((yyvsp[0].memspan_list)->l);
                ll_reverse(&l);
                (yyval.memspan_list) = (bc_memspan_t *)l;
            ;}
    break;

  case 24:
#line 380 "bincfg/bincfg_grmr.y"
    {
                (yyval.memspan_list) = (bc_memspan_t*)ll_insert(&((yyvsp[-1].memspan_list)->l), &((yyvsp[0].memspan_list)->l));
            ;}
    break;

  case 25:
#line 383 "bincfg/bincfg_grmr.y"
    { (yyval.memspan_list) = (yyvsp[0].memspan_list); ;}
    break;

  case 26:
#line 388 "bincfg/bincfg_grmr.y"
    {
                (yyval.memspan_list) = CALLOC(bc_memspan_t, 1);
                BC_CHKOOM((yyval.memspan_list));
                (yyval.memspan_list)->s_fofs = (yyvsp[-5].intv);
                (yyval.memspan_list)->e_fofs = (yyvsp[-3].intv);
                (yyval.memspan_list)->s_addr = (yyvsp[-1].intv);
                (yyval.memspan_list)->e_addr = (yyvsp[-1].intv) + (yyvsp[-3].intv) - (yyvsp[-5].intv);
                (yyval.memspan_list)->flags  = BC_SPAN_PL | BC_SPAN_R;
                (yyval.memspan_list)->width  = 16;
                (yyval.memspan_list)->epage  = BC_SPAN_NOPAGE;
                (yyval.memspan_list)->f_name = NULL;
            ;}
    break;

  case 27:
#line 401 "bincfg/bincfg_grmr.y"
    {
                (yyval.memspan_list) = CALLOC(bc_memspan_t, 1);
                BC_CHKOOM((yyval.memspan_list));
                (yyval.memspan_list)->s_fofs = (yyvsp[-7].intv);
                (yyval.memspan_list)->e_fofs = (yyvsp[-5].intv);
                (yyval.memspan_list)->s_addr = (yyvsp[-3].intv);
                (yyval.memspan_list)->e_addr = (yyvsp[-3].intv) + (yyvsp[-5].intv) - (yyvsp[-7].intv);
                (yyval.memspan_list)->flags  = BC_SPAN_PL | BC_SPAN_R | BC_SPAN_EP;
                (yyval.memspan_list)->width  = 16;
                (yyval.memspan_list)->epage  = (yyvsp[-1].intv);
                (yyval.memspan_list)->f_name = NULL;
            ;}
    break;

  case 28:
#line 414 "bincfg/bincfg_grmr.y"
    {
                (yyval.memspan_list) = NULL;
            ;}
    break;

  case 29:
#line 418 "bincfg/bincfg_grmr.y"
    {
                (yyval.memspan_list) = NULL;
                sprintf(bc_errbuf, "Unexpected token/state '%s'", BC_TOK);
                BC_DIAG(BC_DIAG_ERROR, "[mapping]", 
                        bc_saved_lineno, bc_errbuf);
            ;}
    break;

  case 30:
#line 430 "bincfg/bincfg_grmr.y"
    { S("[memattr]"); ;}
    break;

  case 31:
#line 432 "bincfg/bincfg_grmr.y"
    {
                ll_t *l = &((yyvsp[0].memspan_list)->l);
                ll_reverse(&l);
                (yyval.memspan_list) = (bc_memspan_t *)l;
            ;}
    break;

  case 32:
#line 441 "bincfg/bincfg_grmr.y"
    {
                (yyval.memspan_list) = (bc_memspan_t*)ll_insert(&((yyvsp[-1].memspan_list)->l), &((yyvsp[0].memspan_list)->l));
            ;}
    break;

  case 33:
#line 444 "bincfg/bincfg_grmr.y"
    { (yyval.memspan_list) = (yyvsp[0].memspan_list); ;}
    break;

  case 34:
#line 448 "bincfg/bincfg_grmr.y"
    {
                (yyval.memspan_list) = CALLOC(bc_memspan_t, 1);
                BC_CHKOOM((yyval.memspan_list));
                (yyval.memspan_list)->s_fofs = 0;
                (yyval.memspan_list)->e_fofs = 0;
                (yyval.memspan_list)->s_addr = (yyvsp[-6].intv);
                (yyval.memspan_list)->e_addr = (yyvsp[-4].intv);
                (yyval.memspan_list)->flags  = (yyvsp[-2].intv) | ((yyvsp[-1].intv) < 16 ? BC_SPAN_N : 0);
                (yyval.memspan_list)->width  = (yyvsp[-1].intv);
                (yyval.memspan_list)->epage  = BC_SPAN_NOPAGE;
                (yyval.memspan_list)->f_name = NULL;
            ;}
    break;

  case 35:
#line 461 "bincfg/bincfg_grmr.y"
    {
                (yyval.memspan_list) = CALLOC(bc_memspan_t, 1);
                BC_CHKOOM((yyval.memspan_list));
                (yyval.memspan_list)->s_fofs = 0;
                (yyval.memspan_list)->e_fofs = 0;
                (yyval.memspan_list)->s_addr = (yyvsp[-3].intv);
                (yyval.memspan_list)->e_addr = (yyvsp[-1].intv);
                (yyval.memspan_list)->flags  = BC_SPAN_R;
                (yyval.memspan_list)->width  = 16;
                (yyval.memspan_list)->epage  = BC_SPAN_NOPAGE;
                (yyval.memspan_list)->f_name = NULL;
            ;}
    break;

  case 36:
#line 474 "bincfg/bincfg_grmr.y"
    {   
                (yyval.memspan_list) = NULL;
            ;}
    break;

  case 37:
#line 478 "bincfg/bincfg_grmr.y"
    {
                (yyval.memspan_list) = NULL;
                sprintf(bc_errbuf, "Unexpected token/state '%s'\n", BC_TOK);
                BC_DIAG(BC_DIAG_ERROR, "[memattr]", 
                        bc_saved_lineno, bc_errbuf);
            ;}
    break;

  case 38:
#line 491 "bincfg/bincfg_grmr.y"
    { S("[preload]"); ;}
    break;

  case 39:
#line 493 "bincfg/bincfg_grmr.y"
    {
                ll_t *l = &((yyvsp[0].memspan_list)->l);
                ll_reverse(&l);
                (yyval.memspan_list) = (bc_memspan_t *)l;
            ;}
    break;

  case 40:
#line 501 "bincfg/bincfg_grmr.y"
    {
                (yyval.memspan_list) = (bc_memspan_t*)ll_insert(&((yyvsp[-1].memspan_list)->l), &((yyvsp[0].memspan_list)->l));
            ;}
    break;

  case 41:
#line 504 "bincfg/bincfg_grmr.y"
    { (yyval.memspan_list) = (yyvsp[0].memspan_list); ;}
    break;

  case 42:
#line 508 "bincfg/bincfg_grmr.y"
    {
                (yyval.memspan_list) = CALLOC(bc_memspan_t, 1);
                BC_CHKOOM((yyval.memspan_list));
                (yyval.memspan_list)->s_fofs = (yyvsp[-5].intv);
                (yyval.memspan_list)->e_fofs = (yyvsp[-3].intv);
                (yyval.memspan_list)->s_addr = (yyvsp[-1].intv);
                (yyval.memspan_list)->e_addr = (yyvsp[-1].intv) + (yyvsp[-3].intv) - (yyvsp[-5].intv);
                (yyval.memspan_list)->flags  = BC_SPAN_PL;
                (yyval.memspan_list)->width  = 16;
                (yyval.memspan_list)->epage  = BC_SPAN_NOPAGE;
                (yyval.memspan_list)->f_name = NULL;
            ;}
    break;

  case 43:
#line 521 "bincfg/bincfg_grmr.y"
    {
                (yyval.memspan_list) = NULL;
                sprintf(bc_errbuf, "Unexpected token/state '%s'", BC_TOK);
                BC_DIAG(BC_DIAG_ERROR, "[preload]", 
                        bc_saved_lineno, bc_errbuf);
            ;}
    break;

  case 44:
#line 550 "bincfg/bincfg_grmr.y"
    { S("[macro]"); ;}
    break;

  case 45:
#line 552 "bincfg/bincfg_grmr.y"
    {
                ll_t *l = &((yyvsp[0].macro_list)->l);
                ll_reverse(&l);
                (yyval.macro_list) = (bc_macro_t *)l;
            ;}
    break;

  case 46:
#line 560 "bincfg/bincfg_grmr.y"
    {
                (yyval.macro_list) = (bc_macro_t*)ll_insert(&((yyvsp[-1].macro_list)->l), &((yyvsp[0].macro_list)->l));
            ;}
    break;

  case 47:
#line 563 "bincfg/bincfg_grmr.y"
    { (yyval.macro_list) = (yyvsp[0].macro_list); ;}
    break;

  case 48:
#line 569 "bincfg/bincfg_grmr.y"
    {
                if ((yyvsp[0].macro).cmd == BC_MAC_ERROR)
                    (yyval.macro_list) = NULL;
                else
                {
                    (yyval.macro_list)  = CALLOC(bc_macro_t, 1);
                    BC_CHKOOM((yyval.macro_list));
                    *(yyval.macro_list) = (yyvsp[0].macro);
                    (yyval.macro_list)->quiet = 1;
                }
            ;}
    break;

  case 49:
#line 581 "bincfg/bincfg_grmr.y"
    {
                if ((yyvsp[0].macro).cmd == BC_MAC_ERROR)
                    (yyval.macro_list) = NULL;
                else
                {
                    (yyval.macro_list)  = CALLOC(bc_macro_t, 1);
                    BC_CHKOOM((yyval.macro_list));
                    *(yyval.macro_list) = (yyvsp[0].macro);
                    (yyval.macro_list)->quiet = 0;
                }
            ;}
    break;

  case 50:
#line 593 "bincfg/bincfg_grmr.y"
    {
                (yyval.macro_list) = NULL;
            ;}
    break;

  case 51:
#line 597 "bincfg/bincfg_grmr.y"
    {
                (yyval.macro_list) = NULL;
            ;}
    break;

  case 52:
#line 603 "bincfg/bincfg_grmr.y"
    {
                (yyval.macro).cmd            = BC_MAC_REG;
                (yyval.macro).arg.reg.reg    = (yyvsp[-2].intv);
                (yyval.macro).arg.reg.value  = (yyvsp[-1].intv);
            ;}
    break;

  case 53:
#line 609 "bincfg/bincfg_grmr.y"
    {
                (yyval.macro).cmd            = BC_MAC_LOAD;
                (yyval.macro).arg.load.name  = (yyvsp[-3].strv);
                (yyval.macro).arg.load.width = (yyvsp[-2].intv);
                (yyval.macro).arg.load.addr  = (yyvsp[-1].intv);
            ;}
    break;

  case 54:
#line 616 "bincfg/bincfg_grmr.y"
    {
                (yyval.macro).cmd            = BC_MAC_WATCH;
                (yyval.macro).arg.watch      = (yyvsp[-1].mac_watch);
                (yyval.macro).arg.watch.name = (yyvsp[-2].strv);
            ;}
    break;

  case 55:
#line 622 "bincfg/bincfg_grmr.y"
    {
                (yyval.macro).cmd = BC_MAC_POKE;
                (yyval.macro).arg.poke.addr  = (yyvsp[-2].intv);
                (yyval.macro).arg.poke.value = (yyvsp[-1].intv);
            ;}
    break;

  case 56:
#line 628 "bincfg/bincfg_grmr.y"
    {
                (yyval.macro).cmd = BC_MAC_INSPECT;
                (yyval.macro).arg.inspect.addr = (yyvsp[-1].intv);
            ;}
    break;

  case 57:
#line 633 "bincfg/bincfg_grmr.y"
    {
                (yyval.macro).cmd = BC_MAC_RUNTO;
                (yyval.macro).arg.runto.addr = (yyvsp[-1].intv);
            ;}
    break;

  case 58:
#line 638 "bincfg/bincfg_grmr.y"
    {
                (yyval.macro).cmd = BC_MAC_TRACE;
                (yyval.macro).arg.runto.addr = (yyvsp[-1].intv);
            ;}
    break;

  case 59:
#line 642 "bincfg/bincfg_grmr.y"
    {   (yyval.macro).cmd  = BC_MAC_AHEAD;     ;}
    break;

  case 60:
#line 643 "bincfg/bincfg_grmr.y"
    {   (yyval.macro).cmd  = BC_MAC_BLANK;     ;}
    break;

  case 61:
#line 644 "bincfg/bincfg_grmr.y"
    {   (yyval.macro).cmd  = BC_MAC_RUN;       ;}
    break;

  case 62:
#line 645 "bincfg/bincfg_grmr.y"
    {   (yyval.macro).cmd  = BC_MAC_VIEW;      ;}
    break;

  case 63:
#line 647 "bincfg/bincfg_grmr.y"
    {
                (yyval.macro).cmd = BC_MAC_ERROR;
                sprintf(bc_errbuf, "Unexpected token/state '%s'", BC_TOK);
                BC_DIAG(BC_DIAG_WARNING, "[macro]", 
                        bc_saved_lineno, bc_errbuf);
            ;}
    break;

  case 64:
#line 655 "bincfg/bincfg_grmr.y"
    { (yyval.intv) = bc_hex; ;}
    break;

  case 65:
#line 662 "bincfg/bincfg_grmr.y"
    {
                size_t new_size;
                int new_spans, i;

                new_spans = (yyvsp[-2].mac_watch).spans + (yyvsp[0].mac_watch).spans;
                new_size  = new_spans*sizeof(uint_16)*2;

                (yyval.mac_watch).addr  = (uint_16 *)realloc((yyvsp[-2].mac_watch).addr, new_size);
                (yyval.mac_watch).spans = new_spans;
                BC_CHKOOM((yyval.mac_watch).addr);

                for (i = 0; i < (yyvsp[0].mac_watch).spans*2; i++)
                    (yyval.mac_watch).addr[i + 2*(yyvsp[-2].mac_watch).spans] = (yyvsp[0].mac_watch).addr[i];
            ;}
    break;

  case 66:
#line 676 "bincfg/bincfg_grmr.y"
    { (yyval.mac_watch) = (yyvsp[0].mac_watch); ;}
    break;

  case 69:
#line 684 "bincfg/bincfg_grmr.y"
    {
                (yyval.mac_watch).spans   = 1;
                (yyval.mac_watch).addr    = CALLOC(uint_16, 2); BC_CHKOOM((yyval.mac_watch).addr);
                (yyval.mac_watch).addr[0] = (yyvsp[0].intv);
                (yyval.mac_watch).addr[1] = (yyvsp[0].intv);
            ;}
    break;

  case 70:
#line 693 "bincfg/bincfg_grmr.y"
    {
                (yyval.mac_watch).spans   = 1;
                (yyval.mac_watch).addr    = CALLOC(uint_16, 2); BC_CHKOOM((yyval.mac_watch).addr);
                (yyval.mac_watch).addr[0] = (yyvsp[-2].intv);
                (yyval.mac_watch).addr[1] = (yyvsp[0].intv);
            ;}
    break;

  case 71:
#line 713 "bincfg/bincfg_grmr.y"
    {
                ll_t *l = (ll_t *)(yyvsp[0].var_list);
                ll_reverse(&(l));
                (yyval.varlike).type = (yyvsp[-2].varlike_type);
                (yyval.varlike).vars = (bc_var_t*)l;
            ;}
    break;

  case 72:
#line 721 "bincfg/bincfg_grmr.y"
    { (yyval.varlike_type)=BC_VL_VARS;       S("[vars]"      ); ;}
    break;

  case 73:
#line 722 "bincfg/bincfg_grmr.y"
    { (yyval.varlike_type)=BC_VL_JOYSTICK;   S("[joystick]"  ); ;}
    break;

  case 74:
#line 723 "bincfg/bincfg_grmr.y"
    { (yyval.varlike_type)=BC_VL_KEYS;       S("[keys]"      ); ;}
    break;

  case 75:
#line 724 "bincfg/bincfg_grmr.y"
    { (yyval.varlike_type)=BC_VL_CAPSLOCK;   S("[capslock]"  ); ;}
    break;

  case 76:
#line 725 "bincfg/bincfg_grmr.y"
    { (yyval.varlike_type)=BC_VL_NUMLOCK;    S("[numlock]"   ); ;}
    break;

  case 77:
#line 726 "bincfg/bincfg_grmr.y"
    { (yyval.varlike_type)=BC_VL_SCROLLLOCK; S("[scrolllock]"); ;}
    break;

  case 78:
#line 730 "bincfg/bincfg_grmr.y"
    {
                (yyval.var_list) = (bc_var_t*)ll_insert(&(yyvsp[-1].var_list)->l, &(yyvsp[0].var_list)->l);
            ;}
    break;

  case 79:
#line 733 "bincfg/bincfg_grmr.y"
    { (yyval.var_list) = (yyvsp[0].var_list); ;}
    break;

  case 80:
#line 737 "bincfg/bincfg_grmr.y"
    {
                (yyval.var_list) = CALLOC(bc_var_t, 1);
                BC_CHKOOM((yyval.var_list));

                (yyval.var_list)->name = (yyvsp[-3].strv);
                (yyval.var_list)->val  = (yyvsp[-1].strnum);
            ;}
    break;

  case 81:
#line 744 "bincfg/bincfg_grmr.y"
    { (yyval.var_list) = NULL; ;}
    break;

  case 82:
#line 746 "bincfg/bincfg_grmr.y"
    {
                (yyval.var_list) = NULL;
                sprintf(bc_errbuf, "Unexpected token/state '%s'\n", BC_TOK);
                BC_DIAG(BC_DIAG_WARNING, "[vars]", 
                        bc_saved_lineno, bc_errbuf);
            ;}
    break;

  case 84:
#line 760 "bincfg/bincfg_grmr.y"
    { S("[disasm]"); ;}
    break;

  case 85:
#line 761 "bincfg/bincfg_grmr.y"
    { S("[voices]"); ;}
    break;

  case 86:
#line 763 "bincfg/bincfg_grmr.y"
    {
                static char bc_unknown[256];

                if (bc_txt)
                {
                    strncpy(bc_unknown, bc_txt, 255);
                    bc_unknown[255] = 0;
                }

                S(bc_unknown);
            ;}
    break;

  case 87:
#line 806 "bincfg/bincfg_grmr.y"
    {
                (yyval.strnum).flag    = BC_VAR_STRING;
                (yyval.strnum).str_val = strdup(bc_txt);  BC_CHKOOM((yyval.strnum).str_val);
                (yyval.strnum).dec_val = 0;
                (yyval.strnum).hex_val = 0;
            ;}
    break;

  case 88:
#line 813 "bincfg/bincfg_grmr.y"
    {
                (yyval.strnum).flag    = BC_VAR_STRING | BC_VAR_HEXNUM;
                (yyval.strnum).str_val = strdup(bc_txt);  BC_CHKOOM((yyval.strnum).str_val);
                (yyval.strnum).dec_val = 0;
                (yyval.strnum).hex_val = bc_hex;
            ;}
    break;

  case 89:
#line 820 "bincfg/bincfg_grmr.y"
    {
                (yyval.strnum).flag    = BC_VAR_STRING | BC_VAR_HEXNUM | BC_VAR_DECNUM;
                (yyval.strnum).str_val = strdup(bc_txt);  BC_CHKOOM((yyval.strnum).str_val);
                (yyval.strnum).dec_val = bc_dec;
                (yyval.strnum).hex_val = bc_hex;
            ;}
    break;

  case 90:
#line 827 "bincfg/bincfg_grmr.y"
    {
                (yyval.strnum).flag    = BC_VAR_STRING | BC_VAR_DECNUM;
                (yyval.strnum).str_val = strdup(bc_txt);  BC_CHKOOM((yyval.strnum).str_val);
                (yyval.strnum).dec_val = bc_dec;
                (yyval.strnum).hex_val = 0;
            ;}
    break;

  case 91:
#line 835 "bincfg/bincfg_grmr.y"
    { (yyval.intv) = bc_dec; ;}
    break;

  case 92:
#line 836 "bincfg/bincfg_grmr.y"
    { (yyval.intv) = bc_dec; ;}
    break;

  case 93:
#line 839 "bincfg/bincfg_grmr.y"
    { (yyval.intv) = bc_hex; ;}
    break;

  case 94:
#line 840 "bincfg/bincfg_grmr.y"
    { (yyval.intv) = bc_hex; ;}
    break;

  case 95:
#line 843 "bincfg/bincfg_grmr.y"
    { (yyval.strv) = strdup(bc_txt); BC_CHKOOM((yyval.strv)); ;}
    break;

  case 96:
#line 844 "bincfg/bincfg_grmr.y"
    { (yyval.strv) = strdup(bc_txt); BC_CHKOOM((yyval.strv)); ;}
    break;

  case 97:
#line 845 "bincfg/bincfg_grmr.y"
    { (yyval.strv) = strdup(bc_txt); BC_CHKOOM((yyval.strv)); ;}
    break;

  case 98:
#line 846 "bincfg/bincfg_grmr.y"
    { (yyval.strv) = strdup(bc_txt); BC_CHKOOM((yyval.strv)); ;}
    break;

  case 99:
#line 849 "bincfg/bincfg_grmr.y"
    { (yyval.intv) = BC_SPAN_RAM; ;}
    break;

  case 100:
#line 850 "bincfg/bincfg_grmr.y"
    { (yyval.intv) = BC_SPAN_ROM; ;}
    break;

  case 101:
#line 851 "bincfg/bincfg_grmr.y"
    { (yyval.intv) = BC_SPAN_WOM; ;}
    break;

  case 104:
#line 858 "bincfg/bincfg_grmr.y"
    { bc_dont_save = 0; bc_saved_tok = YYEMPTY; ;}
    break;

  case 105:
#line 862 "bincfg/bincfg_grmr.y"
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
            ;}
    break;


    }

/* Line 1037 of yacc.c.  */
#line 2229 "bincfg/bincfg_grmr.tab.c"

  yyvsp -= yylen;
  yyssp -= yylen;


  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;


  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (YYPACT_NINF < yyn && yyn < YYLAST)
	{
	  YYSIZE_T yysize = 0;
	  int yytype = YYTRANSLATE (yychar);
	  const char* yyprefix;
	  char *yymsg;
	  int yyx;

	  /* Start YYX at -YYN if negative to avoid negative indexes in
	     YYCHECK.  */
	  int yyxbegin = yyn < 0 ? -yyn : 0;

	  /* Stay within bounds of both yycheck and yytname.  */
	  int yychecklim = YYLAST - yyn;
	  int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
	  int yycount = 0;

	  yyprefix = ", expecting ";
	  for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	      {
		yysize += yystrlen (yyprefix) + yystrlen (yytname [yyx]);
		yycount += 1;
		if (yycount == 5)
		  {
		    yysize = 0;
		    break;
		  }
	      }
	  yysize += (sizeof ("syntax error, unexpected ")
		     + yystrlen (yytname[yytype]));
	  yymsg = (char *) YYSTACK_ALLOC (yysize);
	  if (yymsg != 0)
	    {
	      char *yyp = yystpcpy (yymsg, "syntax error, unexpected ");
	      yyp = yystpcpy (yyp, yytname[yytype]);

	      if (yycount < 5)
		{
		  yyprefix = ", expecting ";
		  for (yyx = yyxbegin; yyx < yyxend; ++yyx)
		    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
		      {
			yyp = yystpcpy (yyp, yyprefix);
			yyp = yystpcpy (yyp, yytname[yyx]);
			yyprefix = " or ";
		      }
		}
	      yyerror (yymsg);
	      YYSTACK_FREE (yymsg);
	    }
	  else
	    yyerror ("syntax error; also virtual memory exhausted");
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror ("syntax error");
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse look-ahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* If at end of input, pop the error token,
	     then the rest of the stack, then return failure.  */
	  if (yychar == YYEOF)
	     for (;;)
	       {

		 YYPOPSTACK;
		 if (yyssp == yyss)
		   YYABORT;
		 yydestruct ("Error: popping",
                             yystos[*yyssp], yyvsp);
	       }
        }
      else
	{
	  yydestruct ("Error: discarding", yytoken, &yylval);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse look-ahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

#ifdef __GNUC__
  /* Pacify GCC when the user code never invokes YYERROR and the label
     yyerrorlab therefore never appears in user code.  */
  if (0)
     goto yyerrorlab;
#endif

yyvsp -= yylen;
  yyssp -= yylen;
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;


      yydestruct ("Error: popping", yystos[yystate], yyvsp);
      YYPOPSTACK;
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  *++yyvsp = yylval;


  /* Shift the error token. */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yydestruct ("Error: discarding lookahead",
              yytoken, &yylval);
  yychar = YYEMPTY;
  yyresult = 1;
  goto yyreturn;

#ifndef yyoverflow
/*----------------------------------------------.
| yyoverflowlab -- parser overflow comes here.  |
`----------------------------------------------*/
yyoverflowlab:
  yyerror ("parser stack overflow");
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  return yyresult;
}


#line 875 "bincfg/bincfg_grmr.y"


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


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
/* Line 1318 of yacc.c.  */
#line 127 "bincfg/bincfg_grmr.tab.h"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE bc_lval;




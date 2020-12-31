/* Scanner for INTVPC .CFG files */
%option case-insensitive
%option never-interactive
%option noyywrap
%option batch
%option 8bit
%option nounput
%option prefix="bc_"
%option outfile="bincfg/bincfg_lex.c"

%x SEC_IGN
%s SEC_VAR
%s SEC_BSW
%s SEC_MAP
%s SEC_ATR
%s SEC_PRL
%x SEC_MC0
%x SEC_MC1
%x SEC_MC2

%{

/* put any #includes here */
#include "config.h"
#include "bincfg/bincfg.h"
#include "bincfg/bincfg_grmr.tab.h"


#define bc_lex bc_lex1

int  bc_line_no = 1;
int  bc_dec; 
int  bc_hex;
char *bc_txt = NULL;
static int bc_txt_alloc = 0;


#define YYTEXT_TO_BC_TXT \
        do {                                                            \
            if (yyleng > bc_txt_alloc)                                  \
            {                                                           \
                if (bc_txt)                                             \
                {                                                       \
                    bc_txt_alloc = bc_txt_alloc << 1;                   \
                    free(bc_txt);                                       \
                } else                                                  \
                    bc_txt_alloc = 256;                                 \
                                                                        \
                bc_txt = malloc(bc_txt_alloc);                          \
                                                                        \
                if (!bc_txt)                                            \
                    return TOK_ERROR_OOM;                               \
            }                                                           \
                                                                        \
            strcpy(bc_txt, yytext);                                     \
        } while (0)



%}

HNUM    [0-9A-Fa-f]+
NAME    [A-Za-z_0-9\{\}"']+

%%
                   
%{ 
/* -------------------------------------------------------------------- */
/*  Rules for identifying section headers.                              */
/* -------------------------------------------------------------------- */
%} 
"[bankswitch]"      {   BEGIN(SEC_BSW); return  TOK_SEC_BANKSWITCH;     }
"[mapping]"         {   BEGIN(SEC_MAP); return  TOK_SEC_MAPPING;        }
"[memattr]"         {   BEGIN(SEC_ATR); return  TOK_SEC_MEMATTR;        }
"[preload]"         {   BEGIN(SEC_PRL); return  TOK_SEC_PRELOAD;        }
                        
"[macro]"           {   BEGIN(SEC_MC0); return  TOK_SEC_MACRO;          }

"[vars]"            {   BEGIN(SEC_VAR); return  TOK_SEC_VARS;           }
"[joystick]"        {   BEGIN(SEC_VAR); return  TOK_SEC_JOYSTICK;       }
"[keys]"            {   BEGIN(SEC_VAR); return  TOK_SEC_KEYS;           }
"[capslock]"        {   BEGIN(SEC_VAR); return  TOK_SEC_CAPSLOCK;       }
"[numlock]"         {   BEGIN(SEC_VAR); return  TOK_SEC_NUMLOCK;        }
"[scrolllock]"      {   BEGIN(SEC_VAR); return  TOK_SEC_SCROLLLOCK;     }
                        
"[disasm]"          {   BEGIN(SEC_IGN); return  TOK_SEC_DISASM;         }
"[voices]"          {   BEGIN(SEC_IGN); return  TOK_SEC_VOICES;         }
                        
"["[^\]]*"]"        {   BEGIN(SEC_IGN); return  TOK_SEC_UNKNOWN;        }

%{ 
/* -------------------------------------------------------------------- */
/*  Mini-scanner for ignored sections:  Just eat them right up.         */
/* -------------------------------------------------------------------- */
%} 
<SEC_IGN>\n         {   bc_line_no++; /* eat newlines */                }
<SEC_IGN>;.*        {   /* eat comments */                              }
<SEC_IGN>[^\[;\n]+  {   /* eat non-section openers. */                  }
<SEC_IGN>"["        {   yyless(0);  BEGIN(INITIAL); return '\n';        }

%{ 
/* -------------------------------------------------------------------- */
/*  Keywords that are only valid in memory-attribute section.           */
/* -------------------------------------------------------------------- */
%} 
<SEC_ATR>"RAM"      {   return TOK_RAM;                                 }
<SEC_ATR>"ROM"      {   return TOK_ROM;                                 }
<SEC_ATR>"WOM"      {   return TOK_WOM;                                 }

%{ 
/* -------------------------------------------------------------------- */
/*  Keywords that are only valid in the mapping section.                */
/* -------------------------------------------------------------------- */
%} 
<SEC_MAP>"PAGE"     {   return TOK_PAGE;                                }

%{ 
/* -------------------------------------------------------------------- */
/*  Keywords that are only valid in the mapping section.                */
/* -------------------------------------------------------------------- */
%} 
<SEC_MC0>"["        {   yyless(0); BEGIN(INITIAL);   /*]*/              }
<SEC_MC0>"@"        {   return TOK_MAC_QUIET;                           }
<SEC_MC0>"0"        {   BEGIN(SEC_MC2); bc_hex=0; return TOK_MAC_REG;   }
<SEC_MC0>"1"        {   BEGIN(SEC_MC2); bc_hex=1; return TOK_MAC_REG;   }
<SEC_MC0>"2"        {   BEGIN(SEC_MC2); bc_hex=2; return TOK_MAC_REG;   }
<SEC_MC0>"3"        {   BEGIN(SEC_MC1); bc_hex=3; return TOK_MAC_REG;   }
<SEC_MC0>"4"        {   BEGIN(SEC_MC2); bc_hex=4; return TOK_MAC_REG;   }
<SEC_MC0>"5"        {   BEGIN(SEC_MC2); bc_hex=5; return TOK_MAC_REG;   }
<SEC_MC0>"6"        {   BEGIN(SEC_MC2); bc_hex=6; return TOK_MAC_REG;   }
<SEC_MC0>"7"        {   BEGIN(SEC_MC2); bc_hex=7; return TOK_MAC_REG;   }
<SEC_MC0>"A"        {   BEGIN(SEC_MC2); return TOK_MAC_AHEAD;           }
<SEC_MC0>"B"        {   BEGIN(SEC_MC2); return TOK_MAC_BLANK;           }
<SEC_MC0>"I"        {   BEGIN(SEC_MC2); return TOK_MAC_INSPECT;         }
<SEC_MC0>"L"        {   BEGIN(SEC_MC1); return TOK_MAC_LOAD;            }
<SEC_MC0>"O"        {   BEGIN(SEC_MC2); return TOK_MAC_RUNTO;           }
<SEC_MC0>"P"        {   BEGIN(SEC_MC2); return TOK_MAC_POKE;            }
<SEC_MC0>"R"        {   BEGIN(SEC_MC2); return TOK_MAC_RUN;             }
<SEC_MC0>"T"        {   BEGIN(SEC_MC2); return TOK_MAC_TRACE;           }
<SEC_MC0>"V"        {   BEGIN(SEC_MC2); return TOK_MAC_VIEW;            }
<SEC_MC0>"W"        {   BEGIN(SEC_MC1); return TOK_MAC_WATCH;           }
<SEC_MC0>;.*        {   /* ignore comments.       */                    }
<SEC_MC0>[ \t\r]*   {   /* ignore whitespace.     */                    }
<SEC_MC0>\n         {   bc_line_no++; return '\n';                      }
<SEC_MC0>.          {   /* ignore unknown lines.  */                    }

%{ 
/* -------------------------------------------------------------------- */
/*  Special secondary, tertiary states for macro processing.  Grrr...   */
/*  We enter this state after parsing the first 'name' on a line.       */
/*  This keeps hex ranges that aren't $ adorned from turning into       */
/*  TOK_NAMEs.  Too much of the grammar comes into the lexer.  :-P      */
/* -------------------------------------------------------------------- */
%}

<SEC_MC1,SEC_MC2>[0-9]+         {   
                                    BEGIN(SEC_MC2);
                                    bc_dec = atoi(yytext);
                                    sscanf(yytext    , "%x", &bc_hex); 
                                    YYTEXT_TO_BC_TXT;
                                    return  TOK_DEC;    
                                }
<SEC_MC1,SEC_MC2>[A-F0-9]+      {   
                                    BEGIN(SEC_MC2);
                                    bc_dec = atoi(yytext);
                                    sscanf(yytext    , "%x", &bc_hex); 
                                    YYTEXT_TO_BC_TXT;
                                    return  TOK_HEX;    
                                }
<SEC_MC1,SEC_MC2>\$[A-F0-9]+    {   
                                    BEGIN(SEC_MC2);
                                    bc_dec = atoi(yytext + 1);
                                    sscanf(yytext + 1, "%x", &bc_hex); 
                                    YYTEXT_TO_BC_TXT;
                                    return  TOK_HEX;    
                                }
<SEC_MC1,SEC_MC2>"-"    {   BEGIN(SEC_MC2); return '-';                 }
<SEC_MC1,SEC_MC2>","    {   BEGIN(SEC_MC2); return ',';                 }
<SEC_MC1,SEC_MC2>;.*    {   BEGIN(SEC_MC2); /* eat comments.  */        }
<SEC_MC1,SEC_MC2>\n     {   bc_line_no++; BEGIN(SEC_MC0); return '\n';  }
<SEC_MC1,SEC_MC2>"["    {   yyless(0); BEGIN(INITIAL);   /*]*/          }

<SEC_MC1>[^ \t\n\r;\[\]\$\=\-\,][^ \t\n\r;\[\]\$]* {   
                                    BEGIN(SEC_MC2);
                                    YYTEXT_TO_BC_TXT;                
                                    return  TOK_NAME;   
                                }

<SEC_MC1,SEC_MC2>[ \t\r]* { /* ignore whitespace. */                    }
<SEC_MC1,SEC_MC2>.      {   YYTEXT_TO_BC_TXT; return TOK_ERROR_BAD;     }

%{ 
/* -------------------------------------------------------------------- */
/*  Main scanner with common rules across most sections.                */
/* -------------------------------------------------------------------- */
%}
-[0-9]+     {   
                bc_dec = atoi(yytext);
                sscanf(yytext + 1, "%x", &bc_hex); 
                YYTEXT_TO_BC_TXT;
                return  TOK_DECONLY;    
            }
[0-9]+      {   
                bc_dec = atoi(yytext);
                sscanf(yytext    , "%x", &bc_hex); 
                YYTEXT_TO_BC_TXT;
                return  TOK_DEC;    
            }
[A-F0-9]+   {   
                bc_dec = atoi(yytext);
                sscanf(yytext    , "%x", &bc_hex); 
                YYTEXT_TO_BC_TXT;
                return  TOK_HEX;    
            }
\$[A-F0-9]+ {   
                bc_dec = atoi(yytext + 1);
                sscanf(yytext + 1, "%x", &bc_hex); 
                YYTEXT_TO_BC_TXT;
                return  TOK_HEX;    
            }
"="         {   return '=';                                             }
"-"         {   return '-';                                             }
","         {   return ',';                                             }
;.*         {   /* eat comments.  */                                    }
\n          {   bc_line_no++;
                if (YY_START == SEC_MC1) BEGIN(SEC_MC0);
                return '\n';        /* comments and newlines are same */
            }

<SEC_VAR>[^ \t\n\r;\[\]\$\=\-\,][^ \t\n\r;\[\]\$]* {   
                YYTEXT_TO_BC_TXT;                
                return  TOK_NAME;   
            }

[ \t\r]+    {   /* eat whitespace */                                    }
[^ \t\n\r\=\-\,\[\]A-Z0-9;]+ { YYTEXT_TO_BC_TXT; return TOK_ERROR_BAD;  }
.                            { YYTEXT_TO_BC_TXT; return TOK_ERROR_BAD;  }

%%

#undef bc_lex

/* We wrap bc_lex with a wrapper that outputs an extra newline at EOF */
int bc_lex(void)
{
    static int need_nl = 1;
    int token = bc_lex1();

    if (token <= 0 && need_nl) 
    { 
        need_nl = 0; 
        return '\n'; 
    }
    if (token > 0) 
    {   
        need_nl = 1; 
    }

    return token;
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

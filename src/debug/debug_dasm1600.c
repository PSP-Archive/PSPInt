
/************************************/
/* CP1600/CP1610 Disassembler       */
/* Version  0.2 by Frank Palazzolo  */
/*                 palazzol@tir.com */
/*                                  */
/* Modified by J. Zbiciak to work   */
/*   with jzIntv.  This is a        */
/*   quickly-hacked version which   */
/*   also fixes a handful of bugs.  */
/************************************/

#include "config.h"
#include "debug_dasm1600.h"

#define R2        "R%c",'0'+i2
#define R3        "R%c",'0'+i3
#define RB        "R%c",'4'+((w2>>8)&3)
#define RR        "R%c",'0'+(i3&3)
#define IM        "#$%04hx",(dbd==1?(w2&0xFF)|(w3<<8):w2)
#define ADDR      "$%04hx",w2

#define DA        "$%04hx",((w2&0xfc)<<8)+(w3&0x3ff)
#define DISPMINUS "$%04hx",addr+1-w2
#define DISPPLUS  "$%04hx",addr+2+w2

#define OPCODE(Q)      sprintf(display_buffer.opcode_field,Q)
#define OPERAND1(Q)    sprintf(display_buffer.operand1_field,Q)
#define OPERAND2(Q)    {sprintf(display_buffer.operand2_field,Q); \
                       sprintf(display_buffer.operanddel_field,",");}

#define MAYBE2_1  if(i3&1)OPERAND1("2")
#define MAYBE2_2  if(i3&4)OPERAND2("2")
#define DISP_1     if(sign)OPERAND1(DISPMINUS);else OPERAND1(DISPPLUS)

static char *branch_code[16] = {
"B   ","BC  ","BOV ","BPL ","BEQ ","BLT ","BLE ","BUSC",
"B   ","BNC ","BNOV","BMI ","BNEQ","BGE ","BGT ","BESC"
};


struct tag_display_buffer
{
    char label_field[10];
    char address_field[10];
    char word1_field[10];
    char word2_field[10];
    char word3_field[10];
    char opcode_field[10];
    char operand1_field[10];
    char operanddel_field[10];
    char operand2_field[10];
    char comment_field[80];
} display_buffer;

static int get_num_words(int op, int dbd)
{
    if (op == 0x4)
        return 3;

    if ((op & 0x3c0) == 0x200)
        return 2;

    if ((op & 0x3f8) == 0x240)
        return 2;

    if ((op & 0x3f8) == 0x278)
        return 2;

    if ((op & 0x238) == 0x200)
        return 2;

    if ((op & 0x238) == 0x238)
        return 2 + (dbd == 1);

    return 1;
}


int dasm1600(char *outbuf, int addr, int dbd, int w1, int w2, int w3)
{
    int i1, i2, i3, sign;
    int s;

    unsigned short labeladdr;


    display_buffer.label_field[0] = 0;
    display_buffer.address_field[0] = 0;
    display_buffer.word1_field[0] = 0;
    display_buffer.word2_field[0] = 0;
    display_buffer.word3_field[0] = 0;
    display_buffer.opcode_field[0] = 0;
    display_buffer.operand1_field[0] = 0;
    display_buffer.operanddel_field[0] = 0;
    display_buffer.operand2_field[0] = 0;
    display_buffer.comment_field[0] = 0;

    sprintf(display_buffer.address_field,"%04hx",addr);
    
    s = get_num_words(w1,dbd);

    switch (s)
    {
        case 1:
        {
            sprintf(display_buffer.word1_field,"%04hx",w1);
            break;
        }
        case 2:
        {
            sprintf(display_buffer.word1_field,"%04hx",w1);
            sprintf(display_buffer.word2_field,"%04hx",w2);
            break;
        }
        case 3:
        {
            sprintf(display_buffer.word1_field,"%04hx",w1);
            sprintf(display_buffer.word2_field,"%04hx",w2);
            sprintf(display_buffer.word3_field,"%04hx",w3);
            break;
        }
    }

    i1 = (w1 & 0x03c0)>>6;
    i2 = (w1 & 0x0038)>>3;
    i3 = w1 & 0x0007;

    switch(i1)
    {
    case 0:
    {
        switch (i2)
        {
        case 0:
        {
            switch (i3)
            {
            case 0:
            {
                OPCODE("HLT");
                break;
            }
            case 1:
            {
                OPCODE("SDBD");
                break;
            }
            case 2:
            {
                OPCODE("EIS");
                break;
            }
            case 3:
            {
                OPCODE("DIS");
                break;
            }
            case 4:
            {
                if ((w2 & 0x300) == 0x300)
                {
                if ((w2 & 3)==0)
                {
                    OPCODE("J");
                    OPERAND1(DA);
                    labeladdr = ((w2&0xfc)<<8)+(w3&0x3ff);
                }
                else if ((w2 & 3)==1)
                {
                    OPCODE("JE");
                    OPERAND1(DA);
                    labeladdr = ((w2&0xfc)<<8)+(w3&0x3ff);
                }
                else if ((w2 & 3)==2)
                {
                    OPCODE("JD");
                    OPERAND1(DA);
                    labeladdr = ((w2&0xfc)<<8)+(w3&0x3ff);
                }
                else
                {
                    OPCODE("Ji");
                    OPERAND1(DA);
                    labeladdr = ((w2&0xfc)<<8)+(w3&0x3ff);
                }
                }
                else
                {
                if ((w2 & 3)==0)
                {
                    OPCODE("JSR");
                    OPERAND1(RB);
                    OPERAND2(DA);
                    labeladdr = ((w2&0xfc)<<8)+(w3&0x3ff);

                }
                else if ((w2 & 3)==1)
                {
                    OPCODE("JSRE");
                    OPERAND1(RB);
                    OPERAND2(DA);
                    labeladdr = ((w2&0xfc)<<8)+(w3&0x3ff);
                }
                else if ((w2 & 3)==2)
                {
                    OPCODE("JSRD");
                    OPERAND1(RB);
                    OPERAND2(DA);
                    labeladdr = ((w2&0xfc)<<8)+(w3&0x3ff);
                }
                else
                {
                    OPCODE("JSRi");
                    OPERAND1(RB);
                    OPERAND2(DA);
                    labeladdr = ((w2&0xfc)<<8)+(w3&0x3ff);
                }
                }

                break;
            }
            case 5:
            {
                OPCODE("TCI");
                break;
            }
            case 6:
            {
                OPCODE("CLRC");
                break;
            }
            case 7:
            {
                OPCODE("SETC");
                break;
            }
            }
            break;
        }
        case 1:
        {
            OPCODE("INCR");
            OPERAND1(R3);
            break;
        }
        case 2:
        {
            OPCODE("DECR");
            OPERAND1(R3);
            break;
        }
        case 3:
        {
            OPCODE("COMR");
            OPERAND1(R3);
            break;
        }
        case 4:
        {
            OPCODE("NEGR");
            OPERAND1(R3);
            break;
        }
        case 5:
        {
            OPCODE("ADCR");
            OPERAND1(R3);
            break;
        }
        case 6:
        {
            if (i3 < 4)
            {
            OPCODE("GSWD");
            OPERAND1(RR);
            }
            else if (i3 < 6)
            {
            OPCODE("NOP");
            MAYBE2_1;
            }
            else
            {
            OPCODE("SIN");
            MAYBE2_1;
            }
            break;
        }
        case 7:
        {
            OPCODE("RSWD");
            OPERAND1(R3);
            break;
        }

        break;
        }
        break;
    }

    case 1:
    {
        switch (i2)
        {
        case 0:
        {
            OPCODE("SWAP");
            OPERAND1(RR);
            MAYBE2_2;
            break;
        }
        case 1:
        {
            OPCODE("SLL");
            OPERAND1(RR);
            MAYBE2_2;
            break;
        }
        case 2:
        {
            OPCODE("RLC");
            OPERAND1(RR);
            MAYBE2_2;
            break;
        }
        case 3:
        {
            OPCODE("SLLC");
            OPERAND1(RR);
            MAYBE2_2;
            break;
        }
        case 4:
        {
            OPCODE("SLR");
            OPERAND1(RR);
            MAYBE2_2;
            break;
        }
        case 5:
        {
            OPCODE("SAR");
            OPERAND1(RR);
            MAYBE2_2;
            break;
        }
        case 6:
        {
            OPCODE("RRC");
            OPERAND1(RR);
            MAYBE2_2;
            break;
        }
        case 7:
        {
            OPCODE("SARC");
            OPERAND1(RR);
            MAYBE2_2;
            break;
        }
        break;
        }
        break;
    }
    case 2:
    {
        if (((w1 & 0x38)>>3) == (w1 & 0x7))
        {
        OPCODE("TSTR");
        OPERAND1(R2);
        }
        else
        if ((w1 & 0x3) == 7)
        {
        OPCODE("JR");
        OPERAND1(R2);
        }
        else
        {
        OPCODE("MOVR");
        OPERAND1(R2);
        OPERAND2(R3);
        }
        break;
    }
    case 3:
    {
        OPCODE("ADDR");
        OPERAND1(R2);
        OPERAND2(R3);
        break;
    }
    case 4:
    {
        OPCODE("SUBR");
        OPERAND1(R2);
        OPERAND2(R3);
        break;
    }
    case 5:
    {
        OPCODE("CMPR");
        OPERAND1(R2);
        OPERAND2(R3);
        break;
    }
    case 6:
    {
        OPCODE("ANDR");
        OPERAND1(R2);
        OPERAND2(R3);
        break;
    }
    case 7:
    {
        if (((w1 & 0x38)>>3) == (w1 & 0x7))
        {
        OPCODE("CLRR");
        OPERAND1(R3);
        }
        else
        {
        OPCODE("XORR");
        OPERAND1(R2);
        OPERAND2(R3);
        }
        break;
    }
    case 8:
    {
        sign = (i2 & 4)>>2;
        if ((w1 & 0x1f) == 0x00)
        {
        OPCODE("B");
        DISP_1;
        if(sign)
            labeladdr = addr+1-w2;
        else
            labeladdr = addr+2+w2;
        }
        else if ((w1 & 0x1f) == 0x8)
        {
        OPCODE("NOPP");
        }
        else if ((w1 & 0x10) == 0x00)
        {
        OPCODE(branch_code[w1&0xf]);
        DISP_1;
        if(sign)
            labeladdr = addr+1-w2;
        else
            labeladdr = addr+2+w2;
        }
        else
        {
        OPCODE("BEXT");
        DISP_1;
        if(sign)
            labeladdr = addr+1-w2;
        else
            labeladdr = addr+2+w2;
        OPERAND2("E");
        }
        break;
    }
    case 9:
    {
        switch(i2)
        {
        case 0:
        {
            OPCODE("MVO");
            OPERAND1(R3);
            OPERAND2(ADDR);
            break;
        }
        case 6:
        {
            OPCODE("PSHR");
            OPERAND1(R3);
            break;
        }
        case 7:
        {
            OPCODE("MVOI");
            OPERAND1(R3);
            OPERAND2(IM);
            break;
        }
        default:
        {
            OPCODE("MVO@");
            OPERAND1(R3);
            OPERAND2(R2);
            break;
        }
        }
        break;
    }
    case 0xa:
    {
        switch(i2)
        {
        case 0:
        {
            OPCODE("MVI");
            OPERAND1(ADDR);
            OPERAND2(R3);
            break;
        }
        case 6:
        {
            OPCODE("PULR");
            OPERAND1(R3);
            break;
        }
        case 7:
        {
            OPCODE("MVII");
            OPERAND1(IM);
            OPERAND2(R3);
            break;
        }
        default:
        {
            OPCODE("MVI@");
            OPERAND1(R2);
            OPERAND2(R3);
            break;
        }
        }
        break;
    }
    case 0xb:
    {
        switch(i2)
        {
        case 0:
        {
            OPCODE("ADD");
            OPERAND1(ADDR);
            OPERAND2(R3);
            break;
        }
        case 7:
        {
            OPCODE("ADDI");
            OPERAND1(IM);
            OPERAND2(R3);
            break;
        }
        default:
        {
            OPCODE("ADD@");
            OPERAND1(R2);
            OPERAND2(R3);
            break;
        }
        }
        break;
    }
    case 0xc:
    {
        switch(i2)
        {
        case 0:
        {
            OPCODE("SUB");
            OPERAND1(ADDR);
            OPERAND2(R3);
            break;
        }
        case 7:
        {
            OPCODE("SUBI");
            OPERAND1(IM);
            OPERAND2(R3);
            break;
        }
        default:
        {
            OPCODE("SUB@");
            OPERAND1(R2);
            OPERAND2(R3);
            break;
        }
        }
        break;
    }
    case 0xd:
    {
        switch(i2)
        {
        case 0:
        {
            OPCODE("CMP");
            OPERAND1(ADDR);
            OPERAND2(R3);
            break;
        }
        case 7:
        {
            OPCODE("CMPI");
            OPERAND1(IM);
            OPERAND2(R3);
            break;
        }
        default:
        {
            OPCODE("CMP@");
            OPERAND1(R2);
            OPERAND2(R3);
            break;
        }
        }
        break;
    }
    case 0xe:
    {
        switch(i2)
        {
        case 0:
        {
            OPCODE("AND");
            OPERAND1(ADDR);
            OPERAND2(R3);
            break;
        }
        case 7:
        {
            OPCODE("ANDI");
            OPERAND1(IM);
            OPERAND2(R3);
            break;
        }
        default:
        {
            OPCODE("AND@");
            OPERAND1(R2);
            OPERAND2(R3);
            break;
        }
        }
        break;
    }
    case 0xf:
    {
        switch(i2)
        {
        case 0:
        {
            OPCODE("XOR");
            OPERAND1(ADDR);
            OPERAND2(R3);
            break;
        }
        case 7:
        {
            OPCODE("XORI");
            OPERAND1(IM);
            OPERAND2(R3);
            break;
        }
        default:
        {
            OPCODE("XOR@");
            OPERAND1(R2);
            OPERAND2(R3);
            break;
        }
        }
        break;
    }
    break;
    }

    sprintf(outbuf,"%-10s $%4s  %4s %4s %4s         %-4s %s%1s%s    %s",
        display_buffer.label_field,
        display_buffer.address_field,
        display_buffer.word1_field,
        display_buffer.word2_field,
        display_buffer.word3_field,
        display_buffer.opcode_field,
        display_buffer.operand1_field,
        display_buffer.operanddel_field,
        display_buffer.operand2_field,
        display_buffer.comment_field);

    return s;
}



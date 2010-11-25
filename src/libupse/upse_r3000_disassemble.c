/*
 * UPSE: the unix playstation sound emulator.
 *
 * Filename: upse_r3000_disassemble.c
 * Purpose: libupse: R3000 disassembler.
 *
 * Copyright (c) 2010 William Pitcock <nenolod@dereferenced.org>
 *
 * UPSE is free software, released under the GNU General Public License,
 * version 2.
 *
 * A copy of the GNU General Public License, version 2, is included in
 * the UPSE source kit as COPYING.
 *
 * UPSE is offered without any warranty of any kind, explicit or implicit.
 */

#include "upse-internal.h"

static const char *regname[32] = {
    "$0" , "$at", "$v0", "$v1", "$a0", "$a1", "$a2", "$a3",
    "$t0", "$t1", "$t2", "$t3", "$t4", "$t5", "$t6", "$t7",
    "$s0", "$s1", "$s2", "$s3", "$s4", "$s5", "$s6", "$s7",
    "$t8", "$t9", "$k0", "$k1", "$gp", "$sp", "$fp", "$ra"
};

static const char *C0regname[32] = {
    "C0_0", "C0_1", "C0_2", "C0_3", "C0_4", "C0_5", "C0_6", "C0_7",
    "C0_8", "C0_9", "C0_10", "C0_11", "C0_status", "C0_cause", "C0_epc", "C0_prid",
    "C0_16", "C0_17", "C0_18", "C0_19", "C0_20", "C0_21", "C0_22", "C0_23",
    "C0_24", "C0_25", "C0_26", "C0_27", "C0_28", "C0_29", "C0_30", "C0_31"
};

#define INSN_S(insn) (u32)(((insn) >> 21) & 0x1F)
#define INSN_T(insn) (u32)(((insn) >> 16) & 0x1F)
#define INSN_D(insn) (u32)(((insn) >> 11) & 0x1F)
#define INSN_H(insn) (u32)(((insn) >> 6) & 0x1F)
#define INSN_I(insn) (u32)((insn) & 0xFFFF)

#define S16(x) ((s32)(((s16)(x))))

#define REL_I(pc, insn) ((pc) + 4 + (((s32)(S16(INSN_I(insn)))) << 2))
#define ABS_I(pc, insn) (((pc) & 0xF0000000) | (((insn) << 2) & 0x0FFFFFFC))

/*
 * upse_r3000_disassemble_insn()
 */
int
upse_r3000_disassemble_insn(char *buf, int bufsize, u32 pc, u32 insn)
{
    int delayslot;
    const char *fmt;
    int col, j;

    if (insn < 0x04000000)
    {
        switch (insn & 0x3F)
        {
        case 0x00:
            if (!insn)
            {
                fmt = "nop";
                goto ok;
            }

            if (INSN_D(insn) == INSN_T(insn))
            {
                fmt = "sll D,H";
                goto ok;
            }

            if (!INSN_H(insn))
            {
                fmt = "move D,T";
                goto ok;
            }

            fmt = "sll D,T,H";
            goto ok;

        case 0x02:
            if (INSN_D(insn) == INSN_T(insn))
            {
                fmt = "srl D,H";
                goto ok;
            }

            if (!INSN_H(insn))
            {
                fmt = "move D,T";
                goto ok;
            }

            fmt = "srl D,T,H";
            goto ok;

        case 0x03:
            if (INSN_D(insn) == INSN_T(insn))
            {
                fmt = "sra D,H";
                goto ok;
            }

            if (!INSN_H(insn))
            {
                fmt = "move D,T";
                goto ok;
            }

            fmt = "sra D,T,H";
            goto ok;

        case 0x04:
            if (INSN_D(insn) == INSN_T(insn))
            {
                fmt = "sllv D,S";
                goto ok;
            }

            if (!INSN_S(insn))
            {
                fmt = "move D,T";
                goto ok;
            }

            fmt = "sllv D,T,S";
            goto ok;

        case 0x06:
            if (INSN_D(insn) == INSN_T(insn))
            {
                fmt = "srlv D,S";
                goto ok;
            }

            if (!INSN_S(insn))
            {
                fmt = "move D,T";
                goto ok;
            }

            fmt = "srlv D,T,S";
            goto ok;

        case 0x07:
            if (INSN_D(insn) == INSN_T(insn))
            {
                fmt = "srav D,S";
                goto ok;
            }

            if (!INSN_S(insn))
            {
                fmt = "move D,T";
                goto ok;
            }

            fmt = "srav D,T,S";
            goto ok;

        case 0x08:
            fmt = "jr S";
            goto ok_delayslot;

        case 0x09:
            if (INSN_D(insn) == 31)
            {
                fmt = "jalr S";
                goto ok_delayslot;
            }

            fmt = "jalr D,S";
            goto ok_delayslot;

        case 0x0C:
            fmt = "syscall";
            goto ok;

        case 0x10:
            fmt = "mfhi D";
            goto ok;

        case 0x11:
            fmt = "mthi S";
            goto ok;

        case 0x12:
            fmt = "mflo D";
            goto ok;

        case 0x13:
            fmt = "mtlo S";
            goto ok;

        case 0x18:
            fmt = "mult S,T";
            goto ok;

        case 0x19:
            fmt = "multu S,T";
            goto ok;

        case 0x1A:
            fmt = "div S,T";
            goto ok;

        case 0x1B:
            fmt = "divu S,T";
            goto ok;

        case 0x20:
            if (INSN_D(insn) == INSN_S(insn))
            {
                fmt = "add D,T";
                goto ok;
            }

            if (!INSN_T(insn))
            {
                fmt = "move D,S";
                goto ok;
            }

            if (!INSN_S(insn))
            {
                fmt = "move D,T";
                goto ok;
            }

            fmt = "add D,S,T";
            goto ok;

        case 0x21:
            if (INSN_D(insn) == INSN_S(insn))
            {
                fmt = "addu D,T";
                goto ok;
            }

            if (!INSN_T(insn))
            {
                fmt = "move D,S";
                goto ok;
            }

            if (!INSN_S(insn))
            {
                fmt = "move D,T";
                goto ok;
            }

            fmt = "addu D,S,T";
            goto ok;

        case 0x22:
            if (INSN_D(insn) == INSN_S(insn))
            {
                fmt = "sub D,T";
                goto ok;
            }

            if (!INSN_T(insn))
            {
                fmt = "move D,S";
                goto ok;
            }

            if (!INSN_S(insn))
            {
                if (INSN_D(insn) == INSN_T(insn))
                {
                    fmt = "neg D";
                    goto ok;
                }

                fmt = "neg D,T";
                goto ok;
            }

            fmt = "sub D,S,T";
            goto ok;

        case 0x23:
            if (INSN_D(insn) == INSN_S(insn))
            {
                fmt = "subu D,T";
                goto ok;
            }

            if (!INSN_T(insn))
            {
                fmt = "move D,S";
                goto ok;
            }

            if (!INSN_S(insn))
            {
                if (INSN_D(insn) == INSN_T(insn))
                {
                    fmt = "negu D";
                    goto ok;
                }

                fmt = "negu D,T";
                goto ok;
            }

        case 0x24:
            if (INSN_D(insn) == INSN_S(insn))
            {
                fmt = "and D,T";
                goto ok;
            }

            fmt = "and D,S,T";
            goto ok;

        case 0x25:
            if (INSN_D(insn) == INSN_S(insn))
            {
                fmt = "or D,T";
                goto ok;
            }

            if (!INSN_T(insn))
            {
                fmt = "move D,S";
                goto ok;
            }

            fmt = "or D,S,T";
            goto ok;

        case 0x26:
            if (INSN_D(insn) == INSN_S(insn))
            {
                fmt = "xor D,T";
                goto ok;
            }

            if (!INSN_T(insn))
            {
                fmt = "move D,S";
                goto ok;
            }

            fmt = "xor D,S,T";
            goto ok;

        case 0x27:
            if (!INSN_T(insn))
            {
                if (INSN_D(insn) == INSN_S(insn))
                {
                    fmt = "not D";
                    goto ok;
                }

                fmt = "not D,S";
                goto ok;
            }
            else if (!INSN_S(insn))
            {
                if (INSN_D(insn) == INSN_T(insn))
                {
                    fmt = "not D";
                    goto ok;
                }

                fmt = "not D,T";
                goto ok;
            }
            else
            {
                if (INSN_D(insn) == INSN_S(insn))
                {
                    fmt = "nor D,T";
                    goto ok;
                }

                fmt = "nor D,S,T";
                goto ok;
            }
        case 0x2A:
            fmt = "slt D,S,T";
            goto ok;

        case 0x2B:
            fmt = "sltu D,S,T";
            goto ok;

        default:
            goto invalid;
        }
    }
    else
    {
        switch (insn >> 26)
        {
        case 0x01:
            switch(INSN_T(insn))
            {
                case 0x00:
                    fmt = "bltz S,B";
                    goto ok_delayslot;

                case 0x01:
                    fmt = "bgez S,B";
                    goto ok_delayslot;

                case 0x10:
                    fmt = "bltzal S,B";
                    goto ok_delayslot;

                case 0x11:
                    fmt = "bgezal S,B";
                    goto ok_delayslot;

                default:
                    goto invalid;
            }
            break;

        case 0x02:
            fmt = "j J";
            goto ok_delayslot;
 
        case 0x03:
            fmt = "jal J";
            goto ok_delayslot;

        case 0x04:
            if (INSN_S(insn) == INSN_T(insn))
            {
                fmt = "b B";
                goto ok_delayslot;
            }

            fmt = "beq S,T,B";
            goto ok_delayslot;

        case 0x05:
            fmt = "bne S,T,B";
            goto ok_delayslot;

        case 0x06:
            if (!INSN_S(insn))
            {
                fmt = "b B";
                goto ok_delayslot;
            }

            fmt = "blez S,B";
            goto ok_delayslot;

        case 0x07:
            fmt = "bgtz S,B";
            goto ok_delayslot;

        case 0x08:
            if (INSN_T(insn) == INSN_S(insn))
            {
                fmt = "addi T,I";
                goto ok;
            }

            if (!INSN_S(insn))
            {
                fmt = "li T,I";
                goto ok;
            }

            if (!INSN_I(insn))
            {
                fmt = "move T,S";
                goto ok;
            }

            fmt = "addi T,S,I";
            goto ok;

        case 0x09:
            if (INSN_T(insn) == INSN_S(insn))
            {
                fmt = "addiu T,I";
                goto ok;
            }

            if (!INSN_S(insn))
            {
                fmt = "li T,I";
                goto ok;
            }

            if (!INSN_I(insn))
            {
                fmt = "move T,S";
                goto ok;
            }

            fmt = "addiu T,S,I";
            goto ok;

        case 0x0A:
            fmt = "slti T,S,I";
            goto ok;

        case 0x0B:
            fmt = "sltiu T,S,I";
            goto ok;

        case 0x0C:
            if (INSN_T(insn) == INSN_S(insn))
            {
                fmt = "andi T,I";
                goto ok;
            }

            fmt = "andi T,S,I";
            goto ok;

        case 0x0D:
            if (INSN_T(insn) == INSN_S(insn))
            {
                fmt = "ori T,I";
                goto ok;
            }

            fmt = "ori T,S,I";
            goto ok;

        case 0x0E:
            if (INSN_T(insn) == INSN_S(insn))
            {
                fmt = "xori T,I";
                goto ok;
            }

            fmt = "xori T,S,I";
            goto ok;

        case 0x0F:
            fmt = "lui T,I";
            goto ok;

        case 0x10:
            switch (INSN_S(insn))
            {
            case 0x00:
                fmt = "mfc0 T,CD";
                goto ok;

            case 0x04:
                fmt = "mtc0 T,CD";
                goto ok;

            case 0x10:
                fmt = "rfe";
                goto ok;

            default:
                goto invalid;
            }

            break;

        case 0x20:
            if (!INSN_I(insn))
            {
                fmt = "lb T,(S)";
                goto ok;
            }

            fmt = "lb T,I(S)";
            goto ok;

        case 0x21:
            if (!INSN_I(insn))
            {
                fmt = "lh T,(S)";
                goto ok;
            }

            fmt = "lh T,I(S)";
            goto ok;

        case 0x22:
            if (!INSN_I(insn))
            {
                fmt = "lwl T,(S)";
                goto ok;
            }

            fmt = "lwl T,I(S)";
            goto ok;

        case 0x23:
            if (!INSN_I(insn))
            {
                fmt = "lw T,(S)";
                goto ok;
            }

            fmt = "lw T,I(S)";
            goto ok;

        case 0x24:
            if (!INSN_I(insn))
            {
                fmt = "lbu T,(S)";
                goto ok;
            }

            fmt = "lbu T,I(S)";
            goto ok;

        case 0x25:
            if (!INSN_I(insn))
            {
                fmt = "lhu T,(S)";
                goto ok;
            }

            fmt = "lhu T,I(S)";
            goto ok;

        case 0x26:
            if (!INSN_I(insn))
            {
                fmt = "lwr T,(S)";
                goto ok;
            }

            fmt = "lwr T,I(S)";
            goto ok;

        case 0x28:
            if (!INSN_I(insn))
            {
                fmt = "sb T,(S)";
                goto ok;
            }

            fmt = "sb T,I(S)";
            goto ok;

        case 0x29:
            if (!INSN_I(insn))
            {
                fmt = "sh T,(S)";
                goto ok;
            }

            fmt = "sh T,I(S)";
            goto ok;

        case 0x2A:
            if (!INSN_I(insn))
            {
                fmt = "swl T,(S)";
                goto ok;
            }

            fmt = "swl T,I(S)";
            goto ok;

        case 0x2B:
            if (!INSN_I(insn))
            {
                fmt = "sw T,(S)";
                goto ok;
            }

            fmt = "sw T,I(S)";
            goto ok;

        case 0x2E:
            if (!INSN_I(insn))
            {
                fmt = "swr T,(S)";
                goto ok;
            }

            fmt = "swr T,I(S)";
            goto ok;

        default:
            goto invalid;
        }
    }

ok_delayslot:
    delayslot = 1;

ok:
    for (col = 0; *fmt && col < bufsize; fmt++)
    {
        switch (*fmt)
        {
        case 'S':
            strcpy(buf + col, regname[INSN_S(insn)]);
            while (buf[col])
                col++;

            break;

        case 'T':
            strcpy(buf + col, regname[INSN_T(insn)]);
            while (buf[col])
                col++;

            break;
        case 'D':
            strcpy(buf + col, regname[INSN_D(insn)]);
            while (buf[col])
                col++;

            break;

        case 'H':
            sprintf(buf + col, "%d", INSN_H(insn));
            while (buf[col])
                col++;

            break;

        case 'I':
            sprintf(buf + col, "0x%04X", INSN_I(insn));
            while (buf[col])
                col++;

            break;

        case 'B':
            sprintf(buf + col, "0x%08X", REL_I(pc, insn));
            while (buf[col])
                col++;

            break;

        case 'J':
            sprintf(buf + col, "0x%08X", ABS_I(pc, insn));
            while (buf[col])
                col++;

            break;

        case 'C':
            fmt++;
            j = 0;

            switch(*fmt)
            {
                case 'D':
                    j = INSN_D(insn);
                    break;
            }

            strcpy(buf + col, C0regname[j]);
            while (buf[col])
                col++;

            break;
        default:
            buf[col++] = *fmt;
            break;
        }
    }

    buf[col] = 0;
    return delayslot ? 1 : 0;

invalid:
    return -1;
}

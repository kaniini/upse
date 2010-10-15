/*
 * UPSE: the unix playstation sound emulator.
 *
 * Filename: upse_ps1_bios_base.c
 * Purpose: libupse: UPSE PS1 FakeBIOS base code
 *
 * Copyright (c) 2007 William Pitcock <nenolod@sacredspiral.co.uk>
 * Portions copyright (c) 1999-2002 Pcsx Team
 * Portions copyright (c) 2004 "Xodnizel"
 *
 * UPSE is free software, released under the GNU General Public License,
 * version 2.
 *
 * A copy of the GNU General Public License, version 2, is included in
 * the UPSE source kit as COPYING.
 *
 * UPSE is offered without any warranty of any kind, explicit or implicit.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

#include "upse-internal.h"
#include "upse-ps1-bios-private.h"

//We try to emulate bios :) HELP US :P

char *biosA0n[256] = {
	/* 0x00 */	"open",
	/* 0x01 */	"lseek",
	/* 0x02 */	"read",
	/* 0x03 */	"write",
	/* 0x04 */	"close",
	/* 0x05 */	"ioctl",
	/* 0x06 */	"exit",
	/* 0x07 */	"bios_a0_07",
	/* 0x08 */	"getc",
	/* 0x09 */	"putc",
	/* 0x0a */	"todigit",
	/* 0x0b */	"atof",
	/* 0x0c */	"strtoul",
	/* 0x0d */	"strtol",
	/* 0x0e */	"abs",
	/* 0x0f */	"labs",
	/* 0x10 */	"atoi",
	/* 0x11 */	"atol",
	/* 0x12 */	"atob",
	/* 0x13 */	"setjmp",
	/* 0x14 */	"longjmp",
	/* 0x15 */	"strcat",
	/* 0x16 */	"strncat",
	/* 0x17 */	"strcmp",
	/* 0x18 */	"strncmp",
	/* 0x19 */	"strcpy",
	/* 0x1a */	"strnpy",
	/* 0x1b */	"strlen",
	/* 0x1c */	"index",
	/* 0x1d */	"rindex",
	/* 0x1e */	"strchr",
	/* 0x1f */	"strrchr",
	/* 0x20 */	"strpbrk",
	/* 0x21 */	"strspn",
	/* 0x22 */	"strcspn",
	/* 0x23 */	"strtok",
	/* 0x24 */	"strstr",
	/* 0x25 */	"toupper",
	/* 0x26 */	"tolower",
	/* 0x27 */	"bcopy",
	/* 0x28 */	"bzero",
	/* 0x29 */	"bcmp",
	/* 0x2a */	"memcpy",
	/* 0x2b */	"memset",
	/* 0x2c */	"memmove",
	/* 0x2d */	"memcmp",
	/* 0x2e */	"memchr",
	/* 0x2f */	"rand",
	/* 0x30 */	"srand",
	/* 0x31 */	"qsort",
	/* 0x32 */	"strtod",
	/* 0x33 */	"malloc",
	/* 0x34 */	"free",
	/* 0x35 */	"lsearch",
	/* 0x36 */	"bsearch",
	/* 0x37 */	"calloc",
	/* 0x38 */	"realloc",
	/* 0x39 */	"InitHeap",
	/* 0x3a */	"_exit",
	/* 0x3b */	"getchar",
	/* 0x3c */	"putchar",
	/* 0x3d */	"gets",
	/* 0x3e */	"puts",
	/* 0x3f */	"printf",
	/* 0x40 */	"bios_a0_40",
	/* 0x41 */	"LoadTest",
	/* 0x42 */	"Load",
	/* 0x43 */	"Exec",
	/* 0x44 */	"FlushCache",
	/* 0x45 */	"InstallInterruptHandler",
	/* 0x46 */	"GPU_dw",
	/* 0x47 */	"mem2vram",
	/* 0x48 */	"SendGPU",
	/* 0x49 */	"GPU_cw",
	/* 0x4a */	"GPU_cwb",
	/* 0x4b */	"SendPackets",
	/* 0x4c */	"bios_a0_4c",
	/* 0x4d */	"GetGPUStatus",
	/* 0x4e */	"GPU_sync",
	/* 0x4f */	"bios_a0_4f",
	/* 0x50 */	"bios_a0_50",
	/* 0x51 */	"LoadExec",
	/* 0x52 */	"GetSysSp",
	/* 0x53 */	"bios_a0_53",
	/* 0x54 */	"_96_init",
	/* 0x55 */	"_bu_init",
	/* 0x56 */	"_96_remove",
	/* 0x57 */	"bios_a0_57",
	/* 0x58 */	"bios_a0_58",
	/* 0x59 */	"bios_a0_59",
	/* 0x5a */	"bios_a0_5a",
	/* 0x5b */	"dev_tty_init",
	/* 0x5c */	"dev_tty_open",
	/* 0x5d */	"dev_tty_5d",
	/* 0x5e */	"dev_tty_ioctl",
	/* 0x5f */	"dev_cd_open",
	/* 0x60 */	"dev_cd_read",
	/* 0x61 */	"dev_cd_close",
	/* 0x62 */	"dev_cd_firstfile",
	/* 0x63 */	"dev_cd_nextfile",
	/* 0x64 */	"dev_cd_chdir",
	/* 0x65 */	"dev_card_open",
	/* 0x66 */	"dev_card_read",
	/* 0x67 */	"dev_card_write",
	/* 0x68 */	"dev_card_close",
	/* 0x69 */	"dev_card_firstfile",
	/* 0x6a */	"dev_card_nextfile",
	/* 0x6b */	"dev_card_erase",
	/* 0x6c */	"dev_card_undelete",
	/* 0x6d */	"dev_card_format",
	/* 0x6e */	"dev_card_rename",
	/* 0x6f */	"dev_card_6f",
	/* 0x70 */	"_bu_init(a0_70)",
	/* 0x71 */	"_96_init(a0_71)",
	/* 0x72 */	"_96_remove(a0_72)",
	/* 0x73 */	"bios_a0_73",
	/* 0x74 */	"bios_a0_74",
	/* 0x75 */	"bios_a0_75",
	/* 0x76 */	"bios_a0_76",
	/* 0x77 */	"bios_a0_77",
	/* 0x78 */	"_96_CdSeekL",
	/* 0x79 */	"bios_a0_79",
	/* 0x7a */	"bios_a0_7a",
	/* 0x7b */	"bios_a0_7b",
	/* 0x7c */	"_96_CdGetStatus",
	/* 0x7d */	"bios_a0_7d",
	/* 0x7e */	"_96_CdRead",
	/* 0x7f */	"bios_a0_7f",
	/* 0x80 */	"bios_a0_80",
	/* 0x81 */	"bios_a0_81",
	/* 0x82 */	"bios_a0_82",
	/* 0x83 */	"bios_a0_83",
	/* 0x84 */	"bios_a0_84",
	/* 0x85 */	"_96_CdStop",
	/* 0x86 */	"bios_a0_86",
	/* 0x87 */	"bios_a0_87",
	/* 0x88 */	"bios_a0_88",
	/* 0x89 */	"bios_a0_89",
	/* 0x8a */	"bios_a0_8a",
	/* 0x8b */	"bios_a0_8b",
	/* 0x8c */	"bios_a0_8c",
	/* 0x8d */	"bios_a0_8d",
	/* 0x8e */	"bios_a0_8e",
	/* 0x8f */	"bios_a0_8f",
	/* 0x90 */	"bios_a0_90",
	/* 0x91 */	"bios_a0_91",
	/* 0x92 */	"bios_a0_92",
	/* 0x93 */	"bios_a0_93",
	/* 0x94 */	"bios_a0_94",
	/* 0x95 */	"bios_a0_95",
	/* 0x96 */	"AddCDROMDevice",
	/* 0x97 */	"AddMemCardDevice",
	/* 0x98 */	"DisableKernelIORedirection",
	/* 0x99 */	"EnableKernelIORedirection",
	/* 0x9a */	"bios_a0_9a",
	/* 0x9b */	"bios_a0_9b",
	/* 0x9c */	"SetConf",
	/* 0x9d */	"GetConf",
	/* 0x9e */	"bios_a0_9e",
	/* 0x9f */	"SetMem",
	/* 0xa0 */	"_boot",
	/* 0xa1 */	"SystemError",
	/* 0xa2 */	"EnqueueCdIntr",
	/* 0xa3 */	"DequeueCdIntr",
	/* 0xa4 */	"bios_a0_a4",
	/* 0xa5 */	"ReadSector",
	/* 0xa6 */	"get_cd_status",
	/* 0xa7 */	"bufs_cb_0",
	/* 0xa8 */	"bufs_cb_1",
	/* 0xa9 */	"bufs_cb_2",
	/* 0xaa */	"bufs_cb_3",
	/* 0xab */	"_card_info",
	/* 0xac */	"_card_load",
	/* 0xad */	"_card_auto",
	/* 0xae */	"bufs_cb_4",
	/* 0xaf */	"bios_a0_af",
	/* 0xb0 */	"bios_a0_b0",
	/* 0xb1 */	"bios_a0_b1",
	/* 0xb2 */	"do_a_long_jmp",
	/* 0xb3 */	"bios_a0_b3",
	/* 0xb4 */	"bios_a0_b4",
};

char *biosB0n[256] = {
	/* 0x00 */	"SysMalloc",
	/* 0x01 */	"bios_b0_01",
	/* 0x02 */	"bios_b0_02",
	/* 0x03 */	"bios_b0_03",
	/* 0x04 */	"bios_b0_04",
	/* 0x05 */	"bios_b0_05",
	/* 0x06 */	"bios_b0_06",
	/* 0x07 */	"DeliverEvent",
	/* 0x08 */	"OpenEvent",
	/* 0x09 */	"CloseEvent",
	/* 0x0a */	"WaitEvent",
	/* 0x0b */	"TestEvent",
	/* 0x0c */	"EnableEvent",
	/* 0x0d */	"DisableEvent",
	/* 0x0e */	"OpenTh",
	/* 0x0f */	"CloseTh",
	/* 0x10 */	"ChangeTh",
	/* 0x11 */	"bios_b0_11",
	/* 0x12 */	"InitPAD",
	/* 0x13 */	"StartPAD",
	/* 0x14 */	"StopPAD",
	/* 0x15 */	"PAD_init",
	/* 0x16 */	"PAD_dr",
	/* 0x17 */	"ReturnFromException",
	/* 0x18 */	"ResetEntryInt",
	/* 0x19 */	"HookEntryInt",
	/* 0x1a */	"bios_b0_1a",
	/* 0x1b */	"bios_b0_1b",
	/* 0x1c */	"bios_b0_1c",
	/* 0x1d */	"bios_b0_1d",
	/* 0x1e */	"bios_b0_1e",
	/* 0x1f */	"bios_b0_1f",
	/* 0x20 */	"UnDeliverEvent",
	/* 0x21 */	"bios_b0_21",
	/* 0x22 */	"bios_b0_22",
	/* 0x23 */	"bios_b0_23",
	/* 0x24 */	"bios_b0_24",
	/* 0x25 */	"bios_b0_25",
	/* 0x26 */	"bios_b0_26",
	/* 0x27 */	"bios_b0_27",
	/* 0x28 */	"bios_b0_28",
	/* 0x29 */	"bios_b0_29",
	/* 0x2a */	"bios_b0_2a",
	/* 0x2b */	"bios_b0_2b",
	/* 0x2c */	"bios_b0_2c",
	/* 0x2d */	"bios_b0_2d",
	/* 0x2e */	"bios_b0_2e",
	/* 0x2f */	"bios_b0_2f",
	/* 0x30 */	"bios_b0_30",
	/* 0x31 */	"bios_b0_31",
	/* 0x32 */	"open(b0)",
	/* 0x33 */	"lseek(b0)",
	/* 0x34 */	"read(b0)",
	/* 0x35 */	"write(b0)",
	/* 0x36 */	"close(b0)",
	/* 0x37 */	"ioctl(b0)",
	/* 0x38 */	"exit(b0)",
	/* 0x39 */	"bios_b0_39",
	/* 0x3a */	"getc(b0)",
	/* 0x3b */	"putc(b0)",
	/* 0x3c */	"getchar(b0)",
	/* 0x3d */	"putchar(b0)",
	/* 0x3e */	"gets(b0)",
	/* 0x3f */	"puts(b0)",
	/* 0x40 */	"cd",
	/* 0x41 */	"format",
	/* 0x42 */	"firstfile",
	/* 0x43 */	"nextfile",
	/* 0x44 */	"rename",
	/* 0x45 */	"delete",
	/* 0x46 */	"undelete",
	/* 0x47 */	"AddDevice",
	/* 0x48 */	"RemoteDevice",
	/* 0x49 */	"PrintInstalledDevices",
	/* 0x4a */	"InitCARD",
	/* 0x4b */	"StartCARD",
	/* 0x4c */	"StopCARD",
	/* 0x4d */	"bios_b0_4d",
	/* 0x4e */	"_card_write",
	/* 0x4f */	"_card_read",
	/* 0x50 */	"_new_card",
	/* 0x51 */	"Krom2RawAdd",
	/* 0x52 */	"bios_b0_52",
	/* 0x53 */	"bios_b0_53",
	/* 0x54 */	"_get_errno",
	/* 0x55 */	"_get_error",
	/* 0x56 */	"GetC0Table",
	/* 0x57 */	"GetB0Table",
	/* 0x58 */	"_card_chan",
	/* 0x59 */	"bios_b0_59",
	/* 0x5a */	"bios_b0_5a",
	/* 0x5b */	"ChangeClearPAD",
	/* 0x5c */	"_card_status",
	/* 0x5d */	"_card_wait",
};

char *biosC0n[256] = {
	/* 0x00 */	"InitRCnt",
	/* 0x01 */	"InitException",
	/* 0x02 */	"SysEnqIntRP",
	/* 0x03 */	"SysDeqIntRP",
	/* 0x04 */	"get_free_EvCB_slot",
	/* 0x05 */	"get_free_TCB_slot",
	/* 0x06 */	"ExceptionHandler",
	/* 0x07 */	"InstallExceptionHandler",
	/* 0x08 */	"SysInitMemory",
	/* 0x09 */	"SysInitKMem",
	/* 0x0a */	"ChangeClearRCnt",
	/* 0x0b */	"SystemError(c0)",
	/* 0x0c */	"InitDefInt",
	/* 0x0d */	"bios_c0_0d",
	/* 0x0e */	"bios_c0_0e",
	/* 0x0f */	"bios_c0_0f",
	/* 0x10 */	"bios_c0_10",
	/* 0x11 */	"bios_c0_11",
	/* 0x12 */	"InstallDevices",
	/* 0x13 */	"FlushStdInOutPut",
	/* 0x14 */	"bios_c0_14",
	/* 0x15 */	"_cdevinput",
	/* 0x16 */	"_cdevscan",
	/* 0x17 */	"_circgetc",
	/* 0x18 */	"_circputc",
	/* 0x19 */	"ioabort",
	/* 0x1a */	"bios_c0_1a",
	/* 0x1b */	"KernelRedirect",
	/* 0x1c */	"PatchAOTable",
};

/* *INDENT-ON* */

typedef struct _malloc_chunk
{
    u32 stat;
    u32 size;
    u32 fd;
    u32 bk;
} upse_packed_t malloc_chunk;

#define INUSE 0x1

typedef struct
{
    u32 desc;
    s32 status;
    s32 mode;
    u32 fhandler;
} upse_packed_t EvCB[32];

#define EvStUNUSED	0x0000
#define EvStWAIT	0x1000
#define EvStACTIVE	0x2000
#define EvStALREADY 0x4000

#define EvMdINTR	0x1000
#define EvMdNOINTR	0x2000

typedef struct
{
    s32 status;
    s32 mode;
    u32 reg[32];
    u32 func;
} upse_packed_t TCB;

static u32 *jmp_int;

static u32 regs[35];
static EvCB *Event;

//static EvCB *HwEV; // 0xf0
//static EvCB *EvEV; // 0xf1
static EvCB *RcEV;		// 0xf2
//static EvCB *UeEV; // 0xf3
//static EvCB *SwEV; // 0xf4
//static EvCB *ThEV; // 0xff

static u32 heap_addr;
static u32 SysIntRP[8];
static TCB Thread[8];
static int CurThread;

static INLINE void softCall(u32 pc)
{
    pc0 = pc;
    ra = 0x80001000;
    while (pc0 != 0x80001000)
	upse_r3000_cpu_execute_block();
}

static INLINE void softCall2(u32 pc)
{
    u32 sra = ra;
    pc0 = pc;
    ra = 0x80001000;
    while (pc0 != 0x80001000)
	upse_r3000_cpu_execute_block();
    ra = sra;
}

static INLINE void DeliverEvent(u32 ev, u32 spec)
{
    if (Event[ev][spec].status != BFLIP32S(EvStACTIVE))
	return;

//      Event[ev][spec].status = BFLIP32S(EvStALREADY);
    if (Event[ev][spec].mode == BFLIP32S(EvMdINTR))
    {
	softCall2(BFLIP32S(Event[ev][spec].fhandler));
    }
    else
	Event[ev][spec].status = BFLIP32S(EvStALREADY);
}

/*                                           *
//                                           *
//                                           *
//               System calls A0             */

/* Abs and labs do the same thing? */

static void bios_abs()
{				// 0x0e
    if ((s32) a0 < 0)
	v0 = 0 - (s32) a0;
    else
	v0 = a0;
    //v0 = abs(a0);
    pc0 = ra;
}

static void bios_labs()
{				// 0x0f
    if ((s32) a0 < 0)
	v0 = 0 - (s32) a0;
    else
	v0 = a0;
    //v0 = labs(a0);
    pc0 = ra;
}

static void bios_atoi()
{				// 0x10
    char *arg = Ra0;
    v0 = atoi(arg);
    pc0 = ra;
}

static void bios_atol()
{				// 0x11
    char *arg = Ra0;
    v0 = atol(arg);
    pc0 = ra;
}

static void bios_setjmp()
{				// 13
    u32 *jmp_buf = (u32 *) Ra0;
    int i;

    jmp_buf[0] = BFLIP32(ra);
    jmp_buf[1] = BFLIP32(sp);
    jmp_buf[2] = BFLIP32(fp);
    for (i = 0; i < 8; i++)	// s0-s7
	jmp_buf[3 + i] = BFLIP32(upse_r3000_cpu_regs.GPR.r[16 + i]);
    jmp_buf[11] = BFLIP32(gp);

    v0 = 0;
    pc0 = ra;
}

static void bios_longjmp()
{				//14
    u32 *jmp_buf = (u32 *) Ra0;
    int i;

    ra = BFLIP32(jmp_buf[0]);	/* ra */
    sp = BFLIP32(jmp_buf[1]);	/* sp */
    fp = BFLIP32(jmp_buf[2]);	/* fp */
    for (i = 0; i < 8; i++)	// s0-s7
	upse_r3000_cpu_regs.GPR.r[16 + i] = BFLIP32(jmp_buf[3 + i]);
    gp = BFLIP32(jmp_buf[11]);	/* gp */

    v0 = a1;
    pc0 = ra;
}

static void bios_strcat()
{				// 0x15
    char *pcA0 = (char *) Ra0;
    char *pcA1 = (char *) Ra1;

    if (pcA0 == NULL || pcA1 == NULL)
	return;

    strcat(pcA0, pcA1 != NULL ? pcA1 : "");
    v0 = a0;
    pc0 = ra;
}

/*0x16*/
static void bios_strncat()
{
    char *pcA0 = (char *) Ra0;
    char *pcA1 = (char *) Ra1;

    if (pcA0 == NULL || pcA1 == NULL)
	return;

    strncat(pcA0, pcA1, a2);
    v0 = a0;
    pc0 = ra;
}

static void bios_strcmp()
{				// 0x17
    char *pcA0 = (char *) Ra0;
    char *pcA1 = (char *) Ra1;

    v0 = strcmp(pcA0, pcA1);
    pc0 = ra;
}

static void bios_strncmp()
{				// 0x18
    u32 max = a2;
    u32 string1 = a0;
    u32 string2 = a1;
    s8 tmpv = 0;

    while (max > 0)
    {
	u8 tmp1 = PSXMuR8(string1);
	u8 tmp2 = PSXMuR8(string2);

	if (!tmp1 || !tmp2)
	    break;

	tmpv = tmp1 - tmp2;
	if (tmpv)
	    break;
	if (!tmp1 || !tmp2)
	    break;
	if (!PSXM(string1) || !PSXM(string2))
	    break;
	max--;
	string1++;
	string2++;
    }
    if (tmpv > 0)
	v0 = 1;
    else if (tmpv < 0)
	v0 = -1;
    else
	v0 = 0;
    //printf("%s:%s, %d, %d\n",Ra0,Ra1,a2,v0);
    //v0 = strncmp(Ra0, Ra1, a2);
    pc0 = ra;
}

/*0x19*/
static void bios_strcpy()
{
    u32 src = a1, dest = a0;
    u8 val;

    do
    {
	val = PSXMu8(src);
	PSXMu8(dest) = val;
	src++;
	dest++;
    } while (val);
    //strcpy(Ra0, Ra1); 
    v0 = a0;
    pc0 = ra;
}

/*0x1a*/
static void bios_strncpy()
{
    u32 src = a1, dest = a0, max = a2;
    u8 val;

    do
    {
	val = PSXMu8(src);
	PSXMu8(dest) = val;
	src++;
	dest++;
	max--;
    } while (val && max);

    //strncpy(Ra0, Ra1, a2);  
    v0 = a0;
    pc0 = ra;
}

/*0x1b*/
static void bios_strlen()
{
    u32 src = a0;

    while (PSXMu8(src))
	src++;

    v0 = src - a0;
    pc0 = ra;
}

static void bios_index()
{				// 0x1c
    char *pcA0 = (char *) Ra0;
    char *pcRet = strchr(pcA0, a1);
    if (pcRet)
	v0 = a0 + pcRet - pcA0;
    else
	v0 = 0;
    pc0 = ra;
}

static void bios_rindex()
{				// 0x1d
    char *pcA0 = (char *) Ra0;
    char *pcRet = strrchr(pcA0, a1);
    if (pcRet)
	v0 = a0 + pcRet - pcA0;
    else
	v0 = 0;
    pc0 = ra;
}

static void bios_strchr()
{				// 0x1e
    char *pcA0 = (char *) Ra0;
    char *pcRet = strchr(pcA0, a1);
    if (pcRet)
	v0 = a0 + pcRet - pcA0;
    else
	v0 = 0;
    pc0 = ra;
}

static void bios_strrchr()
{				// 0x1f
    char *pcA0 = (char *) Ra0;
    char *pcRet = strrchr(pcA0, a1);
    if (pcRet)
	v0 = a0 + pcRet - pcA0;
    else
	v0 = 0;
    pc0 = ra;
}

static void bios_strpbrk()
{				// 0x20
    char *pcA0 = (char *) Ra0;
    char *pcA1 = (char *) Ra1;
    char *pcRet = strpbrk(pcA0, pcA1);
    if (pcRet)
	v0 = a0 + pcRet - pcA0;
    else
	v0 = 0;
    pc0 = ra;
}

static void bios_strspn()
{
    char *pcA0 = (char *) Ra0;
    char *pcA1 = (char *) Ra1;
    v0 = strspn(pcA0, pcA1);
    pc0 = ra;
}				/*21 */
static void bios_strcspn()
{
    char *pcA0 = (char *) Ra0;
    char *pcA1 = (char *) Ra1;
    v0 = strcspn(pcA0, pcA1);
    pc0 = ra;
}				/*22 */

static void bios_strtok()
{				// 0x23
    char *pcA0 = (char *) Ra0;
    char *pcA1 = (char *) Ra1;
    char *pcRet = strtok(pcA0, pcA1);
    if (pcRet)
	v0 = a0 + pcRet - pcA0;
    else
	v0 = 0;
    pc0 = ra;
}

static void bios_strstr()
{				// 0x24
    char *pcA0 = (char *) Ra0;
    char *pcA1 = (char *) Ra1;

    char *pcRet = strstr(pcA0, pcA1);

    if (pcRet)
	v0 = a0 + pcRet - pcA0;
    else
	v0 = 0;
    pc0 = ra;
}

/*0x25*/
static void bios_toupper()
{
    v0 = toupper(a0);
    pc0 = ra;
}

/*0x26*/
static void bios_tolower()
{
    v0 = tolower(a0);
    pc0 = ra;
}

/*0x27*/
static void bios_bcopy()
{
    u32 dest = a1, src = a0, len = a2;

    while (len--)
    {
	PSXMu8(dest) = PSXMu8(src);
	dest++;
	src++;
    }
    //memcpy(Ra1,Ra0,a2); 
    pc0 = ra;
}

/*0x28*/
static void bios_bzero()
{
    u32 dest = a0, len = a1;

    while (len--)
    {
	PSXMu8(dest) = 0;
	dest++;
    }

    //memset(Ra0,0,a1); 
    pc0 = ra;
}

/*0x29*/
static void bios_bcmp()
{
    char *pcA0 = (char *) Ra0;
    char *pcA1 = (char *) Ra1;

    v0 = memcmp(pcA0, pcA1, a2);
    pc0 = ra;
}

/*0x2a*/
static void bios_memcpy()
{
    u32 dest = a0, src = a1, len = a2;

    while (len--)
    {
	PSXMu8(dest) = PSXMu8(src);
	dest++;
	src++;
    }
    //memcpy(Ra0, Ra1, a2); 
    v0 = a0;
    pc0 = ra;
}

static void bios_memset()	/*0x2b */
{
    u32 len = a2;
    u32 dest = a0;

    while (len--)
    {
	if (PSXM(dest))
	    PSXMu8(dest) = a1;
	dest++;
    }
    //memset(Ra0, a1, a2);
    v0 = a0;
    pc0 = ra;
}

static void bios_memmove()
{
    char *pcA0 = (char *) Ra0;
    char *pcA1 = (char *) Ra1;

    memmove(pcA0, pcA1, a2);
    v0 = a0;
    pc0 = ra;
}

/*0x2d*/
static void bios_memcmp()
{
    char *pcA0 = (char *) Ra0;
    char *pcA1 = (char *) Ra1;

    v0 = memcmp(pcA0, pcA1, a2);
    pc0 = ra;
}

static void bios_memchr()
{				// 2e
    char *pcA0 = (char *) Ra0;

    void *ret = memchr(pcA0, a1, a2);
    if (ret != NULL)
	v0 = (u32) ((char *) ret - Ra0) + a0;
    else
	v0 = 0;
    pc0 = ra;
}

static void bios_rand()
{				// 2f
    v0 = 1 + (int) (32767.0 * rand() / (RAND_MAX + 1.0));
    pc0 = ra;
}

static void bios_srand()
{				// 30
    srand(a0);
    pc0 = ra;
}

static void bios_malloc()
{				// 33
    u32 chunk;
    u32 fd;

    /* a0:  Number of bytes to allocate. */

    chunk = heap_addr;

    /* Search for first chunk that's large enough and not currently
       being used.
     */
    while ((a0 > BFLIP32(((malloc_chunk *) PSXM(chunk))->size)) || (BFLIP32(((malloc_chunk *) PSXM(chunk))->stat) == INUSE))
	chunk = ((malloc_chunk *) PSXM(chunk))->fd;
    //printf("%08x\n",chunk);

    /* split free chunk */
    fd = chunk + sizeof(malloc_chunk) + a0;
    ((malloc_chunk *) PSXM(fd))->stat = ((malloc_chunk *) PSXM(chunk))->stat;
    ((malloc_chunk *) PSXM(fd))->size = BFLIP32(BFLIP32(((malloc_chunk *) PSXM(chunk))->size) - a0);
    ((malloc_chunk *) PSXM(fd))->fd = ((malloc_chunk *) PSXM(chunk))->fd;
    ((malloc_chunk *) PSXM(fd))->bk = chunk;

    /* set new chunk */
    ((malloc_chunk *) PSXM(chunk))->stat = BFLIP32(INUSE);
    ((malloc_chunk *) PSXM(chunk))->size = BFLIP32(a0);
    ((malloc_chunk *) PSXM(chunk))->fd = fd;

    v0 = chunk + sizeof(malloc_chunk);
    v0 |= 0x80000000;
    //      printf ("malloc %lx,%lx\n", v0, a0);
    pc0 = ra;
}

static void bios_InitHeap()
{				// 39
    malloc_chunk *chunk;

    heap_addr = a0;		// Ra0

    chunk = (malloc_chunk *) PSXM(heap_addr);
    chunk->stat = 0;
    if (((a0 & 0x1fffff) + a1) >= 0x200000)
	chunk->size = BFLIP32(0x1ffffc - (a0 & 0x1fffff));
    else
	chunk->size = BFLIP32(a1);
    chunk->fd = 0;
    chunk->bk = 0;

    pc0 = ra;
}

static void bios_printf()
{
    pc0 = ra;
}

static void bios_puts()
{
    pc0 = ra;

    _DEBUG("%s", Ra0);
}

static void bios_FlushCache()
{				// 44

    pc0 = ra;
}

static void bios__bu_init()
{				// 70

    DeliverEvent(0x11, 0x2);	// 0xf0000011, 0x0004
    DeliverEvent(0x81, 0x2);	// 0xf4000001, 0x0004

    pc0 = ra;
}

static void bios__96_init()
{				// 71

    pc0 = ra;
}

static void bios__96_remove()
{				// 72

    pc0 = ra;
}

/* System calls B0 */

static void bios_SetRCnt()
{				// 02

    a0 &= 0x3;
    if (a0 != 3)
    {
	u32 mode = 0;

	psxRcntWtarget(a0, a1);
	if (a2 & 0x1000)
	    mode |= 0x050;	// Interrupt Mode
	if (a2 & 0x0100)
	    mode |= 0x008;	// Count to 0xffff
	if (a2 & 0x0010)
	    mode |= 0x001;	// Timer stop mode
	if (a0 == 2)
	{
	    if (a2 & 0x0001)
		mode |= 0x200;
	}			// System Clock mode
	else
	{
	    if (a2 & 0x0001)
		mode |= 0x100;
	}			// System Clock mode

	psxRcntWmode(a0, mode);
    }
    pc0 = ra;
}

static void bios_GetRCnt()
{				// 03

    a0 &= 0x3;
    if (a0 != 3)
	v0 = psxRcntRcount(a0);
    else
	v0 = 0;
    pc0 = ra;
}

static void bios_StartRCnt()
{				// 04

    a0 &= 0x3;
    if (a0 != 3)
	psxHu32(0x1074) |= BFLIP32(1 << (a0 + 4));
    else
	psxHu32(0x1074) |= BFLIP32(0x1);
    v0 = 1;
    pc0 = ra;
}

static void bios_StopRCnt()
{				// 05

    a0 &= 0x3;
    if (a0 != 3)
	psxHu32(0x1074) &= BFLIP32(~(1 << (a0 + 4)));
    else
	psxHu32(0x1074) &= BFLIP32(~0x1);
    pc0 = ra;
}

static void bios_ResetRCnt()
{				// 06

    a0 &= 0x3;
    if (a0 != 3)
    {
	psxRcntWmode(a0, 0);
	psxRcntWtarget(a0, 0);
	psxRcntWcount(a0, 0);
    }
    pc0 = ra;
}


/* gets ev for use with Event */
#define GetEv() \
	ev = (a0 >> 24) & 0xf; \
	if (ev == 0xf) ev = 0x5; \
	ev*= 32; \
	ev+= a0&0x1f;

/* gets spec for use with Event */
#define GetSpec() \
	spec = 0; \
	switch (a1) { \
		case 0x0301: spec = 16; break; \
		case 0x0302: spec = 17; break; \
		default: \
			for (i=0; i<16; i++) if (a1 & (1 << i)) { spec = i; break; } \
			break; \
	}

static void bios_DeliverEvent()
{				// 07
    int ev, spec;
    int i;

    GetEv();
    GetSpec();

    DeliverEvent(ev, spec);

    pc0 = ra;
}

static void bios_OpenEvent()
{				// 08
    int ev, spec;
    int i;

    GetEv();
    GetSpec();

    Event[ev][spec].status = BFLIP32S(EvStWAIT);
    Event[ev][spec].mode = BFLIP32(a2);
    Event[ev][spec].fhandler = BFLIP32(a3);

    v0 = ev | (spec << 8);
    pc0 = ra;
}

static void bios_CloseEvent()
{				// 09
    int ev, spec;

    ev = a0 & 0xff;
    spec = (a0 >> 8) & 0xff;

    Event[ev][spec].status = BFLIP32S(EvStUNUSED);

    v0 = 1;
    pc0 = ra;
}

static void bios_WaitEvent()
{				// 0a
    int ev, spec;

    ev = a0 & 0xff;
    spec = (a0 >> 8) & 0xff;

    Event[ev][spec].status = BFLIP32S(EvStACTIVE);

    v0 = 1;
    pc0 = ra;
}

static void bios_TestEvent()
{				// 0b
    int ev, spec;

    ev = a0 & 0xff;
    spec = (a0 >> 8) & 0xff;

    if (Event[ev][spec].status == BFLIP32S(EvStALREADY))
    {
	Event[ev][spec].status = BFLIP32S(EvStACTIVE);
	v0 = 1;
    }
    else
	v0 = 0;

    pc0 = ra;
}

static void bios_EnableEvent()
{				// 0c
    int ev, spec;

    ev = a0 & 0xff;
    spec = (a0 >> 8) & 0xff;

    Event[ev][spec].status = BFLIP32S(EvStACTIVE);

    v0 = 1;
    pc0 = ra;
}

static void bios_DisableEvent()
{				// 0d
    int ev, spec;

    ev = a0 & 0xff;
    spec = (a0 >> 8) & 0xff;

    Event[ev][spec].status = BFLIP32S(EvStWAIT);

    v0 = 1;
    pc0 = ra;
}

/*
 *	long OpenTh(long (*func)(), unsigned long sp, unsigned long gp);
 */

static void bios_OpenTh()
{				// 0e
    int th;

    for (th = 1; th < 8; th++)
	if (Thread[th].status == 0)
	    break;

    Thread[th].status = BFLIP32(1);
    Thread[th].func = BFLIP32(a0);
    Thread[th].reg[29] = BFLIP32(a1);
    Thread[th].reg[28] = BFLIP32(a2);

    v0 = th;
    pc0 = ra;
}

/*
 *	int CloseTh(long thread);
 */

static void bios_CloseTh()
{				// 0f
    int th = a0 & 0xff;

    if (Thread[th].status == 0)
    {
	v0 = 0;
    }
    else
    {
	Thread[th].status = 0;
	v0 = 1;
    }

    pc0 = ra;
}

/*
 *	int ChangeTh(long thread);
 */

static void bios_ChangeTh()
{				// 10
    int th = a0 & 0xff;

    if (Thread[th].status == 0 || CurThread == th)
    {
	v0 = 0;

	pc0 = ra;
    }
    else
    {
	v0 = 1;

	if (Thread[CurThread].status == BFLIP32S(2))
	{
	    Thread[CurThread].status = BFLIP32S(1);
	    Thread[CurThread].func = BFLIP32(ra);
	    memcpy(Thread[CurThread].reg, upse_r3000_cpu_regs.GPR.r, 32 * 4);
	}

	memcpy(upse_r3000_cpu_regs.GPR.r, Thread[th].reg, 32 * 4);
	pc0 = BFLIP32(Thread[th].func);
	Thread[th].status = BFLIP32(2);
	CurThread = th;
    }
}

static void bios_ReturnFromException()
{				// 17
    memcpy(upse_r3000_cpu_regs.GPR.r, regs, 32 * 4);
    upse_r3000_cpu_regs.GPR.n.lo = regs[32];
    upse_r3000_cpu_regs.GPR.n.hi = regs[33];

    pc0 = upse_r3000_cpu_regs.CP0.n.EPC;
    if (upse_r3000_cpu_regs.CP0.n.Cause & 0x80000000)
	pc0 += 4;

    upse_r3000_cpu_regs.CP0.n.Status = (upse_r3000_cpu_regs.CP0.n.Status & 0xfffffff0) | ((upse_r3000_cpu_regs.CP0.n.Status & 0x3c) >> 2);
}

static void bios_ResetEntryInt()
{				// 18

    jmp_int = NULL;
    pc0 = ra;
}

static void bios_HookEntryInt()
{				// 19

    jmp_int = (u32 *) Ra0;
    pc0 = ra;
}

static void bios_UnDeliverEvent()
{				// 0x20
    int ev, spec;
    int i;

    GetEv();
    GetSpec();

    if (Event[ev][spec].status == BFLIP32S(EvStALREADY) && Event[ev][spec].mode == BFLIP32S(EvMdNOINTR))
	Event[ev][spec].status = BFLIP32S(EvStACTIVE);

    pc0 = ra;
}

static void bios_GetC0Table()
{				// 56

    v0 = 0x674;
    pc0 = ra;
}

static void bios_GetB0Table()
{				// 57

    v0 = 0x874;
    pc0 = ra;
}

/* System calls C0 */

/*
 * int SysEnqIntRP(int index , long *queue);
 */

static void bios_SysEnqIntRP()
{				// 02

    SysIntRP[a0] = a1;

    v0 = 0;
    pc0 = ra;
}

/*
 * int SysDeqIntRP(int index , long *queue);
 */

static void bios_SysDeqIntRP()
{				// 03

    SysIntRP[a0] = 0;

    v0 = 0;
    pc0 = ra;
}

static void bios_ChangeClearRCnt()
{				// 0a
    u32 *ptr;

    ptr = (u32 *) PSXM((a0 << 2) + 0x8600);
    v0 = BFLIP32(*ptr);
    *ptr = BFLIP32(a1);

    upse_r3000_cpu_regs.CP0.n.Status|= 0x404;
    pc0 = ra;
}

static void bios_dummy()
{
    u32 call = t1 & 0xff;

    pc0 = ra;
    v0 = a1;

    _DEBUG("WTF: unknown bios call: %x", call);
}

void (*biosA0[256]) ();
void (*biosB0[256]) ();
void (*biosC0[256]) ();

void upse_ps1_bios_init()
{
    u32 base, size;
    u32 *ptr;
    int i;

    if (upse_has_custom_bios())
        return;

    heap_addr = 0;
    CurThread = 0;
    jmp_int = NULL;

    for (i = 0; i < 256; i++)
    {
	biosA0[i] = NULL;
	biosB0[i] = NULL;
	biosC0[i] = NULL;
    }

    for (i = 0; i < 256; i++)
    {
	if (biosA0[i] == NULL)
	    biosA0[i] = bios_dummy;
	if (biosB0[i] == NULL)
	    biosB0[i] = bios_dummy;
	if (biosC0[i] == NULL)
	    biosC0[i] = bios_dummy;
    }

    biosA0[0x0e] = bios_abs;
    biosA0[0x0f] = bios_labs;
    biosA0[0x10] = bios_atoi;
    biosA0[0x11] = bios_atol;
    //biosA0[0x12] = bios_atob;
    biosA0[0x13] = bios_setjmp;
    biosA0[0x14] = bios_longjmp;

    biosA0[0x15] = bios_strcat;
    biosA0[0x16] = bios_strncat;
    biosA0[0x17] = bios_strcmp;
    biosA0[0x18] = bios_strncmp;
    biosA0[0x19] = bios_strcpy;
    biosA0[0x1a] = bios_strncpy;
    biosA0[0x1b] = bios_strlen;
    biosA0[0x1c] = bios_index;
    biosA0[0x1d] = bios_rindex;
    biosA0[0x1e] = bios_strchr;
    biosA0[0x1f] = bios_strrchr;
    biosA0[0x20] = bios_strpbrk;
    biosA0[0x21] = bios_strspn;
    biosA0[0x22] = bios_strcspn;
    biosA0[0x23] = bios_strtok;
    biosA0[0x24] = bios_strstr;
    biosA0[0x25] = bios_toupper;
    biosA0[0x26] = bios_tolower;
    biosA0[0x27] = bios_bcopy;
    biosA0[0x28] = bios_bzero;
    biosA0[0x29] = bios_bcmp;
    biosA0[0x2a] = bios_memcpy;
    biosA0[0x2b] = bios_memset;
    biosA0[0x2c] = bios_memmove;
    biosA0[0x2c] = bios_memcpy;	/* Our code should be compatible
				   with both memcpy and memmove 
				   semantics. */
    biosA0[0x2d] = bios_memcmp;
    biosA0[0x2e] = bios_memchr;

    biosA0[0x2f] = bios_rand;
    biosA0[0x30] = bios_srand;

    //biosA0[0x31] = bios_qsort;
    //biosA0[0x32] = bios_strtod;
    biosA0[0x33] = bios_malloc;
    //biosA0[0x34] = bios_free;
    //biosA0[0x35] = bios_lsearch;
    //biosA0[0x36] = bios_bsearch;
    //biosA0[0x37] = bios_calloc;
    //biosA0[0x38] = bios_realloc;
    biosA0[0x39] = bios_InitHeap;
    //biosA0[0x3a] = bios__exit;
    biosA0[0x3e] = bios_puts;
    biosA0[0x3f] = bios_printf;
    biosA0[0x44] = bios_FlushCache;
    //biosA0[0x45] = bios_InstallInterruptHandler;
    //biosA0[0x4f] = bios_sys_a0_4f;
    //biosA0[0x50] = bios_sys_a0_50;                
    biosA0[0x70] = bios__bu_init;
    biosA0[0x71] = bios__96_init;
    biosA0[0x72] = bios__96_remove;
    //biosA0[0x73] = bios_sys_a0_73;
    //biosA0[0x74] = bios_sys_a0_74;
    //biosA0[0x75] = bios_sys_a0_75;
    //biosA0[0x76] = bios_sys_a0_76;
    //biosA0[0x77] = bios_sys_a0_77;
    //biosA0[0x78] = bios__96_CdSeekL;
    //biosA0[0x79] = bios_sys_a0_79;
    //biosA0[0x7a] = bios_sys_a0_7a;
    //biosA0[0x7b] = bios_sys_a0_7b;
    //biosA0[0x7c] = bios__96_CdGetStatus;
    //biosA0[0x7d] = bios_sys_a0_7d;
    //biosA0[0x7e] = bios__96_CdRead;
    //biosA0[0x7f] = bios_sys_a0_7f;
    //biosA0[0x80] = bios_sys_a0_80;
    //biosA0[0x81] = bios_sys_a0_81;
    //biosA0[0x82] = bios_sys_a0_82;                
    //biosA0[0x83] = bios_sys_a0_83;
    //biosA0[0x84] = bios_sys_a0_84;
    //biosA0[0x85] = bios__96_CdStop;       
    //biosA0[0x86] = bios_sys_a0_86;
    //biosA0[0x87] = bios_sys_a0_87;
    //biosA0[0x88] = bios_sys_a0_88;
    //biosA0[0x89] = bios_sys_a0_89;
    //biosA0[0x8a] = bios_sys_a0_8a;
    //biosA0[0x8b] = bios_sys_a0_8b;
    //biosA0[0x8c] = bios_sys_a0_8c;
    //biosA0[0x8d] = bios_sys_a0_8d;
    //biosA0[0x8e] = bios_sys_a0_8e;                
    //biosA0[0x8f] = bios_sys_a0_8f;
    //biosA0[0x90] = bios_sys_a0_90;
    //biosA0[0x91] = bios_sys_a0_91;
    //biosA0[0x92] = bios_sys_a0_92;
    //biosA0[0x93] = bios_sys_a0_93;
    //biosA0[0x94] = bios_sys_a0_94;
    //biosA0[0x95] = bios_sys_a0_95;
    //biosA0[0x96] = bios_AddCDROMDevice;
    //biosA0[0x97] = bios_AddMemCardDevide;
    //biosA0[0x98] = bios_DisableKernelIORedirection;
    //biosA0[0x99] = bios_EnableKernelIORedirection;
    //biosA0[0x9a] = bios_sys_a0_9a;
    //biosA0[0x9b] = bios_sys_a0_9b;
    //biosA0[0x9c] = bios_SetConf;
    //biosA0[0x9d] = bios_GetConf;
    //biosA0[0x9e] = bios_sys_a0_9e;
    //biosA0[0x9f] = bios_SetMem;
    //biosA0[0xa0] = bios__boot;
    //biosA0[0xa1] = bios_SystemError;
    //biosA0[0xa2] = bios_EnqueueCdIntr;
    //biosA0[0xa3] = bios_DequeueCdIntr;
    //biosA0[0xa4] = bios_sys_a0_a4;
    //biosA0[0xa5] = bios_ReadSector;
    //biosA0[0xa6] = bios_get_cd_status;
    //biosA0[0xa7] = bios_bufs_cb_0;
    //biosA0[0xa8] = bios_bufs_cb_1;
    //biosA0[0xa9] = bios_bufs_cb_2;
    //biosA0[0xaa] = bios_bufs_cb_3;
    //biosA0[0xab] = bios__card_info;
    //biosA0[0xac] = bios__card_load;
    //biosA0[0axd] = bios__card_auto;
    //biosA0[0xae] = bios_bufs_cd_4;
    //biosA0[0xaf] = bios_sys_a0_af;
    //biosA0[0xb0] = bios_sys_a0_b0;
    //biosA0[0xb1] = bios_sys_a0_b1;
    //biosA0[0xb2] = bios_do_a_long_jmp
    //biosA0[0xb3] = bios_sys_a0_b3;
    //biosA0[0xb4] = bios_sub_function;
//*******************B0 CALLS****************************
    //biosB0[0x00] = bios_SysMalloc;
    //biosB0[0x01] = bios_sys_b0_01;
    biosB0[0x02] = bios_SetRCnt;
    biosB0[0x03] = bios_GetRCnt;
    biosB0[0x04] = bios_StartRCnt;
    biosB0[0x05] = bios_StopRCnt;
    biosB0[0x06] = bios_ResetRCnt;
    biosB0[0x07] = bios_DeliverEvent;
    biosB0[0x08] = bios_OpenEvent;
    biosB0[0x09] = bios_CloseEvent;
    biosB0[0x0a] = bios_WaitEvent;
    biosB0[0x0b] = bios_TestEvent;
    biosB0[0x0c] = bios_EnableEvent;
    biosB0[0x0d] = bios_DisableEvent;
    biosB0[0x0e] = bios_OpenTh;
    biosB0[0x0f] = bios_CloseTh;
    biosB0[0x10] = bios_ChangeTh;
    //biosB0[0x11] = bios_bios_b0_11;
    biosB0[0x17] = bios_ReturnFromException;
    biosB0[0x18] = bios_ResetEntryInt;
    biosB0[0x19] = bios_HookEntryInt;
    //biosB0[0x1a] = bios_sys_b0_1a;
    //biosB0[0x1b] = bios_sys_b0_1b;
    //biosB0[0x1c] = bios_sys_b0_1c;
    //biosB0[0x1d] = bios_sys_b0_1d;
    //biosB0[0x1e] = bios_sys_b0_1e;
    //biosB0[0x1f] = bios_sys_b0_1f;
    biosB0[0x20] = bios_UnDeliverEvent;
    //biosB0[0x21] = bios_sys_b0_21;
    //biosB0[0x22] = bios_sys_b0_22;
    //biosB0[0x23] = bios_sys_b0_23;
    //biosB0[0x24] = bios_sys_b0_24;
    //biosB0[0x25] = bios_sys_b0_25;
    //biosB0[0x26] = bios_sys_b0_26;
    //biosB0[0x27] = bios_sys_b0_27;
    //biosB0[0x28] = bios_sys_b0_28;
    //biosB0[0x29] = bios_sys_b0_29;
    //biosB0[0x2a] = bios_sys_b0_2a;
    //biosB0[0x2b] = bios_sys_b0_2b;
    //biosB0[0x2c] = bios_sys_b0_2c;
    //biosB0[0x2d] = bios_sys_b0_2d;
    //biosB0[0x2e] = bios_sys_b0_2e;
    //biosB0[0x2f] = bios_sys_b0_2f;
    //biosB0[0x30] = bios_sys_b0_30;
    //biosB0[0x31] = bios_sys_b0_31;
    biosB0[0x3f] = bios_puts;
    biosB0[0x56] = bios_GetC0Table;
    biosB0[0x57] = bios_GetB0Table;
    //biosB0[0x58] = bios__card_chan;
    //biosB0[0x59] = bios_sys_b0_59;
    //biosB0[0x5a] = bios_sys_b0_5a;
    //biosB0[0x5c] = bios__card_status;
    //biosB0[0x5d] = bios__card_wait;
//*******************C0 CALLS****************************
    //biosC0[0x00] = bios_InitRCnt;
    //biosC0[0x01] = bios_InitException;
    biosC0[0x02] = bios_SysEnqIntRP;
    biosC0[0x03] = bios_SysDeqIntRP;
    //biosC0[0x04] = bios_get_free_EvCB_slot;
    //biosC0[0x05] = bios_get_free_TCB_slot;
    //biosC0[0x06] = bios_ExceptionHandler;
    //biosC0[0x07] = bios_InstallExeptionHandler;
    //biosC0[0x08] = bios_SysInitMemory;
    //biosC0[0x09] = bios_SysInitKMem;
    biosC0[0x0a] = bios_ChangeClearRCnt;
    //biosC0[0x0b] = bios_SystemError;
    //biosC0[0x0c] = bios_InitDefInt;
    //biosC0[0x0d] = bios_sys_c0_0d;
    //biosC0[0x0e] = bios_sys_c0_0e;
    //biosC0[0x0f] = bios_sys_c0_0f;
    //biosC0[0x10] = bios_sys_c0_10;
    //biosC0[0x11] = bios_sys_c0_11;
    //biosC0[0x12] = bios_InstallDevices;
    //biosC0[0x13] = bios_FlushStfInOutPut;
    //biosC0[0x14] = bios_sys_c0_14;
    //biosC0[0x15] = bios__cdevinput;
    //biosC0[0x16] = bios__cdevscan;
    //biosC0[0x17] = bios__circgetc;
    //biosC0[0x18] = bios__circputc;                  
    //biosC0[0x19] = bios_ioabort;
    //biosC0[0x1a] = bios_sys_c0_1a
    //biosC0[0x1b] = bios_KernelRedirect;
    //biosC0[0x1c] = bios_PatchAOTable;
    biosC0[0x3f] = bios_printf;
//************** THE END ***************************************

    base = 0x1000;
    size = sizeof(EvCB) * 32;
    Event = (void *) &psxR[base];
    base += size * 6;
    memset(Event, 0, size * 6);
    //HwEV = Event;
    //EvEV = Event + 32;
    RcEV = Event + 32 * 2;
    //UeEV = Event + 32*3;
    //SwEV = Event + 32*4;
    //ThEV = Event + 32*5;

    ptr = (u32 *) & psxM[0x0874];	// b0 table
    ptr[0] = BFLIP32(0x4c54 - 0x884);

    ptr = (u32 *) & psxM[0x0674];	// c0 table
    ptr[6] = BFLIP32(0xc80);

    memset(SysIntRP, 0, sizeof(SysIntRP));
    memset(Thread, 0, sizeof(Thread));
    Thread[0].status = BFLIP32(2);	// main thread

    psxMu32(0x0150) = BFLIP32(0x160);
    psxMu32(0x0154) = BFLIP32(0x320);
    psxMu32(0x0160) = BFLIP32(0x248);
    strcpy(&psxM[0x248], "bu");

	psxMu32(0x0ca8) = BFLIP32(0x1f410004);
	psxMu32(0x0cf0) = BFLIP32(0x3c020000);
	psxMu32(0x0cf4) = BFLIP32(0x2442641c);
	psxMu32(0x09e0) = BFLIP32(0x43d0);
	psxMu32(0x4d98) = BFLIP32(0x946f000a);

    // opcode HLE
    psxRu32(0x0000) = BFLIP32((0x3b << 26) | 4);
    psxMu32(0x0000) = BFLIP32((0x3b << 26) | 0);
    psxMu32(0x00a0) = BFLIP32((0x3b << 26) | 1);
    psxMu32(0x00b0) = BFLIP32((0x3b << 26) | 2);
    psxMu32(0x00c0) = BFLIP32((0x3b << 26) | 3);
    psxMu32(0x4c54) = BFLIP32((0x3b << 26) | 0);
    psxMu32(0x8000) = BFLIP32((0x3b << 26) | 5);
    psxMu32(0x07a0) = BFLIP32((0x3b << 26) | 0);
    psxMu32(0x0884) = BFLIP32((0x3b << 26) | 0);
    psxMu32(0x0894) = BFLIP32((0x3b << 26) | 0);
}

void upse_ps1_bios_shutdown()
{
}

void biosInterrupt()
{
    if (BFLIP32(psxHu32(0x1070)) & 0x1)
    {				// Vsync
	if (RcEV[3][1].status == BFLIP32S(EvStACTIVE))
	{
	    softCall(BFLIP32(RcEV[3][1].fhandler));
//                      hwwrite_32(0x1f801070, ~(1));
	}
    }

    if (BFLIP32(psxHu32(0x1070)) & 0x70)
    {				// Rcnt 0,1,2
	int i;

	for (i = 0; i < 3; i++)
	{
	    if (BFLIP32(psxHu32(0x1070)) & (1 << (i + 4)))
	    {
		if (RcEV[i][1].status == BFLIP32S(EvStACTIVE))
		{
		    softCall(BFLIP32(RcEV[i][1].fhandler));
		    upse_ps1_hal_write_32(0x1f801070, ~(1 << (i + 4)));
		}
	    }
	}
    }
}

static INLINE void SaveRegs()
{
    memcpy(regs, upse_r3000_cpu_regs.GPR.r, 32 * 4);
    regs[32] = upse_r3000_cpu_regs.GPR.n.lo;
    regs[33] = upse_r3000_cpu_regs.GPR.n.hi;
    regs[34] = upse_r3000_cpu_regs.pc;
}

void upse_ps1_bios_exception()
{
    int i;

    switch (upse_r3000_cpu_regs.CP0.n.Cause & 0x3c)
    {
      case 0x00:		// Interrupt
#ifdef PSXCPU_LOG
//                      PSXCPU_LOG("interrupt\n");
#endif
	  SaveRegs();

	  biosInterrupt();

	  for (i = 0; i < 8; i++)
	  {
	      if (SysIntRP[i])
	      {
		  u32 *queue = (u32 *) PSXM(SysIntRP[i]);

		  s0 = BFLIP32(queue[2]);
		  softCall(BFLIP32(queue[1]));
	      }
	  }

	  if (jmp_int != NULL)
	  {
	      int i;

	      upse_ps1_hal_write_32(0x1f801070, 0xffffffff);

	      ra = BFLIP32(jmp_int[0]);
	      sp = BFLIP32(jmp_int[1]);
	      fp = BFLIP32(jmp_int[2]);
	      for (i = 0; i < 8; i++)	// s0-s7
		  upse_r3000_cpu_regs.GPR.r[16 + i] = BFLIP32(jmp_int[3 + i]);
	      gp = BFLIP32(jmp_int[11]);

	      v0 = 1;
	      pc0 = ra;
	      return;
	  }
	  upse_ps1_hal_write_16(0x1f801070, 0);
	  break;
      case 0x20:		// Syscall
#ifdef PSXCPU_LOG
//                      PSXCPU_LOG("syscall exp %x\n", a0);
#endif
	  switch (a0)
	  {
	    case 1:		// EnterCritical - disable irq's
		upse_r3000_cpu_regs.CP0.n.Status &= ~0x404;
		break;
	    case 2:		// ExitCritical - enable irq's
		upse_r3000_cpu_regs.CP0.n.Status |= 0x404;
		break;
	  }
	  pc0 = upse_r3000_cpu_regs.CP0.n.EPC + 4;

	  upse_r3000_cpu_regs.CP0.n.Status = (upse_r3000_cpu_regs.CP0.n.Status & 0xfffffff0) | ((upse_r3000_cpu_regs.CP0.n.Status & 0x3c) >> 2);
	  return;
      default:
          _DEBUG("WTF: unknown exception <%x>", upse_r3000_cpu_regs.CP0.n.Cause & 0x3c);
	  break;
    }

    pc0 = upse_r3000_cpu_regs.CP0.n.EPC;
    if (upse_r3000_cpu_regs.CP0.n.Cause & 0x80000000)
	pc0 += 4;

    upse_r3000_cpu_regs.CP0.n.Status = (upse_r3000_cpu_regs.CP0.n.Status & 0xfffffff0) | ((upse_r3000_cpu_regs.CP0.n.Status & 0x3c) >> 2);
}

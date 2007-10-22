/*
 * UPSE: the unix playstation sound emulator.
 *
 * Filename: upse_r3000_abstract.c
 * Purpose: libupse: r3K abstract implementation factory
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "upse-internal.h"

psxRegisters psxRegs;
R3000Acpu *psxCpu;

int psxInit()
{

    psxCpu = &psxInt;

    if (psxMemInit() == -1)
	return -1;

    return psxCpu->Init();
}

void psxReset()
{
    psxCpu->Reset();
    psxMemReset();

    memset(&psxRegs, 0, sizeof(psxRegs));

    psxRegs.pc = 0xbfc00000;	// Start in bootstrap
    psxRegs.CP0.r[12] = 0x10900000;	// COP0 enabled | BEV = 1 | TS = 1
    psxRegs.CP0.r[15] = 0x00000002;	// PRevID = Revision ID, same as R3000A

    psxHwReset();
    psxBiosInit();
}

void psxShutdown()
{
    psxMemShutdown();
    psxBiosShutdown();

    psxCpu->Shutdown();
    SPUclose();
}

void psxException(u32 code, u32 bd)
{
    // Set the Cause
    psxRegs.CP0.n.Cause = code;

#ifdef PSXCPU_LOG
    if (bd)
	PSXCPU_LOG("bd set\n");
#endif
    // Set the EPC & PC
    if (bd)
    {
	psxRegs.CP0.n.Cause |= 0x80000000;
	psxRegs.CP0.n.EPC = (psxRegs.pc - 4);
    }
    else
	psxRegs.CP0.n.EPC = (psxRegs.pc);

    if (psxRegs.CP0.n.Status & 0x400000)
	psxRegs.pc = 0xbfc00180;
    else
	psxRegs.pc = 0x80000080;

    // Set the Status
    psxRegs.CP0.n.Status = (psxRegs.CP0.n.Status & ~0x3f) | ((psxRegs.CP0.n.Status & 0xf) << 2);

    psxBiosException();
}

void psxBranchTest()
{
    if ((psxRegs.cycle - psxNextsCounter) >= psxNextCounter)
	psxRcntUpdate();

    if (psxHu32(0x1070) & psxHu32(0x1074))
    {
	if ((psxRegs.CP0.n.Status & 0x401) == 0x401)
	{
	    psxException(0x400, 0);
	}
    }

}
void psxExecuteBios()
{
    while (psxRegs.pc != 0x80030000)
	psxCpu->ExecuteBlock();
}

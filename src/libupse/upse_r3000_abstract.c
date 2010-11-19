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

upse_r3000_cpu_registers_t upse_r3000_cpu_regs;
upse_spu_state_t *spu = NULL;

int psxInit(upse_module_instance_t *ins)
{
    int ret;

    if (upse_ps1_memory_init(ins) == -1)
	return -1;

    ret = upse_r3000_cpu_init();

    return ret;
}

void psxReset(upse_module_instance_t *ins, upse_psx_revision_t rev)
{
    upse_r3000_cpu_reset();
    upse_ps1_memory_reset(ins);

    ins->spu = upse_ps1_spu_open(ins);

    memset(&upse_r3000_cpu_regs, 0, sizeof(upse_r3000_cpu_regs));

    upse_r3000_cpu_regs.pc = 0xbfc00000;	// Start in bootstrap
    upse_r3000_cpu_regs.CP0.r[12] = 0x10900000;	// COP0 enabled | BEV = 1 | TS = 1

    switch (rev)
    {
    case UPSE_PSX_REV_PS2_IOP:
        upse_r3000_cpu_regs.CP0.r[15] = 0x00000010;	// PRevID = Revision ID, same as IOP
        break;
    case UPSE_PSX_REV_PS1:
        upse_r3000_cpu_regs.CP0.r[15] = 0x00000002;	// PRevID = Revision ID, same as R3000A
        break;
    }

    upse_ps1_hal_reset(ins);
    upse_ps1_bios_init(ins);

    /* start up the bios */
    if (upse_has_custom_bios())
        psxExecuteBios(ins);
}

void psxShutdown(upse_module_instance_t *ins)
{
    upse_ps1_memory_shutdown(ins);
    upse_ps1_bios_shutdown(ins);

    upse_r3000_cpu_shutdown();

    upse_ps1_spu_close(ins->spu);
    ins->spu = NULL;
}

void psxException(upse_module_instance_t *ins, u32 code, u32 bd)
{
    _DEBUG("exception, code %d bd %d.", code, bd);

    // Set the Cause
    upse_r3000_cpu_regs.CP0.n.Cause = code;

#ifdef PSXCPU_LOG
    if (bd)
	PSXCPU_LOG("bd set\n");
#endif
    // Set the EPC & PC
    if (bd)
    {
	upse_r3000_cpu_regs.CP0.n.Cause |= 0x80000000;
	upse_r3000_cpu_regs.CP0.n.EPC = (upse_r3000_cpu_regs.pc - 4);
    }
    else
	upse_r3000_cpu_regs.CP0.n.EPC = (upse_r3000_cpu_regs.pc);

    if (upse_r3000_cpu_regs.CP0.n.Status & 0x400000)
	upse_r3000_cpu_regs.pc = 0xbfc00180;
    else
	upse_r3000_cpu_regs.pc = 0x80000080;

    // Set the Status
    upse_r3000_cpu_regs.CP0.n.Status = (upse_r3000_cpu_regs.CP0.n.Status & ~0x3f) | ((upse_r3000_cpu_regs.CP0.n.Status & 0xf) << 2);

    if (!upse_has_custom_bios())
        upse_ps1_bios_exception(ins);
}

void psxBranchTest(upse_module_instance_t *ins)
{
    upse_psx_counter_state_t *ctrstate = ins->ctrstate;

    _ENTER;

    if ((upse_r3000_cpu_regs.cycle - ctrstate->psxNextsCounter) >= ctrstate->psxNextCounter)
	psxRcntUpdate(ins);

    if (psxHu32(ins, 0x1070) & psxHu32(ins, 0x1074))
    {
	if ((upse_r3000_cpu_regs.CP0.n.Status & 0x401) == 0x401)
	{
	    psxException(ins, 0x400, 0);
	}
    }

    _LEAVE;
}

void psxExecuteBios(upse_module_instance_t *ins)
{
    while (upse_r3000_cpu_regs.pc != 0x80030000)
        upse_r3000_cpu_execute_block(ins);
}

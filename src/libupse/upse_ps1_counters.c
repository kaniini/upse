/*
 * UPSE: the unix playstation sound emulator.
 *
 * Filename: upse_ps1_counters.c
 * Purpose: libupse: PS1 timekeeping implementation
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

#include <string.h>

#include "upse-internal.h"

static int cnts = 4;
static u32 last = 0;

static void psxRcntUpd(upse_module_instance_t *ins, u32 index)
{
    upse_psx_counter_state_t *ctrstate = ins->ctrstate;

    ctrstate->psxCounters[index].sCycle = upse_r3000_cpu_regs.cycle;
    if (((!(ctrstate->psxCounters[index].mode & 1)) || (index != 2)) && ctrstate->psxCounters[index].mode & 0x30)
    {
	if (ctrstate->psxCounters[index].mode & 0x10)
	{			// Interrupt on target
	    ctrstate->psxCounters[index].Cycle = ((ctrstate->psxCounters[index].target - ctrstate->psxCounters[index].count) * ctrstate->psxCounters[index].rate) / BIAS;
	}
	else
	{			// Interrupt on 0xffff
	    ctrstate->psxCounters[index].Cycle = ((0xffff - ctrstate->psxCounters[index].count) * ctrstate->psxCounters[index].rate) / BIAS;
	}
    }
    else
	ctrstate->psxCounters[index].Cycle = 0xffffffff;
}

static void psxRcntReset(upse_module_instance_t *ins, u32 index)
{
    upse_psx_counter_state_t *ctrstate = ins->ctrstate;

    ctrstate->psxCounters[index].count = 0;
    psxRcntUpd(ins, index);

    psxHu32(0x1070) |= BFLIP32(ctrstate->psxCounters[index].interrupt);
    if (!(ctrstate->psxCounters[index].mode & 0x40))
    {				// Only 1 interrupt
	ctrstate->psxCounters[index].Cycle = 0xffffffff;
    }
}

static void psxRcntSet(upse_module_instance_t *ins)
{
    int i;
    upse_psx_counter_state_t *ctrstate = ins->ctrstate;

    ctrstate->psxNextCounter = 0x7fffffff;
    ctrstate->psxNextsCounter = upse_r3000_cpu_regs.cycle;

    for (i = 0; i < cnts; i++)
    {
	s32 count;

	if (ctrstate->psxCounters[i].Cycle == 0xffffffff)
	    continue;

	count = ctrstate->psxCounters[i].Cycle - (upse_r3000_cpu_regs.cycle - ctrstate->psxCounters[i].sCycle);

	if (count < 0)
	{
	    ctrstate->psxNextCounter = 0;
	    break;
	}

	if (count < (s32) ctrstate->psxNextCounter)
	{
	    ctrstate->psxNextCounter = count;
	}
    }
}

void psxRcntInit(upse_module_instance_t *ins)
{
    upse_psx_counter_state_t *ctrstate;

    ctrstate = calloc(sizeof(upse_psx_counter_state_t), 1);

    ctrstate->psxCounters[0].rate = 1;
    ctrstate->psxCounters[0].interrupt = 0x10;
    ctrstate->psxCounters[1].rate = 1;
    ctrstate->psxCounters[1].interrupt = 0x20;
    ctrstate->psxCounters[2].rate = 1;
    ctrstate->psxCounters[2].interrupt = 64;

    ctrstate->psxCounters[3].interrupt = 1;
    ctrstate->psxCounters[3].mode = 0x58;	// The VSync counter mode
    ctrstate->psxCounters[3].target = 1;

    ins->ctrstate = ctrstate;

    psxUpdateVSyncRate(ins);

    cnts = 4;

    psxRcntUpd(ins, 0);
    psxRcntUpd(ins, 1);
    psxRcntUpd(ins, 2);
    psxRcntUpd(ins, 3);
    psxRcntSet(ins);
    last = 0;
}

void CounterDeadLoopSkip(upse_module_instance_t *ins)
{
    s32 min, x, lmin;
    upse_psx_counter_state_t *ctrstate = ins->ctrstate;

    lmin = 0x7FFFFFFF;

    for (x = 0; x < 4; x++)
    {
	if (ctrstate->psxCounters[x].Cycle != 0xffffffff)
        {
            min = ctrstate->psxCounters[x].Cycle;
            min -= (upse_r3000_cpu_regs.cycle - ctrstate->psxCounters[x].sCycle);
            if (min < lmin)
                lmin = min;
        }
    }

    if (lmin > 0)
        upse_r3000_cpu_regs.cycle += lmin;
}

int CounterSPURun(upse_module_instance_t *ins)
{
    u32 cycles;

    if (upse_r3000_cpu_regs.cycle < last)
    {
	cycles = 0xFFFFFFFF - last;
	cycles += upse_r3000_cpu_regs.cycle;
    }
    else
	cycles = upse_r3000_cpu_regs.cycle - last;

    if (cycles >= 16)
    {
	if (!upse_ps1_spu_render(ins->spu, cycles))
	    return (0);
	last = upse_r3000_cpu_regs.cycle;
    }
    return (1);
}

void psxUpdateVSyncRate(upse_module_instance_t *ins)
{
    upse_psx_counter_state_t *ctrstate = ins->ctrstate;

    ctrstate->psxCounters[3].rate = (PSXCLK / 60);
}

void psxRcntUpdate(upse_module_instance_t *ins)
{
    upse_psx_counter_state_t *ctrstate = ins->ctrstate;

    if ((upse_r3000_cpu_regs.cycle - ctrstate->psxCounters[3].sCycle) >= ctrstate->psxCounters[3].Cycle)
    {
	psxRcntUpd(ins, 3);
	psxHu32(0x1070) |= BFLIP32(1);
    }
    if ((upse_r3000_cpu_regs.cycle - ctrstate->psxCounters[0].sCycle) >= ctrstate->psxCounters[0].Cycle)
    {
	psxRcntReset(ins, 0);
    }

    if ((upse_r3000_cpu_regs.cycle - ctrstate->psxCounters[1].sCycle) >= ctrstate->psxCounters[1].Cycle)
    {
	psxRcntReset(ins, 1);
    }

    if ((upse_r3000_cpu_regs.cycle - ctrstate->psxCounters[2].sCycle) >= ctrstate->psxCounters[2].Cycle)
    {
	psxRcntReset(ins, 2);
    }

    psxRcntSet(ins);
}

void psxRcntWcount(upse_module_instance_t *ins, u32 index, u32 value)
{
    upse_psx_counter_state_t *ctrstate = ins->ctrstate;

    ctrstate->psxCounters[index].count = value;
    psxRcntUpd(ins, index);
    psxRcntSet(ins);
}

void psxRcntWmode(upse_module_instance_t *ins, u32 index, u32 value)
{
    upse_psx_counter_state_t *ctrstate = ins->ctrstate;

    ctrstate->psxCounters[index].mode = value;
    ctrstate->psxCounters[index].count = 0;

    if (index == 0)
    {
	switch (value & 0x300)
	{
	  case 0x100:
	      ctrstate->psxCounters[index].rate = ((ctrstate->psxCounters[3].rate /** BIAS*/ ) / 386) / 262;	// seems ok
	      break;
	  default:
	      ctrstate->psxCounters[index].rate = 1;
	}
    }
    else if (index == 1)
    {
	switch (value & 0x300)
	{
	  case 0x100:
	      ctrstate->psxCounters[index].rate = (ctrstate->psxCounters[3].rate /** BIAS*/ ) / 262;	// seems ok
	      //ctrstate->psxCounters[index].rate = (PSXCLK / 60)/262; //(ctrstate->psxCounters[3].rate*16/262);
	      //printf("%d\n",ctrstate->psxCounters[index].rate);
	      break;
	  default:
	      ctrstate->psxCounters[index].rate = 1;
	}
    }
    else if (index == 2)
    {
	switch (value & 0x300)
	{
	  case 0x200:
	      ctrstate->psxCounters[index].rate = 8;	// 1/8 speed
	      break;
	  default:
	      ctrstate->psxCounters[index].rate = 1;	// normal speed
	}
    }

    // Need to set a rate and target
    psxRcntUpd(ins, index);
    psxRcntSet(ins);
}

void psxRcntWtarget(upse_module_instance_t *ins, u32 index, u32 value)
{
    upse_psx_counter_state_t *ctrstate = ins->ctrstate;

    ctrstate->psxCounters[index].target = value;
    psxRcntUpd(ins, index);
    psxRcntSet(ins);
}

u32 psxRcntRcount(upse_module_instance_t *ins, u32 index)
{
    u32 ret;
    upse_psx_counter_state_t *ctrstate = ins->ctrstate;

    if (ctrstate->psxCounters[index].mode & 0x08)
    {				// Wrap at target
	ret = (ctrstate->psxCounters[index].count + BIAS * ((upse_r3000_cpu_regs.cycle - ctrstate->psxCounters[index].sCycle) / ctrstate->psxCounters[index].rate)) & 0xffff;
    }
    else
    {				// Wrap at 0xffff
	ret = (ctrstate->psxCounters[index].count + BIAS * (upse_r3000_cpu_regs.cycle / ctrstate->psxCounters[index].rate)) & 0xffff;
    }

    return ret;
}

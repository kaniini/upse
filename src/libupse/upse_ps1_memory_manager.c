/*
 * UPSE: the unix playstation sound emulator.
 *
 * Filename: upse_ps1_memory_manager.c
 * Purpose: libupse: UPSE PS1 Memory Manager
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
#include <stdlib.h>

#include "upse-internal.h"

void LoadPSXMem(upse_module_instance_t *ins, u32 address, s32 length, unsigned char *data)
{
    while (length > 0)
    {
	if (address & 65535)
	{
	    u32 tmplen;

	    tmplen = ((65536 - (address & 65535)) > (u32) length) ? (u32) length : 65536 - (address & 65535);
	    if (ins->upse_ps1_memory_LUT[address >> 16])
		memcpy((char *) (ins->upse_ps1_memory_LUT[address >> 16] + (address & 65535)), data, tmplen);
	    address += tmplen;
	    data += tmplen;
	    length -= tmplen;
	    continue;
	}
	if (ins->upse_ps1_memory_LUT[address >> 16])
	{
	    memcpy((char *) (ins->upse_ps1_memory_LUT[address >> 16]), data, (length < 65536) ? length : 65536);
	}
	data += 65536;
	address += 65536;
	length -= 65536;
    }
}

void ClearPSXMem(upse_module_instance_t *ins, u32 address, s32 length)
{
    while (length > 0)
    {
	if (address & 65535)
	{
	    u32 tmplen;

	    tmplen = ((65536 - (address & 65535)) > (u32) length) ? (u32) length : 65536 - (address & 65535);
	    if (ins->upse_ps1_memory_LUT[address >> 16])
		memset((char *) (ins->upse_ps1_memory_LUT[address >> 16] + (address & 65535)), '\0', tmplen);
	    address += tmplen;
	    length -= tmplen;
	    continue;
	}
	if (ins->upse_ps1_memory_LUT[address >> 16])
	{
	    memset((char *) (ins->upse_ps1_memory_LUT[address >> 16]), '\0', (length < 65536) ? length : 65536);
	}
	address += 65536;
	length -= 65536;
    }
}

static int writeok;
int upse_ps1_memory_init(upse_module_instance_t *ins)
{
    int i;

    writeok = 1;

    ins->upse_ps1_memory_LUT = malloc(0x10000 * sizeof *ins->upse_ps1_memory_LUT);
    memset(ins->upse_ps1_memory_LUT, 0, 0x10000 * sizeof *ins->upse_ps1_memory_LUT);

    ins->psxM = (char *) malloc(0x00200000);
    ins->psxP = (char *) malloc(0x00010000);
    ins->psxH = (char *) malloc(0x00010000);
    ins->psxR = (char *) malloc(0x00080000);
    if (ins->upse_ps1_memory_LUT == NULL || ins->psxM == NULL || ins->psxP == NULL || ins->psxH == NULL || ins->psxR == NULL)
    {
	printf("Error allocating memory");
	return -1;
    }

    for (i = 0; i < 0x80; i++)
	ins->upse_ps1_memory_LUT[i + 0x0000] = &ins->psxM[(i & 0x1f) << 16];

    memcpy(ins->upse_ps1_memory_LUT + 0x8000, ins->upse_ps1_memory_LUT, 0x80 * sizeof *ins->upse_ps1_memory_LUT);
    memcpy(ins->upse_ps1_memory_LUT + 0xa000, ins->upse_ps1_memory_LUT, 0x80 * sizeof *ins->upse_ps1_memory_LUT);

    for (i = 0; i < 0x01; i++)
	ins->upse_ps1_memory_LUT[i + 0x1f00] = &ins->psxP[i << 16];

    for (i = 0; i < 0x01; i++)
	ins->upse_ps1_memory_LUT[i + 0x1f80] = &ins->psxH[i << 16];

    for (i = 0; i < 0x08; i++)
	ins->upse_ps1_memory_LUT[i + 0xbfc0] = &ins->psxR[i << 16];

    return 0;
}

void upse_ps1_memory_reset(upse_module_instance_t *ins)
{
    memset(ins->psxM, 0, 0x00200000);
    memset(ins->psxP, 0, 0x00010000);

    if (upse_has_custom_bios())
    {
        FILE *f;

        _DEBUG("custom bios: %s", upse_get_custom_bios());

        f = fopen(upse_get_custom_bios(), "rb");
        if (f == NULL) {
            _DEBUG("opening custom bios failed");
            memset(ins->psxR, 0, 0x80000);
            upse_set_custom_bios(NULL);
        } else {
            fread(ins->psxR, 1, 0x80000, f);
            fclose(f);
        }
    }
}

void upse_ps1_memory_shutdown(upse_module_instance_t *ins)
{
    if (ins->psxM)
	free(ins->psxM);

    if (ins->psxP)
	free(ins->psxP);

    if (ins->psxH)
	free(ins->psxH);

    if (ins->psxR)
	free(ins->psxR);

    if (ins->upse_ps1_memory_LUT)
	free(ins->upse_ps1_memory_LUT);

    ins->psxM = ins->psxP = ins->psxH = ins->psxR = NULL;
    ins->upse_ps1_memory_LUT = NULL;
}

u8 upse_ps1_memory_read_8(upse_module_instance_t *ins, u32 mem)
{
    char *p;
    u32 t;

    t = mem >> 16;
    if (t == 0x1f80)
    {
	if (mem < 0x1f801000)
	    return psxHu8(ins, mem);
	else
	    return upse_ps1_hal_read_8(ins, mem);
    }
    else
    {
	p = (char *) (ins->upse_ps1_memory_LUT[t]);
	if (p != NULL)
	{
	    return *(u8 *) (p + (mem & 0xffff));
	}
	else
	{
	    return 0;
	}
    }
}

u16 upse_ps1_memory_read_16(upse_module_instance_t *ins, u32 mem)
{
    char *p;
    u32 t;

    t = mem >> 16;
    if (t == 0x1f80)
    {
	if (mem < 0x1f801000)
	    return BFLIP16(psxHu16(ins, mem));
	else
	    return upse_ps1_hal_read_16(ins, mem);
    }
    else
    {
	p = (char *) (ins->upse_ps1_memory_LUT[t]);
	if (p != NULL)
	{
	    return BFLIP16(*(u16 *) (p + (mem & 0xffff)));
	}
	else
	{
	    return 0;
	}
    }
}

u32 upse_ps1_memory_read_32(upse_module_instance_t *ins, u32 mem)
{
    char *p;
    u32 t;

    t = mem >> 16;
    if (t == 0x1f80)
    {
	if (mem < 0x1f801000)
	    return BFLIP32(psxHu32(ins, mem));
	else
	    return upse_ps1_hal_read_32(ins, mem);
    }
    else
    {
	p = (char *) (ins->upse_ps1_memory_LUT[t]);
	if (p != NULL)
	{
	    return BFLIP32(*(u32 *) (p + (mem & 0xffff)));
	}
	else
	{
	    return 0;
	}
    }
}

void upse_ps1_memory_write_8(upse_module_instance_t *ins, u32 mem, u8 value)
{
    char *p;
    u32 t;

    t = mem >> 16;
    if (t == 0x1f80)
    {
	if (mem < 0x1f801000)
	    psxHu8(ins, mem) = value;
	else
	    upse_ps1_hal_write_8(ins, mem, value);
    }
    else
    {
	p = (char *) (ins->upse_ps1_memory_LUT[t]);
	if (p != NULL)
	{
	    *(u8 *) (p + (mem & 0xffff)) = value;
	}
    }
}

void upse_ps1_memory_write_16(upse_module_instance_t *ins, u32 mem, u16 value)
{
    char *p;
    u32 t;

    t = mem >> 16;
    if (t == 0x1f80)
    {
	if (mem < 0x1f801000)
	    psxHu16(ins, mem) = BFLIP16(value);
	else
	    upse_ps1_hal_write_16(ins, mem, value);
    }
    else
    {
	p = (char *) (ins->upse_ps1_memory_LUT[t]);
	if (p != NULL)
	{
	    *(u16 *) (p + (mem & 0xffff)) = BFLIP16(value);
	}
    }
}

void upse_ps1_memory_write_32(upse_module_instance_t *ins, u32 mem, u32 value)
{
    char *p;
    u32 t;

    t = mem >> 16;
    if (t == 0x1f80)
    {
	if (mem < 0x1f801000)
	    psxHu32(ins, mem) = BFLIP32(value);
	else
	    upse_ps1_hal_write_32(ins, mem, value);
    }
    else
    {
	p = (char *) (ins->upse_ps1_memory_LUT[t]);
	if (p != NULL)
	{
	    *(u32 *) (p + (mem & 0xffff)) = BFLIP32(value);
	}
	else
	{
	    if (mem != 0xfffe0130)
	    {

	    }
	    else
	    {
		int i;

		switch (value)
		{
		  case 0x800:
		  case 0x804:
		      if (writeok == 0)
			  break;
		      writeok = 0;
		      memset(ins->upse_ps1_memory_LUT + 0x0000, 0, 0x80 * sizeof *ins->upse_ps1_memory_LUT);
		      memset(ins->upse_ps1_memory_LUT + 0x8000, 0, 0x80 * sizeof *ins->upse_ps1_memory_LUT);
		      memset(ins->upse_ps1_memory_LUT + 0xa000, 0, 0x80 * sizeof *ins->upse_ps1_memory_LUT);
		      break;
		  case 0x1e988:
		      if (writeok == 1)
			  break;
		      writeok = 1;
		      for (i = 0; i < 0x80; i++)
			  ins->upse_ps1_memory_LUT[i + 0x0000] = &ins->psxM[(i & 0x1f) << 16];
		      memcpy(ins->upse_ps1_memory_LUT + 0x8000, ins->upse_ps1_memory_LUT, 0x80 * sizeof *ins->upse_ps1_memory_LUT);
		      memcpy(ins->upse_ps1_memory_LUT + 0xa000, ins->upse_ps1_memory_LUT, 0x80 * sizeof *ins->upse_ps1_memory_LUT);
		      break;
		  default:
		      break;
		}
	    }
	}
    }
}

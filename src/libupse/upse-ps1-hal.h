/*
 * UPSE: the unix playstation sound emulator.
 *
 * Filename: upse-ps1-hal.h
 * Purpose: libupse: PS1 HAL implementation header
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

#ifndef _UPSE__LIBUPSE__UPSE_PS1_HAL_H__GUARD
#define _UPSE__LIBUPSE__UPSE_PS1_HAL_H__GUARD

void psxHwReset();
u8 psxHwRead8(u32 add);
u16 psxHwRead16(u32 add);
u32 psxHwRead32(u32 add);
void psxHwWrite8(u32 add, u8 value);
void psxHwWrite16(u32 add, u16 value);
void psxHwWrite32(u32 add, u32 value);

#endif

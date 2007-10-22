/*
 * UPSE: the unix playstation sound emulator.
 *
 * Filename: upse-ps1-bios-private.h
 * Purpose: libupse: UPSE PS1 FakeBIOS private defines/functions
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

#ifndef _UPSE__LIBUPSE__UPSE_PS1_BIOS_PRIVATE_H__GUARD
#define _UPSE__LIBUPSE__UPSE_PS1_BIOS_PRIVATE_H__GUARD

//#define r0 (psxRegs.GPR.n.r0)
#define at (psxRegs.GPR.n.at)
#define v0 (psxRegs.GPR.n.v0)
#define v1 (psxRegs.GPR.n.v1)
#define a0 (psxRegs.GPR.n.a0)
#define a1 (psxRegs.GPR.n.a1)
#define a2 (psxRegs.GPR.n.a2)
#define a3 (psxRegs.GPR.n.a3)
#define t0 (psxRegs.GPR.n.t0)
#define t1 (psxRegs.GPR.n.t1)
#define t2 (psxRegs.GPR.n.t2)
#define t3 (psxRegs.GPR.n.t3)
#define t4 (psxRegs.GPR.n.t4)
#define t5 (psxRegs.GPR.n.t5)
#define t6 (psxRegs.GPR.n.t6)
#define t7 (psxRegs.GPR.n.t7)
#define s0 (psxRegs.GPR.n.s0)
#define s1 (psxRegs.GPR.n.s1)
#define s2 (psxRegs.GPR.n.s2)
#define s3 (psxRegs.GPR.n.s3)
#define s4 (psxRegs.GPR.n.s4)
#define s5 (psxRegs.GPR.n.s5)
#define s6 (psxRegs.GPR.n.s6)
#define s7 (psxRegs.GPR.n.s7)
#define t8 (psxRegs.GPR.n.t6)
#define t9 (psxRegs.GPR.n.t7)
#define k0 (psxRegs.GPR.n.k0)
#define k1 (psxRegs.GPR.n.k1)
#define gp (psxRegs.GPR.n.gp)
#define sp (psxRegs.GPR.n.sp)
#define fp (psxRegs.GPR.n.s8)
#define ra (psxRegs.GPR.n.ra)
#define pc0 (psxRegs.pc)

#define Ra0 ((char*)PSXM(a0))
#define Ra1 ((char*)PSXM(a1))
#define Ra2 ((char*)PSXM(a2))
#define Ra3 ((char*)PSXM(a3))
#define Rv0 ((char*)PSXM(v0))
#define Rsp ((char*)PSXM(sp))

#endif

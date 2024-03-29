/*
 * UPSE: the unix playstation sound emulator.
 *
 * Filename: upse_ps1_spu_reverb.c
 * Purpose: libupse: PS1 SPU reverb
 *
 * Copyright (c) 2007 William Pitcock <nenolod@sacredspiral.co.uk>
 * Portions copyright (c) 2002 Pete Bernert <BlackDove@addcom.de>
 * Portions copyright (c) 2003 Neill Corlett
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
#include "upse-spu-internal.h"
#include "upse-debug.h"

////////////////////////////////////////////////////////////////////////
// globals
////////////////////////////////////////////////////////////////////////

// REVERB info and timing vars...

////////////////////////////////////////////////////////////////////////

// get_buffer content helper: takes care about wraps
static INLINE s64 g_buffer(upse_spu_state_t *spu, int iOff)
{
    s16 *p = (s16 *) spu->spuMem;
    iOff = (iOff * 4) + spu->rvb.CurrAddr;
    while (iOff > 0x3FFFF)
	iOff = spu->rvb.StartAddr + (iOff - 0x40000);
    while (iOff < spu->rvb.StartAddr)
	iOff = 0x3ffff - (spu->rvb.StartAddr - iOff);
    return (int) (s16) BFLIP16(*(p + iOff));
}

////////////////////////////////////////////////////////////////////////

// set_buffer content helper: takes care about wraps and clipping
static INLINE void s_buffer(upse_spu_state_t *spu, int iOff, int iVal)
{
    s16 *p = (s16 *) spu->spuMem;
    iOff = (iOff * 4) + spu->rvb.CurrAddr;
    while (iOff > 0x3FFFF)
	iOff = spu->rvb.StartAddr + (iOff - 0x40000);
    while (iOff < spu->rvb.StartAddr)
	iOff = 0x3ffff - (spu->rvb.StartAddr - iOff);
    if (iVal < -32768L)
	iVal = -32768L;
    if (iVal > 32767L)
	iVal = 32767L;
    *(p + iOff) = (s16) BFLIP16((s16) iVal);
}

////////////////////////////////////////////////////////////////////////

// set_buffer (+1 sample) content helper: takes care about wraps and clipping
static INLINE void s_buffer1(upse_spu_state_t *spu, int iOff, int iVal)
{
    s16 *p = (s16 *) spu->spuMem;
    iOff = (iOff * 4) + spu->rvb.CurrAddr + 1;
    while (iOff > 0x3FFFF)
	iOff = spu->rvb.StartAddr + (iOff - 0x40000);
    while (iOff < spu->rvb.StartAddr)
	iOff = 0x3ffff - (spu->rvb.StartAddr - iOff);
    if (iVal < -32768L)
	iVal = -32768L;
    if (iVal > 32767L)
	iVal = 32767L;
    *(p + iOff) = (s16) BFLIP16((s16) iVal);
}

static u32 _do_reverb = 1;
static u32 _reverb_no_downsample = 0;

void upse_set_reverb_mode(u32 setting)
{
    _ENTER;

    _do_reverb = setting;

    _LEAVE;
}

void upse_set_reverb_no_downsample(u32 setting)
{
    _ENTER;

    _reverb_no_downsample = setting;

    _LEAVE;
}

static const s32 downcoeffs[8] = { 1283, 5344, 10895, 15243, 15243, 10895, 5344, 1283 };

void MixREVERBLeftRight(upse_spu_state_t *spu, s32 * oleft, s32 * oright, s32 inleft, s32 inright)
{
    int x;

    if (!spu->rvb.StartAddr || !_do_reverb)	// reverb is off or forcefully disabled
    {
	spu->rvb.iRVBLeft = spu->rvb.iRVBRight = 0;
	return;
    }

    //if(inleft<-32767 || inleft>32767) printf("%d\n",inleft);
    //if(inright<-32767 || inright>32767) printf("%d\n",inright);
    spu->downbuf[0][spu->dbpos] = inleft;
    spu->downbuf[1][spu->dbpos] = inright;
    spu->dbpos = (spu->dbpos + 1) & 7;

    if (spu->dbpos & 1 || _reverb_no_downsample)	// we work on every second left value: downsample to 22 khz
    {
	if (spu->spuCtrl & 0x80)	// -> reverb on? oki
	{
	    int ACC0, ACC1, FB_A0, FB_A1, FB_B0, FB_B1;
	    s32 INPUT_SAMPLE_L = 0;
	    s32 INPUT_SAMPLE_R = 0;

	    for (x = 0; x < 8; x++)
	    {
		INPUT_SAMPLE_L += (spu->downbuf[0][(spu->dbpos + x) & 7] * downcoeffs[x]) >> 8;	/* Lose insignificant
											   digits to prevent
											   overflow(check this) */
		INPUT_SAMPLE_R += (spu->downbuf[1][(spu->dbpos + x) & 7] * downcoeffs[x]) >> 8;
	    }

	    INPUT_SAMPLE_L >>= (16 - 8);
	    INPUT_SAMPLE_R >>= (16 - 8);

	    {
		const s64 IIR_INPUT_A0 = ((g_buffer(spu, spu->rvb.IIR_SRC_A0) * spu->rvb.IIR_COEF) >> 15) + ((INPUT_SAMPLE_L * spu->rvb.IN_COEF_L) >> 15);
		const s64 IIR_INPUT_A1 = ((g_buffer(spu, spu->rvb.IIR_SRC_A1) * spu->rvb.IIR_COEF) >> 15) + ((INPUT_SAMPLE_R * spu->rvb.IN_COEF_R) >> 15);
		const s64 IIR_INPUT_B0 = ((g_buffer(spu, spu->rvb.IIR_SRC_B0) * spu->rvb.IIR_COEF) >> 15) + ((INPUT_SAMPLE_L * spu->rvb.IN_COEF_L) >> 15);
		const s64 IIR_INPUT_B1 = ((g_buffer(spu, spu->rvb.IIR_SRC_B1) * spu->rvb.IIR_COEF) >> 15) + ((INPUT_SAMPLE_R * spu->rvb.IN_COEF_R) >> 15);
		const s64 IIR_A0 = ((IIR_INPUT_A0 * spu->rvb.IIR_ALPHA) >> 15) + ((g_buffer(spu, spu->rvb.IIR_DEST_A0) * (32768L - spu->rvb.IIR_ALPHA)) >> 15);
		const s64 IIR_A1 = ((IIR_INPUT_A1 * spu->rvb.IIR_ALPHA) >> 15) + ((g_buffer(spu, spu->rvb.IIR_DEST_A1) * (32768L - spu->rvb.IIR_ALPHA)) >> 15);
		const s64 IIR_B0 = ((IIR_INPUT_B0 * spu->rvb.IIR_ALPHA) >> 15) + ((g_buffer(spu, spu->rvb.IIR_DEST_B0) * (32768L - spu->rvb.IIR_ALPHA)) >> 15);
		const s64 IIR_B1 = ((IIR_INPUT_B1 * spu->rvb.IIR_ALPHA) >> 15) + ((g_buffer(spu, spu->rvb.IIR_DEST_B1) * (32768L - spu->rvb.IIR_ALPHA)) >> 15);

		s_buffer1(spu, spu->rvb.IIR_DEST_A0, IIR_A0);
		s_buffer1(spu, spu->rvb.IIR_DEST_A1, IIR_A1);
		s_buffer1(spu, spu->rvb.IIR_DEST_B0, IIR_B0);
		s_buffer1(spu, spu->rvb.IIR_DEST_B1, IIR_B1);

		ACC0 = ((g_buffer(spu, spu->rvb.ACC_SRC_A0) * spu->rvb.ACC_COEF_A) >> 15) +
		    ((g_buffer(spu, spu->rvb.ACC_SRC_B0) * spu->rvb.ACC_COEF_B) >> 15) + ((g_buffer(spu, spu->rvb.ACC_SRC_C0) * spu->rvb.ACC_COEF_C) >> 15) + ((g_buffer(spu, spu->rvb.ACC_SRC_D0) * spu->rvb.ACC_COEF_D) >> 15);
		ACC1 = ((g_buffer(spu, spu->rvb.ACC_SRC_A1) * spu->rvb.ACC_COEF_A) >> 15) +
		    ((g_buffer(spu, spu->rvb.ACC_SRC_B1) * spu->rvb.ACC_COEF_B) >> 15) + ((g_buffer(spu, spu->rvb.ACC_SRC_C1) * spu->rvb.ACC_COEF_C) >> 15) + ((g_buffer(spu, spu->rvb.ACC_SRC_D1) * spu->rvb.ACC_COEF_D) >> 15);

		FB_A0 = g_buffer(spu, spu->rvb.MIX_DEST_A0 - spu->rvb.FB_SRC_A);
		FB_A1 = g_buffer(spu, spu->rvb.MIX_DEST_A1 - spu->rvb.FB_SRC_A);
		FB_B0 = g_buffer(spu, spu->rvb.MIX_DEST_B0 - spu->rvb.FB_SRC_B);
		FB_B1 = g_buffer(spu, spu->rvb.MIX_DEST_B1 - spu->rvb.FB_SRC_B);

		s_buffer(spu, spu->rvb.MIX_DEST_A0, ACC0 - ((FB_A0 * spu->rvb.FB_ALPHA) >> 15));
		s_buffer(spu, spu->rvb.MIX_DEST_A1, ACC1 - ((FB_A1 * spu->rvb.FB_ALPHA) >> 15));

		s_buffer(spu, spu->rvb.MIX_DEST_B0, ((spu->rvb.FB_ALPHA * ACC0) >> 15) - ((FB_A0 * (int) (spu->rvb.FB_ALPHA ^ 0xFFFF8000)) >> 15) - ((FB_B0 * spu->rvb.FB_X) >> 15));
		s_buffer(spu, spu->rvb.MIX_DEST_B1, ((spu->rvb.FB_ALPHA * ACC1) >> 15) - ((FB_A1 * (int) (spu->rvb.FB_ALPHA ^ 0xFFFF8000)) >> 15) - ((FB_B1 * spu->rvb.FB_X) >> 15));

		spu->rvb.iRVBLeft = (g_buffer(spu, spu->rvb.MIX_DEST_A0) + g_buffer(spu, spu->rvb.MIX_DEST_B0)) / 3;
		spu->rvb.iRVBRight = (g_buffer(spu, spu->rvb.MIX_DEST_A1) + g_buffer(spu, spu->rvb.MIX_DEST_B1)) / 3;

		spu->rvb.iRVBLeft = ((s64) spu->rvb.iRVBLeft * spu->rvb.VolLeft) >> 14;
		spu->rvb.iRVBRight = ((s64) spu->rvb.iRVBRight * spu->rvb.VolRight) >> 14;

		spu->upbuf[0][spu->ubpos] = spu->rvb.iRVBLeft;
		spu->upbuf[1][spu->ubpos] = spu->rvb.iRVBRight;
		spu->ubpos = (spu->ubpos + 1) & 7;
	    }
	}
	else			// -> reverb off
	{
	    spu->rvb.iRVBLeft = spu->rvb.iRVBRight = 0;
	    return;
	}
	spu->rvb.CurrAddr++;
	if (spu->rvb.CurrAddr > 0x3ffff)
	    spu->rvb.CurrAddr = spu->rvb.StartAddr;
    }
    else
    {
	spu->upbuf[0][spu->ubpos] = 0;
	spu->upbuf[1][spu->ubpos] = 0;
	spu->ubpos = (spu->ubpos + 1) & 7;
    }
    {
	s32 retl = 0, retr = 0;
	for (x = 0; x < 8; x++)
	{
	    retl += (spu->upbuf[0][(spu->ubpos + x) & 7] * downcoeffs[x]) >> 8;
	    retr += (spu->upbuf[1][(spu->ubpos + x) & 7] * downcoeffs[x]) >> 8;
	}
	retl >>= (16 - 8 - 1);	/* -1 To adjust for the null padding. */
	retr >>= (16 - 8 - 1);

	*oleft += retl;
	*oright += retr;
    }
}

////////////////////////////////////////////////////////////////////////

/*
-----------------------------------------------------------------------------
PSX reverb hardware notes
by Neill Corlett
-----------------------------------------------------------------------------

Yadda yadda disclaimer yadda probably not perfect yadda well it's okay anyway
yadda yadda.

-----------------------------------------------------------------------------

Basics
------

- The reverb buffer is 22khz 16-bit mono PCM.
- It starts at the reverb address given by 1DA2, extends to
  the end of sound RAM, and wraps back to the 1DA2 address.

Setting the address at 1DA2 resets the current reverb work address.

This work address ALWAYS increments every 1/22050 sec., regardless of
whether reverb is enabled (bit 7 of 1DAA set).

And the contents of the reverb buffer ALWAYS play, scaled by the
"reverberation depth left/right" volumes (1D84/1D86).
(which, by the way, appear to be scaled so 3FFF=approx. 1.0, 4000=-1.0)

-----------------------------------------------------------------------------

Register names
--------------

These are probably not their real names.
These are probably not even correct names.
We will use them anyway, because we can.

1DC0: FB_SRC_A       (offset)
1DC2: FB_SRC_B       (offset)
1DC4: IIR_ALPHA      (coef.)
1DC6: ACC_COEF_A     (coef.)
1DC8: ACC_COEF_B     (coef.)
1DCA: ACC_COEF_C     (coef.)
1DCC: ACC_COEF_D     (coef.)
1DCE: IIR_COEF       (coef.)
1DD0: FB_ALPHA       (coef.)
1DD2: FB_X           (coef.)
1DD4: IIR_DEST_A0    (offset)
1DD6: IIR_DEST_A1    (offset)
1DD8: ACC_SRC_A0     (offset)
1DDA: ACC_SRC_A1     (offset)
1DDC: ACC_SRC_B0     (offset)
1DDE: ACC_SRC_B1     (offset)
1DE0: IIR_SRC_A0     (offset)
1DE2: IIR_SRC_A1     (offset)
1DE4: IIR_DEST_B0    (offset)
1DE6: IIR_DEST_B1    (offset)
1DE8: ACC_SRC_C0     (offset)
1DEA: ACC_SRC_C1     (offset)
1DEC: ACC_SRC_D0     (offset)
1DEE: ACC_SRC_D1     (offset)
1DF0: IIR_SRC_B1     (offset)
1DF2: IIR_SRC_B0     (offset)
1DF4: MIX_DEST_A0    (offset)
1DF6: MIX_DEST_A1    (offset)
1DF8: MIX_DEST_B0    (offset)
1DFA: MIX_DEST_B1    (offset)
1DFC: IN_COEF_L      (coef.)
1DFE: IN_COEF_R      (coef.)

The coefficients are signed fractional values.
-32768 would be -1.0
 32768 would be  1.0 (if it were possible... the highest is of course 32767)

The offsets are (byte/8) offsets into the reverb buffer.
i.e. you multiply them by 8, you get byte offsets.
You can also think of them as (samples/4) offsets.
They appear to be signed.  They can be negative.
None of the documented presets make them negative, though.

Yes, 1DF0 and 1DF2 appear to be backwards.  Not a typo.

-----------------------------------------------------------------------------

What it does
------------

We take all reverb sources:
- regular channels that have the reverb bit on
- cd and external sources, if their reverb bits are on
and mix them into one stereo 44100hz signal.

Lowpass/downsample that to 22050hz.  The PSX uses a proper bandlimiting
algorithm here, but I haven't figured out the hysterically exact specifics.
I use an 8-tap filter with these coefficients, which are nice but probably
not the real ones:

0.037828187894
0.157538631280
0.321159685278
0.449322115345
0.449322115345
0.321159685278
0.157538631280
0.037828187894

So we have two input samples (INPUT_SAMPLE_L, INPUT_SAMPLE_R) every 22050hz.

* IN MY EMULATION, I divide these by 2 to make it clip less.
  (and of course the L/R output coefficients are adjusted to compensate)
  The real thing appears to not do this.

At every 22050hz tick:
- If the reverb bit is enabled (bit 7 of 1DAA), execute the reverb
  steady-state algorithm described below
- AFTERWARDS, retrieve the "wet out" L and R samples from the reverb buffer
  (This part may not be exactly right and I guessed at the coefs. TODO: check later.)
  L is: 0.333 * (buffer[MIX_DEST_A0] + buffer[MIX_DEST_B0])
  R is: 0.333 * (buffer[MIX_DEST_A1] + buffer[MIX_DEST_B1])
- Advance the current buffer position by 1 sample

The wet out L and R are then upsampled to 44100hz and played at the
"reverberation depth left/right" (1D84/1D86) volume, independent of the main
volume.

-----------------------------------------------------------------------------

Reverb steady-state
-------------------

The reverb steady-state algorithm is fairly clever, and of course by
"clever" I mean "batshit insane".

buffer[x] is relative to the current buffer position, not the beginning of
the buffer.  Note that all buffer offsets must wrap around so they're
contained within the reverb work area.

Clipping is performed at the end... maybe also sooner, but definitely at
the end.

IIR_INPUT_A0 = buffer[IIR_SRC_A0] * IIR_COEF + INPUT_SAMPLE_L * IN_COEF_L;
IIR_INPUT_A1 = buffer[IIR_SRC_A1] * IIR_COEF + INPUT_SAMPLE_R * IN_COEF_R;
IIR_INPUT_B0 = buffer[IIR_SRC_B0] * IIR_COEF + INPUT_SAMPLE_L * IN_COEF_L;
IIR_INPUT_B1 = buffer[IIR_SRC_B1] * IIR_COEF + INPUT_SAMPLE_R * IN_COEF_R;

IIR_A0 = IIR_INPUT_A0 * IIR_ALPHA + buffer[IIR_DEST_A0] * (1.0 - IIR_ALPHA);
IIR_A1 = IIR_INPUT_A1 * IIR_ALPHA + buffer[IIR_DEST_A1] * (1.0 - IIR_ALPHA);
IIR_B0 = IIR_INPUT_B0 * IIR_ALPHA + buffer[IIR_DEST_B0] * (1.0 - IIR_ALPHA);
IIR_B1 = IIR_INPUT_B1 * IIR_ALPHA + buffer[IIR_DEST_B1] * (1.0 - IIR_ALPHA);

buffer[IIR_DEST_A0 + 1sample] = IIR_A0;
buffer[IIR_DEST_A1 + 1sample] = IIR_A1;
buffer[IIR_DEST_B0 + 1sample] = IIR_B0;
buffer[IIR_DEST_B1 + 1sample] = IIR_B1;

ACC0 = buffer[ACC_SRC_A0] * ACC_COEF_A +
       buffer[ACC_SRC_B0] * ACC_COEF_B +
       buffer[ACC_SRC_C0] * ACC_COEF_C +
       buffer[ACC_SRC_D0] * ACC_COEF_D;
ACC1 = buffer[ACC_SRC_A1] * ACC_COEF_A +
       buffer[ACC_SRC_B1] * ACC_COEF_B +
       buffer[ACC_SRC_C1] * ACC_COEF_C +
       buffer[ACC_SRC_D1] * ACC_COEF_D;

FB_A0 = buffer[MIX_DEST_A0 - FB_SRC_A];
FB_A1 = buffer[MIX_DEST_A1 - FB_SRC_A];
FB_B0 = buffer[MIX_DEST_B0 - FB_SRC_B];
FB_B1 = buffer[MIX_DEST_B1 - FB_SRC_B];

buffer[MIX_DEST_A0] = ACC0 - FB_A0 * FB_ALPHA;
buffer[MIX_DEST_A1] = ACC1 - FB_A1 * FB_ALPHA;
buffer[MIX_DEST_B0] = (FB_ALPHA * ACC0) - FB_A0 * (FB_ALPHA^0x8000) - FB_B0 * FB_X;
buffer[MIX_DEST_B1] = (FB_ALPHA * ACC1) - FB_A1 * (FB_ALPHA^0x8000) - FB_B1 * FB_X;

-----------------------------------------------------------------------------
*/

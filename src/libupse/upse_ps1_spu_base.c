/*
 * UPSE: the unix playstation sound emulator.
 *
 * Filename: upse_ps1_spu_base.c
 * Purpose: libupse: PS1 SPU base code
 *
 * Copyright (c) 2007 William Pitcock <nenolod@sacredspiral.co.uk>
 * Portions copyright (c) 2002 Pete Bernert <BlackDove@addcom.de>
 *
 * UPSE is free software, released under the GNU General Public License,
 * version 2.
 *
 * A copy of the GNU General Public License, version 2, is included in
 * the UPSE source kit as COPYING.
 *
 * UPSE is offered without any warranty of any kind, explicit or implicit.
 */

#define _IN_SPU

#include "upse-types.h"
#include "upse-internal.h"
#include "upse-ps1-spu-base.h"
#include "upse-spu-internal.h"
#include "upse-ps1-spu-register-io.h"
#include "upse-ps1-spu-reverb.h"
#include "upse-ps1-spu-adsr-filter.h"

#include "upse.h"
#include "upse-ps1-memory-manager.h"

////////////////////////////////////////////////////////////////////////
// globals
////////////////////////////////////////////////////////////////////////

// psx buffer / addresses

upse_spu_state_t *spu;

// user settings          
static int iVolume;

// MAIN infos struct for each channel
static const int f[5][2] = {
    {0, 0},
    {60, 0},
    {115, -52},
    {98, -55},
    {122, -60}
};

////////////////////////////////////////////////////////////////////////
// CODE AREA
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
// helpers for so-called "gauss interpolation"

#define gval0 (((int *)(&spu->s_chan[ch].SB[29]))[gpos])
#define gval(x) (((int *)(&spu->s_chan[ch].SB[29]))[(gpos+x)&3])

// 1024 entries
const int gauss[] = {
    0x172, 0x519, 0x176, 0x000, 0x16E, 0x519, 0x17A, 0x000,
    0x16A, 0x518, 0x17D, 0x000, 0x166, 0x518, 0x181, 0x000,
    0x162, 0x518, 0x185, 0x000, 0x15F, 0x518, 0x189, 0x000,
    0x15B, 0x518, 0x18D, 0x000, 0x157, 0x517, 0x191, 0x000,
    0x153, 0x517, 0x195, 0x000, 0x150, 0x517, 0x19A, 0x000,
    0x14C, 0x516, 0x19E, 0x000, 0x148, 0x516, 0x1A2, 0x000,
    0x145, 0x515, 0x1A6, 0x000, 0x141, 0x514, 0x1AA, 0x000,
    0x13E, 0x514, 0x1AE, 0x000, 0x13A, 0x513, 0x1B2, 0x000,
    0x137, 0x512, 0x1B7, 0x001, 0x133, 0x511, 0x1BB, 0x001,
    0x130, 0x511, 0x1BF, 0x001, 0x12C, 0x510, 0x1C3, 0x001,
    0x129, 0x50F, 0x1C8, 0x001, 0x125, 0x50E, 0x1CC, 0x001,
    0x122, 0x50D, 0x1D0, 0x001, 0x11E, 0x50C, 0x1D5, 0x001,
    0x11B, 0x50B, 0x1D9, 0x001, 0x118, 0x50A, 0x1DD, 0x001,
    0x114, 0x508, 0x1E2, 0x001, 0x111, 0x507, 0x1E6, 0x002,
    0x10E, 0x506, 0x1EB, 0x002, 0x10B, 0x504, 0x1EF, 0x002,
    0x107, 0x503, 0x1F3, 0x002, 0x104, 0x502, 0x1F8, 0x002,
    0x101, 0x500, 0x1FC, 0x002, 0x0FE, 0x4FF, 0x201, 0x002,
    0x0FB, 0x4FD, 0x205, 0x003, 0x0F8, 0x4FB, 0x20A, 0x003,
    0x0F5, 0x4FA, 0x20F, 0x003, 0x0F2, 0x4F8, 0x213, 0x003,
    0x0EF, 0x4F6, 0x218, 0x003, 0x0EC, 0x4F5, 0x21C, 0x004,
    0x0E9, 0x4F3, 0x221, 0x004, 0x0E6, 0x4F1, 0x226, 0x004,
    0x0E3, 0x4EF, 0x22A, 0x004, 0x0E0, 0x4ED, 0x22F, 0x004,
    0x0DD, 0x4EB, 0x233, 0x005, 0x0DA, 0x4E9, 0x238, 0x005,
    0x0D7, 0x4E7, 0x23D, 0x005, 0x0D4, 0x4E5, 0x241, 0x005,
    0x0D2, 0x4E3, 0x246, 0x006, 0x0CF, 0x4E0, 0x24B, 0x006,
    0x0CC, 0x4DE, 0x250, 0x006, 0x0C9, 0x4DC, 0x254, 0x006,
    0x0C7, 0x4D9, 0x259, 0x007, 0x0C4, 0x4D7, 0x25E, 0x007,
    0x0C1, 0x4D5, 0x263, 0x007, 0x0BF, 0x4D2, 0x267, 0x008,
    0x0BC, 0x4D0, 0x26C, 0x008, 0x0BA, 0x4CD, 0x271, 0x008,
    0x0B7, 0x4CB, 0x276, 0x009, 0x0B4, 0x4C8, 0x27B, 0x009,
    0x0B2, 0x4C5, 0x280, 0x009, 0x0AF, 0x4C3, 0x284, 0x00A,
    0x0AD, 0x4C0, 0x289, 0x00A, 0x0AB, 0x4BD, 0x28E, 0x00A,
    0x0A8, 0x4BA, 0x293, 0x00B, 0x0A6, 0x4B7, 0x298, 0x00B,
    0x0A3, 0x4B5, 0x29D, 0x00B, 0x0A1, 0x4B2, 0x2A2, 0x00C,
    0x09F, 0x4AF, 0x2A6, 0x00C, 0x09C, 0x4AC, 0x2AB, 0x00D,
    0x09A, 0x4A9, 0x2B0, 0x00D, 0x098, 0x4A6, 0x2B5, 0x00E,
    0x096, 0x4A2, 0x2BA, 0x00E, 0x093, 0x49F, 0x2BF, 0x00F,
    0x091, 0x49C, 0x2C4, 0x00F, 0x08F, 0x499, 0x2C9, 0x00F,
    0x08D, 0x496, 0x2CE, 0x010, 0x08B, 0x492, 0x2D3, 0x010,
    0x089, 0x48F, 0x2D8, 0x011, 0x086, 0x48C, 0x2DC, 0x011,
    0x084, 0x488, 0x2E1, 0x012, 0x082, 0x485, 0x2E6, 0x013,
    0x080, 0x481, 0x2EB, 0x013, 0x07E, 0x47E, 0x2F0, 0x014,
    0x07C, 0x47A, 0x2F5, 0x014, 0x07A, 0x477, 0x2FA, 0x015,
    0x078, 0x473, 0x2FF, 0x015, 0x076, 0x470, 0x304, 0x016,
    0x075, 0x46C, 0x309, 0x017, 0x073, 0x468, 0x30E, 0x017,
    0x071, 0x465, 0x313, 0x018, 0x06F, 0x461, 0x318, 0x018,
    0x06D, 0x45D, 0x31D, 0x019, 0x06B, 0x459, 0x322, 0x01A,
    0x06A, 0x455, 0x326, 0x01B, 0x068, 0x452, 0x32B, 0x01B,
    0x066, 0x44E, 0x330, 0x01C, 0x064, 0x44A, 0x335, 0x01D,
    0x063, 0x446, 0x33A, 0x01D, 0x061, 0x442, 0x33F, 0x01E,
    0x05F, 0x43E, 0x344, 0x01F, 0x05E, 0x43A, 0x349, 0x020,
    0x05C, 0x436, 0x34E, 0x020, 0x05A, 0x432, 0x353, 0x021,
    0x059, 0x42E, 0x357, 0x022, 0x057, 0x42A, 0x35C, 0x023,
    0x056, 0x425, 0x361, 0x024, 0x054, 0x421, 0x366, 0x024,
    0x053, 0x41D, 0x36B, 0x025, 0x051, 0x419, 0x370, 0x026,
    0x050, 0x415, 0x374, 0x027, 0x04E, 0x410, 0x379, 0x028,
    0x04D, 0x40C, 0x37E, 0x029, 0x04C, 0x408, 0x383, 0x02A,
    0x04A, 0x403, 0x388, 0x02B, 0x049, 0x3FF, 0x38C, 0x02C,
    0x047, 0x3FB, 0x391, 0x02D, 0x046, 0x3F6, 0x396, 0x02E,
    0x045, 0x3F2, 0x39B, 0x02F, 0x043, 0x3ED, 0x39F, 0x030,
    0x042, 0x3E9, 0x3A4, 0x031, 0x041, 0x3E5, 0x3A9, 0x032,
    0x040, 0x3E0, 0x3AD, 0x033, 0x03E, 0x3DC, 0x3B2, 0x034,
    0x03D, 0x3D7, 0x3B7, 0x035, 0x03C, 0x3D2, 0x3BB, 0x036,
    0x03B, 0x3CE, 0x3C0, 0x037, 0x03A, 0x3C9, 0x3C5, 0x038,
    0x038, 0x3C5, 0x3C9, 0x03A, 0x037, 0x3C0, 0x3CE, 0x03B,
    0x036, 0x3BB, 0x3D2, 0x03C, 0x035, 0x3B7, 0x3D7, 0x03D,
    0x034, 0x3B2, 0x3DC, 0x03E, 0x033, 0x3AD, 0x3E0, 0x040,
    0x032, 0x3A9, 0x3E5, 0x041, 0x031, 0x3A4, 0x3E9, 0x042,
    0x030, 0x39F, 0x3ED, 0x043, 0x02F, 0x39B, 0x3F2, 0x045,
    0x02E, 0x396, 0x3F6, 0x046, 0x02D, 0x391, 0x3FB, 0x047,
    0x02C, 0x38C, 0x3FF, 0x049, 0x02B, 0x388, 0x403, 0x04A,
    0x02A, 0x383, 0x408, 0x04C, 0x029, 0x37E, 0x40C, 0x04D,
    0x028, 0x379, 0x410, 0x04E, 0x027, 0x374, 0x415, 0x050,
    0x026, 0x370, 0x419, 0x051, 0x025, 0x36B, 0x41D, 0x053,
    0x024, 0x366, 0x421, 0x054, 0x024, 0x361, 0x425, 0x056,
    0x023, 0x35C, 0x42A, 0x057, 0x022, 0x357, 0x42E, 0x059,
    0x021, 0x353, 0x432, 0x05A, 0x020, 0x34E, 0x436, 0x05C,
    0x020, 0x349, 0x43A, 0x05E, 0x01F, 0x344, 0x43E, 0x05F,
    0x01E, 0x33F, 0x442, 0x061, 0x01D, 0x33A, 0x446, 0x063,
    0x01D, 0x335, 0x44A, 0x064, 0x01C, 0x330, 0x44E, 0x066,
    0x01B, 0x32B, 0x452, 0x068, 0x01B, 0x326, 0x455, 0x06A,
    0x01A, 0x322, 0x459, 0x06B, 0x019, 0x31D, 0x45D, 0x06D,
    0x018, 0x318, 0x461, 0x06F, 0x018, 0x313, 0x465, 0x071,
    0x017, 0x30E, 0x468, 0x073, 0x017, 0x309, 0x46C, 0x075,
    0x016, 0x304, 0x470, 0x076, 0x015, 0x2FF, 0x473, 0x078,
    0x015, 0x2FA, 0x477, 0x07A, 0x014, 0x2F5, 0x47A, 0x07C,
    0x014, 0x2F0, 0x47E, 0x07E, 0x013, 0x2EB, 0x481, 0x080,
    0x013, 0x2E6, 0x485, 0x082, 0x012, 0x2E1, 0x488, 0x084,
    0x011, 0x2DC, 0x48C, 0x086, 0x011, 0x2D8, 0x48F, 0x089,
    0x010, 0x2D3, 0x492, 0x08B, 0x010, 0x2CE, 0x496, 0x08D,
    0x00F, 0x2C9, 0x499, 0x08F, 0x00F, 0x2C4, 0x49C, 0x091,
    0x00F, 0x2BF, 0x49F, 0x093, 0x00E, 0x2BA, 0x4A2, 0x096,
    0x00E, 0x2B5, 0x4A6, 0x098, 0x00D, 0x2B0, 0x4A9, 0x09A,
    0x00D, 0x2AB, 0x4AC, 0x09C, 0x00C, 0x2A6, 0x4AF, 0x09F,
    0x00C, 0x2A2, 0x4B2, 0x0A1, 0x00B, 0x29D, 0x4B5, 0x0A3,
    0x00B, 0x298, 0x4B7, 0x0A6, 0x00B, 0x293, 0x4BA, 0x0A8,
    0x00A, 0x28E, 0x4BD, 0x0AB, 0x00A, 0x289, 0x4C0, 0x0AD,
    0x00A, 0x284, 0x4C3, 0x0AF, 0x009, 0x280, 0x4C5, 0x0B2,
    0x009, 0x27B, 0x4C8, 0x0B4, 0x009, 0x276, 0x4CB, 0x0B7,
    0x008, 0x271, 0x4CD, 0x0BA, 0x008, 0x26C, 0x4D0, 0x0BC,
    0x008, 0x267, 0x4D2, 0x0BF, 0x007, 0x263, 0x4D5, 0x0C1,
    0x007, 0x25E, 0x4D7, 0x0C4, 0x007, 0x259, 0x4D9, 0x0C7,
    0x006, 0x254, 0x4DC, 0x0C9, 0x006, 0x250, 0x4DE, 0x0CC,
    0x006, 0x24B, 0x4E0, 0x0CF, 0x006, 0x246, 0x4E3, 0x0D2,
    0x005, 0x241, 0x4E5, 0x0D4, 0x005, 0x23D, 0x4E7, 0x0D7,
    0x005, 0x238, 0x4E9, 0x0DA, 0x005, 0x233, 0x4EB, 0x0DD,
    0x004, 0x22F, 0x4ED, 0x0E0, 0x004, 0x22A, 0x4EF, 0x0E3,
    0x004, 0x226, 0x4F1, 0x0E6, 0x004, 0x221, 0x4F3, 0x0E9,
    0x004, 0x21C, 0x4F5, 0x0EC, 0x003, 0x218, 0x4F6, 0x0EF,
    0x003, 0x213, 0x4F8, 0x0F2, 0x003, 0x20F, 0x4FA, 0x0F5,
    0x003, 0x20A, 0x4FB, 0x0F8, 0x003, 0x205, 0x4FD, 0x0FB,
    0x002, 0x201, 0x4FF, 0x0FE, 0x002, 0x1FC, 0x500, 0x101,
    0x002, 0x1F8, 0x502, 0x104, 0x002, 0x1F3, 0x503, 0x107,
    0x002, 0x1EF, 0x504, 0x10B, 0x002, 0x1EB, 0x506, 0x10E,
    0x002, 0x1E6, 0x507, 0x111, 0x001, 0x1E2, 0x508, 0x114,
    0x001, 0x1DD, 0x50A, 0x118, 0x001, 0x1D9, 0x50B, 0x11B,
    0x001, 0x1D5, 0x50C, 0x11E, 0x001, 0x1D0, 0x50D, 0x122,
    0x001, 0x1CC, 0x50E, 0x125, 0x001, 0x1C8, 0x50F, 0x129,
    0x001, 0x1C3, 0x510, 0x12C, 0x001, 0x1BF, 0x511, 0x130,
    0x001, 0x1BB, 0x511, 0x133, 0x001, 0x1B7, 0x512, 0x137,
    0x000, 0x1B2, 0x513, 0x13A, 0x000, 0x1AE, 0x514, 0x13E,
    0x000, 0x1AA, 0x514, 0x141, 0x000, 0x1A6, 0x515, 0x145,
    0x000, 0x1A2, 0x516, 0x148, 0x000, 0x19E, 0x516, 0x14C,
    0x000, 0x19A, 0x517, 0x150, 0x000, 0x195, 0x517, 0x153,
    0x000, 0x191, 0x517, 0x157, 0x000, 0x18D, 0x518, 0x15B,
    0x000, 0x189, 0x518, 0x15F, 0x000, 0x185, 0x518, 0x162,
    0x000, 0x181, 0x518, 0x166, 0x000, 0x17D, 0x518, 0x16A,
    0x000, 0x17A, 0x519, 0x16E, 0x000, 0x176, 0x519, 0x172
};

/*
 * Noise table used by the SPU.  Verified on a real PS1.
 *
 * Noise is created using a very simple PRNG:
 *     u16 bias = bias + bias + noisetable[(bias >> 10) & 63]
 *     u16 freq = 0x8000 >> ((spu->spuCtrl & 0x3f00) >> 2)
 */
static const int noisetable[] = {
    1, 0, 0, 1, 0, 1, 1, 0,
    1, 0, 0, 1, 0, 1, 1, 0,
    1, 0, 0, 1, 0, 1, 1, 0,
    1, 0, 0, 1, 0, 1, 1, 0,
    0, 1, 1, 0, 1, 0, 0, 1,
    0, 1, 1, 0, 1, 0, 0, 1,
    0, 1, 1, 0, 1, 0, 0, 1,
    0, 1, 1, 0, 1, 0, 0, 1
};

////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
// START SOUND... called by main thread to setup a new sound on a channel
////////////////////////////////////////////////////////////////////////

static INLINE void StartSound(int ch)
{
    StartADSR(spu, ch);

    spu->s_chan[ch].pCurr = spu->s_chan[ch].pStart;	// set sample start

    spu->s_chan[ch].s_1 = 0;		// init mixing vars
    spu->s_chan[ch].s_2 = 0;
    spu->s_chan[ch].iSBPos = 28;

    spu->s_chan[ch].bNew = 0;	// init channel flags
    spu->s_chan[ch].bStop = 0;
    spu->s_chan[ch].bOn = 1;

    spu->s_chan[ch].SB[29] = 0;	// init our interpolation helpers
    spu->s_chan[ch].SB[30] = 0;

    spu->s_chan[ch].spos = 0x40000L;
    spu->s_chan[ch].SB[28] = 0;	// -> start with more decoding
}

////////////////////////////////////////////////////////////////////////
// MAIN SPU FUNCTION
// here is the main job handler... thread, timer or direct func call
// basically the whole sound processing is done in this fat func!
////////////////////////////////////////////////////////////////////////

// Counting to 65536 results in full volume offage.
void upse_ps1_spu_setlength(s32 stop, s32 fade)
{
    _ENTER;

    _DEBUG("stop [%d] fade [%d]", stop, fade);

    if (stop == 0)
    {
	spu->decaybegin = 0;
        _LEAVE;
    }
    else
    {
	stop = (stop * 441) / 10;
	fade = (fade * 441) / 10;

	spu->decaybegin = stop;
	spu->decayend = stop + fade;
    }

    _LEAVE;
}

void upse_ps1_spu_setvolume(int volume)
{
    _ENTER;

    if (volume >= 32)
        iVolume = 0;
    else
        iVolume = (volume / 4);

    _DEBUG("new volume [%d] [%d]", iVolume, volume);

    _LEAVE;
}

int upse_seek(u32 t)
{
    spu->seektime = t * 441 / 10;
    if (spu->seektime > spu->sampcount)
	return (1);
    return (0);
}

#define CLIP(_x) {if(_x>32767) _x=32767; if(_x<-32767) _x=-32767;}
int upse_ps1_spu_render(u32 cycles)
{
    s32 dosampies;
    s32 temp;

    spu->nextirq += cycles;
    dosampies = spu->nextirq / 384;
    if (!dosampies)
	return (1);
    spu->nextirq -= dosampies * 384;
    temp = dosampies;

    while (temp)
    {
	s32 revLeft = 0, revRight = 0;
	s32 sl = 0, sr = 0;
	int ch, fa;

	temp--;
	//--------------------------------------------------//
	//- main channel loop                              -// 
	//--------------------------------------------------//
	{
	    for (ch = 0; ch < MAXCHAN; ch++)	// loop em all.
	    {
		if (spu->s_chan[ch].bNew)
		    StartSound(ch);	// start new sound
		if (!spu->s_chan[ch].bOn)
		    continue;	// channel not playing? next


		if (spu->s_chan[ch].iActFreq != spu->s_chan[ch].iUsedFreq)	// new psx frequency?
		{
		    spu->s_chan[ch].iUsedFreq = spu->s_chan[ch].iActFreq;	// -> take it and calc steps
		    spu->s_chan[ch].sinc = spu->s_chan[ch].iRawPitch << 4;
		    if (!spu->s_chan[ch].sinc)
			spu->s_chan[ch].sinc = 1;
		}

		while (spu->s_chan[ch].spos >= 0x10000L)
		{
		    if (spu->s_chan[ch].iSBPos == 28)	// 28 reached?
		    {
			int predict_nr, shift_factor, flags, d, s;
			u8 *start;
			unsigned int nSample;
			int s_1, s_2;

			start = spu->s_chan[ch].pCurr;	// set up the current pos

			if (start == (u8 *) - 1)	// special "stop" sign
			{
			    spu->s_chan[ch].bOn = 0;	// -> turn everything off
			    spu->s_chan[ch].ADSRX.lVolume = 0;
			    spu->s_chan[ch].ADSRX.EnvelopeVol = 0;
			    goto ENDX;	// -> and done for this channel
			}

			spu->s_chan[ch].iSBPos = 0;	// Reset buffer play index.

			//////////////////////////////////////////// spu irq handler here? mmm... do it later

			s_1 = spu->s_chan[ch].s_1;
			s_2 = spu->s_chan[ch].s_2;

			predict_nr = (int) *start;
			start++;
			shift_factor = predict_nr & 0xf;
			predict_nr >>= 4;
			flags = (int) *start;
			start++;

			// -------------------------------------- // 
			// Decode new samples into spu->s_chan[ch].SB[0 through 27]
			for (nSample = 0; nSample < 28; start++)
			{
			    d = (int) *start;
			    s = ((d & 0xf) << 12);
			    if (s & 0x8000)
				s |= 0xffff0000;

			    fa = (s >> shift_factor);
			    fa = fa + ((s_1 * f[predict_nr][0]) >> 6) + ((s_2 * f[predict_nr][1]) >> 6);
			    s_2 = s_1;
			    s_1 = fa;
			    s = ((d & 0xf0) << 8);

			    spu->s_chan[ch].SB[nSample++] = fa;

			    if (s & 0x8000)
				s |= 0xffff0000;
			    fa = (s >> shift_factor);
			    fa = fa + ((s_1 * f[predict_nr][0]) >> 6) + ((s_2 * f[predict_nr][1]) >> 6);
			    s_2 = s_1;
			    s_1 = fa;

			    spu->s_chan[ch].SB[nSample++] = fa;
			}

			//////////////////////////////////////////// irq check

			if (spu->spuCtrl & 0x40)	// irq active?
			{
			    if ((spu->pSpuIrq > start - 16 &&	// irq address reached?
				 spu->pSpuIrq <= start) || ((flags & 1) &&	// special: irq on looping addr, when stop/loop flag is set 
						       (spu->pSpuIrq > spu->s_chan[ch].pLoop - 16 && spu->pSpuIrq <= spu->s_chan[ch].pLoop)))
			    {
				//extern s32 spuirqvoodoo;
				spu->s_chan[ch].iIrqDone = 1;	// -> debug flag
				SPUirq();
				//puts("IRQ");
				//if(spuirqvoodoo!=-1)
				//{
				// spuirqvoodoo=temp*384;
				// temp=0;
				//}
			    }
			}

			//////////////////////////////////////////// flag handler

			if ((flags & 4) && (!spu->s_chan[ch].bIgnoreLoop))
			    spu->s_chan[ch].pLoop = start - 16;	// loop adress

			if (flags & 1)	// 1: stop/loop
			{
			    // We play this block out first...
			    //if(!(flags&2))                          // 1+2: do loop... otherwise: stop
			    if (flags != 3 || spu->s_chan[ch].pLoop == NULL)	// PETE: if we don't check exactly for 3, loop hang ups will happen (DQ4, for example)
			    {	// and checking if pLoop is set avoids crashes, yeah
				start = (u8 *) - 1;
			    }
			    else
			    {
				start = spu->s_chan[ch].pLoop;
			    }
			}

			spu->s_chan[ch].pCurr = start;	// store values for next cycle
			spu->s_chan[ch].s_1 = s_1;
			spu->s_chan[ch].s_2 = s_2;

			////////////////////////////////////////////
		    }

		    fa = spu->s_chan[ch].SB[spu->s_chan[ch].iSBPos++];	// get sample data

		    if ((spu->spuCtrl & 0x4000) == 0)
			fa = 0;	// muted?
		    else
			CLIP(fa);

		    {
			int gpos;
			gpos = spu->s_chan[ch].SB[28];
			gval0 = fa;
			gpos = (gpos + 1) & 3;
			spu->s_chan[ch].SB[28] = gpos;
		    }
		    spu->s_chan[ch].spos -= 0x10000L;
		}

		if (spu->s_chan[ch].bNoise)
		{
		    _DEBUG("noise, ch:%d", ch);
		    fa = spu->s_chan[ch].iOldNoise;

		    spu->s_chan[ch].iNoisePos += (u16) (0x8000 >> ((spu->spuCtrl & 0x3f00) >> 2));
		    fa += (spu->s_chan[ch].iNoisePos + spu->s_chan[ch].iNoisePos + noisetable[(spu->s_chan[ch].iNoisePos >> 10) & 63]);

		    CLIP(fa);

		    spu->s_chan[ch].iOldNoise = fa;
		}		//----------------------------------------
		else		// NO NOISE (NORMAL SAMPLE DATA) HERE 
		{
		    int vl, vr, gpos;
		    vl = (spu->s_chan[ch].spos >> 6) & ~3;
		    gpos = spu->s_chan[ch].SB[28];

		    vr = ((gauss[vl] >> 2) * gval0) >> 5;
		    vr += ((gauss[vl + 1] >> 2) * gval(1)) >> 5;
		    vr += ((gauss[vl + 2] >> 2) * gval(2)) >> 5;
		    vr += ((gauss[vl + 3] >> 2) * gval(3)) >> 5;
		    fa = vr >> 4;
		}

		spu->s_chan[ch].sval = (MixADSR(spu, ch) * fa) >> 10;	// / 1023;  // add adsr
		if (spu->s_chan[ch].bFMod == 2)	// fmod freq channel
		{
		    int NP = spu->s_chan[ch + 1].iRawPitch;
		    NP = ((32768L + spu->s_chan[ch].sval) * NP) >> 15;	///32768L;

		    if (NP > 0x3fff)
			NP = 0x3fff;
		    if (NP < 0x1)
			NP = 0x1;

		    // mmmm... if I do this, all is screwed              
		    //           spu->s_chan[ch+1].iRawPitch=NP;

		    NP = (44100L * NP) / (4096L);	// calc frequency

		    spu->s_chan[ch + 1].iActFreq = NP;
		    spu->s_chan[ch + 1].iUsedFreq = NP;
		    spu->s_chan[ch + 1].sinc = (((NP / 10) << 16) / 4410);
		    if (!spu->s_chan[ch + 1].sinc)
			spu->s_chan[ch + 1].sinc = 1;

		    // mmmm... set up freq decoding positions?
		    //           spu->s_chan[ch+1].iSBPos=28;
		    //           spu->s_chan[ch+1].spos=0x10000L;
		}
		else
		{
		    //////////////////////////////////////////////
		    // ok, left/right sound volume (psx volume goes from 0 ... 0x3fff)
		    int tmpl, tmpr;

		    tmpl = (spu->s_chan[ch].sval * spu->s_chan[ch].iLeftVolume) >> 14;
		    tmpr = (spu->s_chan[ch].sval * spu->s_chan[ch].iRightVolume) >> 14;

		    sl += tmpl;
		    sr += tmpr;

		    if (((spu->rvb.Enabled >> ch) & 1) && (spu->spuCtrl & 0x80))
		    {
			revLeft += tmpl;
			revRight += tmpr;
		    }
		}

		spu->s_chan[ch].spos += spu->s_chan[ch].sinc;
	      ENDX:;
	    }
	}

	///////////////////////////////////////////////////////
	// mix all channels (including reverb) into one buffer
	MixREVERBLeftRight(spu, &sl, &sr, revLeft, revRight);
	if (spu->decaybegin != 0 && spu->sampcount >= spu->decaybegin)
	{
	    s32 dmul;
	    if (spu->decaybegin != 0)
	    {
		if (spu->sampcount >= spu->decayend)
		    return (0);
		dmul = 256 - (256 * (spu->sampcount - spu->decaybegin) / (spu->decayend - spu->decaybegin));
		sl = (sl * dmul) >> 8;
		sr = (sr * dmul) >> 8;
	    }
	}
	spu->sampcount++;

#if 0
	/* fix dynamic range. */
	sl >>= iVolume;
	sr >>= iVolume;
#endif

        CLIP(sl);
        CLIP(sr);

	*spu->pS++ = sl;
	*spu->pS++ = sr;
    }

    return (1);
}

void upse_ps1_stop(void)
{
    spu->decaybegin = 1;
    spu->decayend = 0;
}

void upse_set_audio_callback(upse_audio_callback_func_t func, void *user_data)
{
    _ENTER;

    spu->cb = func;
    spu->cb_userdata = user_data;

    _DEBUG("set audio callback function to <%p>", spu->cb);
    _DEBUG("set audio callback userdata to <%p>", spu->cb_userdata);

    _LEAVE;
}

void upse_ps1_spu_finalize(void)
{
    if ((spu->seektime != (u32) ~ 0) && spu->seektime > spu->sampcount)
    {
	spu->pS = (s16 *) spu->pSpuBuffer;

	if (spu->cb != NULL)
	    spu->cb(0, 0, spu->cb_userdata);
    }
    else if ((u8 *) spu->pS > ((u8 *) spu->pSpuBuffer + 1024))
    {
	if (spu->cb != NULL)
	    spu->cb((u8 *) spu->pSpuBuffer, (u8 *) spu->pS - (u8 *) spu->pSpuBuffer, spu->cb_userdata);

	spu->pS = (s16 *) spu->pSpuBuffer;
    }
}

int upse_ps1_spu_finalize_count(s16 ** s)
{
    if ((spu->seektime != (u32) ~ 0) && spu->seektime > spu->sampcount)
    {
        unsigned samples_skipped = ( (u8 *) spu->pS - (u8 *) spu->pSpuBuffer ) / 4;
        spu->pS = (s16 *) spu->pSpuBuffer;
        *s = NULL;
        return 1;
    }
    else if ((u8 *) spu->pS > ((u8 *) spu->pSpuBuffer + 1024))
    {
        unsigned samples_rendered = ( (u8 *) spu->pS - (u8 *) spu->pSpuBuffer ) / 4;
        spu->pS = (s16 *) spu->pSpuBuffer;
        *s = spu->pS;
        return samples_rendered;
    }

    return 0;
}

////////////////////////////////////////////////////////////////////////
// INIT/EXIT STUFF
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
// SPUINIT: this func will be called first by the main emu
////////////////////////////////////////////////////////////////////////

int upse_ps1_spu_init(void)
{
    return 0;
}

////////////////////////////////////////////////////////////////////////
// SETUPSTREAMS: init most of the spu buffers
////////////////////////////////////////////////////////////////////////

void SetupStreams(void)
{
    int i;

    spu->pSpuBuffer = (u8 *) malloc(32768);	// alloc mixing buffer
    spu->pS = (s16 *) spu->pSpuBuffer;

    for (i = 0; i < MAXCHAN; i++)	// loop sound channels
    {
	spu->s_chan[i].ADSRX.SustainLevel = 1024;	// -> init sustain
	spu->s_chan[i].iIrqDone = 0;
	spu->s_chan[i].pLoop = spu->spuMemC;
	spu->s_chan[i].pStart = spu->spuMemC;
	spu->s_chan[i].pCurr = spu->spuMemC;
    }
}

////////////////////////////////////////////////////////////////////////
// REMOVESTREAMS: free most buffer
////////////////////////////////////////////////////////////////////////

void RemoveStreams(void)
{
    free(spu->pSpuBuffer);		// free mixing buffer
    spu->pSpuBuffer = NULL;

#ifdef TIMEO
    {
	u64 tmp;
	tmp = SexyTime64();
	tmp -= begintime;
	if (tmp)
	    tmp = (u64) sampcount *1000000 / tmp;
	printf("%lld samples per second\n", tmp);
    }
#endif
}


////////////////////////////////////////////////////////////////////////
// SPUOPEN: called by main emu after init
////////////////////////////////////////////////////////////////////////

int upse_ps1_spu_open(void)
{
    spu = calloc(sizeof(upse_spu_state_t), 1);

    spu->spuMemC = (u8 *) spu->spuMem;	// just small setup
    memset((void *) spu->s_chan, 0, MAXCHAN * sizeof(SPUCHAN));
    memset((void *) &spu->rvb, 0, sizeof(REVERBInfo));
    memset(spu->regArea, 0, sizeof(spu->regArea));
    memset(spu->spuMem, 0, sizeof(spu->spuMem));
    InitADSR();
    spu->sampcount = spu->nextirq = 0;
    spu->seektime = (u32) ~ 0;

    if (spu->bSPUIsOpen)
	return 0;		// security for some stupid main emus
    spu->spuIrq = 0;

    spu->spuStat = spu->spuCtrl = 0;
    spu->spuAddr = 0xffffffff;

    spu->spuMemC = (u8 *) spu->spuMem;
    memset((void *) spu->s_chan, 0, (MAXCHAN + 1) * sizeof(SPUCHAN));
    spu->pSpuIrq = 0;

    iVolume = 0;		// full volume (0dB), volume past this point is seen as a pad, where 8 = -64dB
    SetupStreams();		// prepare streaming

    spu->bSPUIsOpen = 1;

    return 1;
}

////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
// SPUCLOSE: called before shutdown
////////////////////////////////////////////////////////////////////////

int upse_ps1_spu_close(void)
{
    if (spu == NULL)
	return 0;		// some security

    RemoveStreams();		// no more streaming

    free(spu);

    return 0;
}

////////////////////////////////////////////////////////////////////////
// SPUSHUTDOWN: called by main emu on final exit
////////////////////////////////////////////////////////////////////////

int SPUshutdown(void)
{
    return 0;
}

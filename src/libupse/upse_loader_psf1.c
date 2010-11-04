/*
 * UPSE: the unix playstation sound emulator.
 *
 * Filename: upse_loader_psf1.c
 * Purpose: libupse: PSF/MiniPSF loader.
 *
 * Copyright (c) 2008 William Pitcock <nenolod@dereferenced.org>
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
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include "upse-internal.h"
#include "upse.h"
#include "upse-string.h"

// LOAD STUFF

#ifdef WIN32_MSC
#pragma pack(4)
#endif

typedef struct
{
    unsigned char id[8];
    u32 text;
    u32 data;
    u32 pc0;
    u32 gp0;
    u32 t_addr;
    u32 t_size;
    u32 d_addr;
    u32 d_size;
    u32 b_addr;
    u32 b_size;
    u32 s_addr;
    u32 s_size;
    u32 SavedSP;
    u32 SavedFP;
    u32 SavedGP;
    u32 SavedRA;
    u32 SavedS0;
} upse_packed_t upse_exe_header_t;

static char *_upse_resolve_path(const char *f, const char *newfile)
{
    static char *ret;
    char *tp1;

#if PSS_STYLE==1
    tp1 = ((char *) strrchr(f, '/'));
#else
    tp1 = ((char *) strrchr(f, '\\'));
#if PSS_STYLE!=3
    {
	char *tp3;

	tp3 = ((char *) strrchr(f, '/'));
	if (tp1 < tp3)
	    tp1 = tp3;
        tp3 = ((char *) strrchr(f, '|'));
        if (tp1 < tp3)
	    tp1 = tp3;
    }
#endif
#endif
    if (!tp1)
    {
	ret = malloc(strlen(newfile) + 1);
	strcpy(ret, newfile);
    }
    else
    {
	ret = malloc(tp1 - f + 2 + strlen(newfile));	// 1(NULL), 1(/).
	memcpy(ret, f, tp1 - f + 1);
	ret[tp1 - f + 1] = 0;
	strcat(ret, newfile);
    }
    return (ret);
}

// Type==1 for just info load.
static upse_psf_t *_upse_load_psf(void *fp, const char *path, int level, int type, upse_iofuncs_t *funcs);

static upse_psf_t *
_upse_load_psf_from_file(const char *path, int level, int type, upse_iofuncs_t *funcs)
{
    void *fp;
    upse_psf_t *ret;

    if (!(fp = funcs->open_impl(path, "rb")))
    {
	_ERROR("path %s failed to load\n", path);
	return NULL;
    }

    ret = _upse_load_psf(fp, path, level, type, funcs);
    funcs->close_impl(fp);

    return ret;
}

static upse_psf_t *
_upse_load_psf(void *fp, const char *path, int level, int type, upse_iofuncs_t *funcs)
{
    upse_exe_header_t tmpHead;
    unsigned char *in, *out = 0;
    uLongf outlen;
    upse_psf_t *psfi;
    upse_xsf_t *xsf;
    u32 inlen;

    _ENTER;

    in = upse_get_buffer(fp, funcs, &inlen);
    xsf = upse_xsf_decode(in, inlen, &out, &outlen);

    memcpy(&tmpHead, out, sizeof(upse_exe_header_t));

    psfi = calloc(sizeof(upse_psf_t), 1);
    psfi->xsf = xsf;
    psfi->volume = upse_strtof(xsf->inf_volume) * 32;
    psfi->fade = upse_time_to_ms(xsf->inf_fade);
    psfi->stop = upse_time_to_ms(xsf->inf_length);
    psfi->title = xsf->inf_title;
    psfi->artist = xsf->inf_artist;
    psfi->copyright = xsf->inf_copy;
    psfi->game = xsf->inf_game;
    psfi->year = xsf->inf_year;

    /* if querying information, we do not want to upset the emulator. */
    if (!type)
    {
        upse_r3000_cpu_regs.pc = BFLIP32(tmpHead.pc0);
        upse_r3000_cpu_regs.GPR.n.gp = BFLIP32(tmpHead.gp0);
        upse_r3000_cpu_regs.GPR.n.sp = BFLIP32(tmpHead.s_addr);
        if (upse_r3000_cpu_regs.GPR.n.sp == 0)
            upse_r3000_cpu_regs.GPR.n.sp = 0x801fff00; /* first executable block in memory */
    }

    /* we are loading a psflib */
    if (level && !type)
    {
        LoadPSXMem(BFLIP32(tmpHead.t_addr), BFLIP32(tmpHead.t_size), out + 0x800);
        free(in);
        free(out);

        return psfi;
    }

    if (!type && *xsf->lib != '\0')
    {
        upse_psf_t *tmpi;

        if (*xsf->lib != '\0')
        {
            char *tmpfn;

            tmpfn = _upse_resolve_path(path, xsf->lib);
    	    tmpi = _upse_load_psf_from_file(tmpfn, level + 1, 0, funcs);

            free(tmpfn);
            upse_free_psf_metadata(tmpi);
        }
    }				// if(!type)

    if (!level && !type)
    {
        upse_psf_t *tmpi;
        char *lib;
        int i;
        u32 ba[3]; /* table holding base addresses for restore after loading aux libs */

        LoadPSXMem(BFLIP32(tmpHead.t_addr), BFLIP32(tmpHead.t_size), out + 0x800);
        free(in);
        free(out);

        for (lib = xsf->libaux[0], i = 1; *lib != '\0'; lib = xsf->libaux[i], i++)
        {
            char *tmpfn;

            ba[0] = upse_r3000_cpu_regs.pc;
            ba[1] = upse_r3000_cpu_regs.GPR.n.gp;
            ba[2] = upse_r3000_cpu_regs.GPR.n.sp;

            tmpfn = _upse_resolve_path(path, lib);
    	    tmpi = _upse_load_psf_from_file(tmpfn, level + 1, 0, funcs);
            if (tmpi == NULL)
                continue;

            free(tmpfn);
            upse_free_psf_metadata(tmpi);

            upse_r3000_cpu_regs.pc = ba[0];
            upse_r3000_cpu_regs.GPR.n.gp = ba[1];
            upse_r3000_cpu_regs.GPR.n.sp = ba[2];
        }
    }

    _LEAVE;
    return psfi;
}

void upse_free_psf_metadata(upse_psf_t * info)
{
    if (info->xsf)
        free(info->xsf);

    free(info);
}

upse_psf_t *
upse_get_psf_metadata(const char *path, upse_iofuncs_t * iofuncs)
{
    upse_psf_t *ret;

    _ENTER;

    if (!(ret = _upse_load_psf_from_file(path, 0, 1, iofuncs)))
	return NULL;

    if (ret->stop == (u32) ~ 0)
	ret->fade = 0;

    ret->length = ret->stop + ret->fade;

    _LEAVE;
    return ret;
}

upse_module_t *
upse_load_psf(void *fp, const char *path, upse_iofuncs_t * iofuncs)
{
    upse_psf_t *psf;
    upse_module_t *ret;

    _ENTER;

    psxInit();
    psxReset();

    upse_ps1_spu_init();
    upse_ps1_spu_open();

    if (!(psf = _upse_load_psf(fp, path, 0, 0, iofuncs)))
    {
	psxShutdown();
	return NULL;
    }

    if (psf->stop == (u32) ~ 0)
	psf->fade = 0;

    upse_ps1_spu_setvolume(psf->volume);
    upse_ps1_spu_setlength(psf->stop, psf->fade);
    psf->length = psf->stop + psf->fade;
    psf->rate = 44100;

    /*
     * Several rips by CaitSith2 have an invalid jump to the branch delay slot
     * before the BNE instruction, which causes the branching unit to deadlock
     * when we're cycle-accurate or running on the real deal.  This patch has
     * existed for some time in PlayPSF, actually, but since PlayPSF has been
     * separated from UPSE in UPSE2, we have to patch it here in the loader.
     *
     * Before, we would just stuff the PSF into memory after chainloading
     * PlayPSF.exe.
     *
     *     -- nenolod.
     */
    _DEBUG("checking for CaitSith2's JNE-delay-slot bug in this rip");
    if (PSXMu32(0xbc090) == BFLIP32(0x0802f040)) {
       _DEBUG("...fixing. :(");
       PSXMu32(0xbc090) = BFLIP32(0);
       PSXMu32(0xbc094) = BFLIP32(0x0802f040);
       PSXMu32(0xbc098) = BFLIP32(0);
    }

    _DEBUG("applying bugfixes for naughtydog replayers...");
    if (PSXMu32(0x118b8) == BFLIP32(0x1060fffd)) {
       _DEBUG("naughtydog patch applied.");
       PSXMu32(0x118b8) = BFLIP32(0);
    }

    ret = (upse_module_t *) calloc(sizeof(upse_module_t), 1);
    ret->metadata = psf;
    ret->evloop_run = upse_r3000_cpu_execute;
    ret->evloop_stop = upse_ps1_stop;
    ret->evloop_render = upse_r3000_cpu_execute_render;

    _LEAVE;
    return ret;
}

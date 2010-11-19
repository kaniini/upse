/*
 * UPSE: the unix playstation sound emulator.
 *
 * Filename: upse-module.h
 * Purpose: libupse: Module loading and probing.
 *
 * Copyright (c) 2008, 2010 William Pitcock <nenolod@dereferenced.org>
 *
 * UPSE is free software, released under the GNU General Public License,
 * version 2.
 *
 * A copy of the GNU General Public License, version 2, is included in
 * the UPSE source kit as COPYING.
 *
 * UPSE is offered without any warranty of any kind, explicit or implicit.
 */

#ifndef __UPSE__LIBUPSE__UPSE_MODULE_H__GUARD
#define __UPSE__LIBUPSE__UPSE_MODULE_H__GUARD

typedef struct {
    void *spu;
    void *ctrstate;
} upse_module_instance_t;

typedef void (*upse_eventloop_func_t)(upse_module_instance_t *ins);
typedef int (*upse_eventloop_render_func_t)(upse_module_instance_t *ins, s16 **samples);
typedef void (*upse_eventloop_setcb_func_t)(upse_module_instance_t *ins, upse_audio_callback_func_t func, void *user_data);
typedef int (*upse_eventloop_seek_func_t)(upse_module_instance_t *ins, u32 t);

typedef struct {
    void *opaque;
    upse_psf_t *metadata; /* XXX */
    upse_eventloop_func_t evloop_run;
    upse_eventloop_func_t evloop_stop;
    upse_eventloop_render_func_t evloop_render;
    upse_eventloop_setcb_func_t evloop_setcb;
    upse_eventloop_seek_func_t evloop_seek;
    upse_module_instance_t instance;
} upse_module_t;

typedef upse_module_t *(*upse_loader_func_t)(void *fileptr, const char *path, upse_iofuncs_t *iofuncs);

upse_loader_func_t upse_module_probe(void *fileptr, upse_iofuncs_t *funcs);
int upse_module_is_supported(void *fileptr, upse_iofuncs_t *funcs);
int upse_file_is_supported(char *file, upse_iofuncs_t *funcs);
upse_module_t *upse_module_open(const char *file, upse_iofuncs_t *funcs);
void upse_module_close(upse_module_t *mod);
void upse_module_init(void);

#endif

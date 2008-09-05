/*
 * UPSE: the unix playstation sound emulator.
 *
 * Filename: upse-module.h
 * Purpose: libupse: Module loading and probing.
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

#ifndef __UPSE__LIBUPSE__UPSE_MODULE_H__GUARD
#define __UPSE__LIBUPSE__UPSE_MODULE_H__GUARD

upse_loader_func_t upse_module_probe(void *fileptr, upse_iofuncs_t *funcs);
int upse_module_is_supported(void *fileptr, upse_iofuncs_t *funcs);
int upse_file_is_supported(char *file, upse_iofuncs_t *funcs);

#endif

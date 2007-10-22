/*
 * UPSE: the unix playstation sound emulator.
 *
 * Filename: upse.h
 * Purpose: libupse development header
 *
 * Copyright (c) 2007 William Pitcock <nenolod@sacredspiral.co.uk>
 *
 * UPSE is free software, released under the GNU General Public License,
 * version 2.
 *
 * A copy of the GNU General Public License, version 2, is included in
 * the UPSE source kit as COPYING.
 *
 * UPSE is offered without any warranty of any kind, explicit or implicit.
 */

#ifndef __UPSE_LIBUPSE_UPSE_H_GUARD
#define __UPSE_LIBUPSE_UPSE_H_GUARD

#include <unistd.h>
#include <upse-types.h>

typedef struct _upse_psftag
{
	struct _upse_psftag *next;
	char *key;
	char *value;
} upse_psftag_t;

typedef struct
{
	u32 length;
	u32 stop;
	u32 fade;
	char *title;
	char *artist;
	char *game;
	char *year;
	char *genre;
	char *psfby;
	char *comment;
	char *copyright;
	upse_psftag_t *taglist;
} upse_psf_t;

int upse_seek(u32 t);
void upse_stop(void);
void upse_execute(void);

typedef struct
{
	void *(*open_impl) (char *path, char *mode);
	  size_t(*read_impl) (void *ptr, size_t size, size_t nmemb, void *file);
	int (*seek_impl) (void *file, long offset, int whence);
	int (*close_impl) (void *file);
} upse_iofuncs_t;

upse_psf_t *upse_load(char *path, upse_iofuncs_t * iofuncs);
upse_psf_t *upse_get_psf_metadata(char *path, upse_iofuncs_t * iofuncs);
void upse_free_psf_metadata(upse_psf_t * info);

typedef void (*upse_audio_callback_func_t) (unsigned char *, long);
void upse_set_audio_callback(upse_audio_callback_func_t func);

#endif

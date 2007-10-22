/*
 * UPSE: the unix playstation sound emulator.
 *
 * Filename: upse123_main.c
 * Purpose: libupse: UPSE123 (console player) main loop
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/mman.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <sys/soundcard.h>
#include <upse.h>

static void *
upse123_open_impl(char *path, char *mode)
{
    return fopen(path, mode);
}

static size_t
upse123_read_impl(void *ptr, size_t sz, size_t nmemb, void *file)
{
    FILE *f = (FILE *) ptr;

    return fread(f, sz, nmemb, file);
}

static int
upse123_seek_impl(void *ptr, long offset, int whence)
{
    FILE *f = (FILE *) ptr;

    return fseek(f, offset, whence);
}

static int
upse123_close_impl(void *ptr)
{
    FILE *f = (FILE *) ptr;

    return fclose(f);
}

static upse_iofuncs_t upse123_iofuncs = {
    upse123_open_impl,
    upse123_read_impl,
    upse123_seek_impl,
    upse123_close_impl
};

static int oss_audio_fd = -1;

void 
upse123_write_audio(unsigned char* data, long bytes)
{
    if (oss_audio_fd == -1)
        return;

    write(oss_audio_fd, data, bytes);
}

void
upse123_init_audio(void)
{
    int pspeed = 44100;
    int pstereo = 1;
    int format;
    int oss_speed, oss_stereo;

    oss_speed = pspeed;
    oss_stereo = pstereo;

    if ((oss_audio_fd = open("/dev/dsp1", O_WRONLY, 0)) == -1)
    {
        printf("Sound device not available!\n");
        exit(EXIT_FAILURE);
    }

    if (ioctl(oss_audio_fd, SNDCTL_DSP_RESET, 0) == -1)
    {
        printf("Sound reset failed\n");
        exit(EXIT_FAILURE);
    }

    format = AFMT_S16_LE;

    if (ioctl(oss_audio_fd, SNDCTL_DSP_SETFMT, &format) == -1)
    {
        printf("Sound format not supported!\n");
        exit(EXIT_FAILURE);
    }

    if (format != AFMT_S16_LE)
    {
        printf("Sound format not supported!\n");
        exit(EXIT_FAILURE);
    }

    if (ioctl(oss_audio_fd,SNDCTL_DSP_STEREO,&oss_stereo)==-1)
    {
        printf("Stereo mode not supported!\n");
        exit(EXIT_FAILURE);
    }

    if (ioctl(oss_audio_fd,SNDCTL_DSP_SPEED,&oss_speed)==-1)
    {
        printf("Sound frequency not supported\n");
        exit(EXIT_FAILURE);
    }

    if (oss_speed!=pspeed)
    {
        printf("Sound frequency not supported\n");
        exit(EXIT_FAILURE);
    }

    upse_set_audio_callback(upse123_write_audio);
}

void
upse123_close_audio(void)
{
    upse_set_audio_callback(NULL);

    if (oss_audio_fd > 0)
        close(oss_audio_fd);
}

void
upse123_print_field(char *field, char *data)
{
    printf("\033[00;36m%-20s\033[01;36m|\033[0m %s\n", field, data);
}

int
main(int argc, char *argv[])
{
    upse_psf_t *psf;
    int i;

    if (argc < 2)
    {
        printf("%s: usage: %s <psf file>\n", argv[0], argv[0]);
        return -1;
    }

    printf("\033[K\033[01;36mUPSE123\033[00;36m: High quality PSF player.\033[0m\n");
    printf("\033[K\033[00;36mCopyright (C) 2007 William Pitcock <nenolod@sacredspiral.co.uk>\033[0m\n");
    printf("\n\033[01mUPSE123 is free software; licensed under the GNU GPL version 2.\nAs such, NO WARRANTY IS PROVIDED. USE AT YOUR OWN RISK!\033[0m\n");

    upse123_init_audio();

    for (i = 1; i < argc; i++)
    {
        if ((psf = upse_load(argv[i], &upse123_iofuncs)) == NULL)
        {
            printf("%s: failed to load `%s'\n", argv[0], argv[1]);
            continue;
        }

        printf("\nInformation about \033[01m%s\033[00;0m:\n\n", argv[i]);

        upse123_print_field("Game", psf->game);
        upse123_print_field("Title", psf->title);
        upse123_print_field("Artist", psf->artist);
        upse123_print_field("Year", psf->year);
        upse123_print_field("Genre", psf->genre);
        upse123_print_field("Ripper", psf->psfby);
        upse123_print_field("Copyright", psf->copyright);

        upse_execute();
        upse_free_psf_metadata(psf);
    }

    upse123_close_audio();

    return 0;
}

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
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <sys/soundcard.h>
#include <upse.h>

#include "../../config.h"

#ifdef HAVE_AO
# include <ao/ao.h>
#endif

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

#ifndef HAVE_AO
static int oss_audio_fd = -1;
#else
static ao_device *ao_dev_ = NULL;
static ao_option *ao_opts_ = NULL;
#endif
static char *audio_dev_ = NULL;

static int decode_position = 0;
static upse_psf_t *psf;

#ifdef HAVE_AO
int
upse123_ao_add_option(ao_option **optv, const char *string)
{
    char *key, *value;
    int ret;

    if (string == NULL)
        return 0;

    key = strdup(string);
    if (key == NULL)
        return 0;

    value = strchr(key, ':');
    if (value == NULL)
    {
        free(key);
        return 0;
    }

    *value++ = '\0';
    ret = ao_append_option(optv, key, value);
    free(key);

    return ret;
}
#endif

void 
upse123_write_audio(unsigned char* data, long bytes, void *unused)
{
    int remaining;

#ifndef HAVE_AO
    if (oss_audio_fd == -1)
        return;

    write(oss_audio_fd, data, bytes);
#else
    if (ao_dev_ == NULL)
        return;

    ao_play(ao_dev_, (char *) data, bytes);
#endif

    decode_position += ((bytes / 4 * 1000) / 44100);
    remaining = psf->length - decode_position;

    printf("\033[00;36mTime:\033[0m %02d:%02d.%02d%s",
         (int)(decode_position / 1000.0) / 60,
         (int)(decode_position / 1000.0) % 60,
         (int)(decode_position / 10.0) % 100,
	 psf->length == -1 ? "\r" : "");

    if (psf->length != -1)
    {
        printf(" [-%02d:%02d.%02d]",
             (int)(remaining / 1000.0) / 60,
             (int)(remaining / 1000.0) % 60,
             (int)(remaining / 10.0) % 100);

        printf(" of %02d:%02d.%02d\r",
             (int)(psf->length / 1000.0) / 60,
             (int)(psf->length / 1000.0) % 60,
             (int)(psf->length / 10.0) % 100);
    }

    fflush(stdout);
}

void
upse123_init_audio(void)
{
#ifndef HAVE_AO
    int pspeed = 44100;
    int pstereo = 1;
    int format;
    int oss_speed, oss_stereo;

    oss_speed = pspeed;
    oss_stereo = pstereo;

    if ((oss_audio_fd = open(audio_dev_ != NULL ? audio_dev_ : "/dev/dsp", O_WRONLY, 0)) == -1)
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
#else
    ao_initialize();

    {
        ao_sample_format format_ = { 16, 44100, 2, AO_FMT_NATIVE };
        ao_dev_ = ao_open_live(audio_dev_ != NULL ? ao_driver_id(audio_dev_) : ao_default_driver_id(),
                               &format_, ao_opts_);
    }
#endif

    upse_set_audio_callback(upse123_write_audio, NULL);
}

void
upse123_close_audio(void)
{
    upse_set_audio_callback(NULL, NULL);

#ifndef HAVE_AO
    if (oss_audio_fd > 0)
        close(oss_audio_fd);

    oss_audio_fd = -1;
#else
    ao_close(ao_dev_);
    ao_dev_ = NULL;

    ao_free_options(ao_opts_);
    ao_opts_ = NULL;

    ao_shutdown();
#endif
}

void
upse123_print_field(char *field, char *data)
{
    if (data == NULL)
        return;

    printf("\033[00;36m%-20s\033[01;36m|\033[0m %s\n", field, data);
}

void
usage(const char *progname)
{
    printf("\nUsage: %s [options] files - plays PSF files\n\n", progname);
#ifdef HAVE_AO
    printf("  -o        Sets an audio device option.\n");
#endif
    printf("  -d        Sets the audio output device to use.\n");
#ifdef HAVE_AO
    printf("            Use a sound system name like 'oss', 'alsa' or 'esd' here.\n");
#else
    printf("            Use a path to your OSS /dev/dsp here. (default: /dev/dsp)\n");
#endif
    printf("  -s [secs] Pauses [secs] between songs.\n");
    printf("  -R        Disables reverb.\n");
    printf("  -q        Enables non-resampled reverb. [experimental]\n");
    printf("  -h        Displays this message.\n");
    printf("  -v        Displays upse123's version.\n");
    printf("  -B        Sets a custom BIOS to use.\n");
    printf("\nReport bugs to <" PACKAGE_BUGREPORT ">.\n");

    exit(EXIT_SUCCESS);
}

void
version(const char *progname)
{
    printf("%s " PACKAGE_VERSION "\n", progname);
}

int
main(int argc, char *argv[])
{
    int i;
    char r;
    int sleep_value_ = 0;

    while ((r = getopt(argc, argv, "hvo:d:s:RqB:")) >= 0)
    {
        switch(r) {
            case 'h':
                usage(argv[0]);
                return 0;
                break;
            case 'v':
                version(argv[0]);
                return 0;
                break;
#ifdef HAVE_AO
            case 'o':
                upse123_ao_add_option(&ao_opts_, optarg);
                break;
#endif
            case 'd':
                audio_dev_ = strdup(optarg);
                break;
            case 's':
                sleep_value_ = atoi(optarg);
                break;
            case 'R':
                upse_set_reverb_mode(0);
                break;
            case 'q':
                upse_set_reverb_no_downsample(1);
                break;
            case 'B':
                upse_set_custom_bios(optarg);
                break;
            default:
                break;
        }
    }

    printf("\033[K\033[01;36mUPSE123\033[00;36m: High quality PSF player.\033[0m\n");
    printf("\033[K\033[00;36mCopyright (C) 2007 William Pitcock <nenolod@sacredspiral.co.uk>\033[0m\n");
    printf("\n\033[01mUPSE123 is free software; licensed under the GNU GPL version 2.\nAs such, NO WARRANTY IS PROVIDED. USE AT YOUR OWN RISK!\033[0m\n");

    upse123_init_audio();

    if (argc - optind <= 0)
        usage(argv[0]);

    for (i = optind; i < argc; i++)
    {
        if ((psf = upse_load(argv[i], &upse123_iofuncs)) == NULL)
        {
            printf("%s: failed to load `%s'\n", argv[0], argv[1]);
            continue;
        }

        decode_position = 0;

        printf("\nInformation about \033[01m%s\033[00;0m:\n\n", argv[i]);

        upse123_print_field("Game", psf->game);
        upse123_print_field("Title", psf->title);
        upse123_print_field("Artist", psf->artist);
        upse123_print_field("Year", psf->year);
        upse123_print_field("Genre", psf->genre);
        upse123_print_field("Ripper", psf->psfby);
        upse123_print_field("Copyright", psf->copyright);

	printf("\n");

        upse_execute();
        upse_free_psf_metadata(psf);

        if (sleep_value_)
            sleep(sleep_value_);
    }

    upse123_close_audio();

    return 0;
}

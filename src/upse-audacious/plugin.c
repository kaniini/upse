/*
 * UPSE: the unix playstation emulator.
 *
 * Filename: plugin.c
 * Purpose: Glue to Audacious' 1.4 Component API.
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

#include <audacious/plugin.h>
#include <libaudcore/vfs.h>
#include <libaudcore/tuple.h>
#include <libaudgui/libaudgui.h>
#include <libaudgui/libaudgui-gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <upse.h>

#include "../../config.h"

static int seek = 0;

static Tuple *upse_aud_get_tuple_psf(const gchar *fn, upse_psf_t *psf);

static void *upse_aud_open_impl(const gchar *path, const gchar *mode) {
    return vfs_fopen(path, mode);
}

static size_t upse_aud_read_impl(void *ptr, size_t sz, size_t nmemb, void *file) {
    return vfs_fread(ptr, sz, nmemb, file);
}

static int upse_aud_seek_impl(void *ptr, long offset, int whence) {
    return vfs_fseek(ptr, offset, whence);
}

static int upse_aud_close_impl(void *ptr) {
    return vfs_fclose(ptr);
}

static long upse_aud_tell_impl(void *ptr) {
    return vfs_ftell(ptr);
}

static upse_iofuncs_t upse_aud_iofuncs = {
    upse_aud_open_impl,
    upse_aud_read_impl,
    upse_aud_seek_impl,
    upse_aud_close_impl,
    upse_aud_tell_impl
};

static int is_our_fd(const gchar *filename, VFSFile *file) {
    gchar magic[4];
    vfs_fread(magic, 1, 4, file);

    // only allow PSF1 for now
    if (!memcmp(magic, "PSF\x01", 4))
        return 1;
    return 0;
}

gboolean stop_flag = FALSE;
upse_module_t *playback_mod = NULL;

void upse_aud_update(unsigned char *buffer, long count, gconstpointer data)
{
    const InputPlayback *playback = data;

    if (stop_flag)
        upse_eventloop_stop(playback_mod);

    playback->output->write_audio(buffer, count);

    if (seek)
    {
        if(upse_eventloop_seek(playback_mod, seek))
        {
            playback->output->flush(seek);
            seek = 0;
        }
        else
        {
            upse_eventloop_stop(playback_mod);
            return;
        }
    }
}

static gboolean upse_aud_play(InputPlayback *playback, const gchar * filename, VFSFile * file, gint start_time, gint stop_time, gboolean pause)
{
    upse_psf_t *psf;
    static int initialized = 0;

    if (!initialized)
    {
        upse_module_init();
        initialized++;
    }

    if(!(playback_mod = upse_module_open(filename, &upse_aud_iofuncs)))
        return FALSE;

    psf = playback_mod->metadata;
    seek = start_time;

    playback->set_params(playback, psf->rate * 2 * 2 * 8, psf->rate, 2);

    if (!playback->output->open_audio(FMT_S16_NE, psf->rate, 2))
    {
        upse_module_close(playback_mod);
        return FALSE;
    }

    upse_eventloop_set_audio_callback(playback_mod, upse_aud_update, playback);

    stop_flag = FALSE;
    playback->set_pb_ready(playback);

    for (;;)
    {
        upse_eventloop_run(playback_mod);

        if (stop_flag)
            break;

        if (seek)
        {
            playback->output->flush(seek);

            upse_module_close(playback_mod);
            if(!(playback_mod = upse_module_open(filename, &upse_aud_iofuncs)))
                break;

            upse_eventloop_seek(playback_mod, seek); 
            seek = 0;
            continue;
        }

        break;
    }

    upse_module_close(playback_mod);

    while (!stop_flag && playback->output->buffer_playing())
        g_usleep(10000);

    playback->output->close_audio();
    stop_flag = FALSE;

    return TRUE;
}

static void upse_aud_stop(InputPlayback *playback)
{
    if (stop_flag)
        return;

    playback->output->pause(0);
    stop_flag = TRUE;
}

static void upse_aud_pause(InputPlayback *playback, gboolean p)
{
    playback->output->pause(p);
}

static void upse_aud_mseek(InputPlayback *playback, gint time)
{
    if (stop_flag)
        return;

    seek = time;
}

static Tuple *upse_aud_get_tuple_psf(const gchar *fn, upse_psf_t *psf) {
    Tuple *tuple = NULL;

    if (psf->length) {
        tuple = tuple_new_from_filename(fn);
        tuple_associate_int(tuple, FIELD_LENGTH, NULL, psf->length);
        tuple_associate_string(tuple, FIELD_ARTIST, NULL, psf->artist);
        tuple_associate_string(tuple, FIELD_ALBUM, NULL, psf->game);
        tuple_associate_string(tuple, -1, "game", psf->game);
        tuple_associate_string(tuple, FIELD_TITLE, NULL, psf->title);
        tuple_associate_string(tuple, FIELD_GENRE, NULL, psf->genre);
        tuple_associate_string(tuple, FIELD_COPYRIGHT, NULL, psf->copyright);
        tuple_associate_string(tuple, FIELD_QUALITY, NULL, "sequenced");
        tuple_associate_string(tuple, FIELD_CODEC, NULL, "PlayStation Audio");
        tuple_associate_string(tuple, -1, "console", "PlayStation");
        tuple_associate_string(tuple, -1, "dumper", psf->psfby);
        tuple_associate_string(tuple, FIELD_COMMENT, NULL, psf->comment);
    }

    return tuple;
}   

static Tuple *get_tuple_psf(const gchar *fn, VFSFile *fd) {
    upse_psf_t *psf = upse_get_psf_metadata(fn, &upse_aud_iofuncs);

    if (psf != NULL) {
        Tuple *ret = upse_aud_get_tuple_psf(fn, psf);
        upse_free_psf_metadata(psf);
        return ret;
    }

    return NULL;
}

static void upse_aud_about(void) {
#if 0
    audacious_info_dialog("About " PACKAGE_STRING,
                          PACKAGE " is a plugin and emulation library which produces near-bit-exact\n"
                          "output comparable to a PlayStation. It can be used to play chiptunes, and other\n"
                          "fun things.\n"
                          "\n"
                          "Find out more at http://carpathia.dereferenced.org/~nenolod/upse\n"
                          "\n"
                          PACKAGE_STRING " is licensed to you under the GNU General Public License, \n"
                          "version 2. A copy of this license is included in the " PACKAGE " source kit,\n"
                          "or on the internet at <http://www.gnu.org/licenses>.\n"
                          "\n"
                          "Talk about " PACKAGE " in it's very own forum! <http://boards.nenolod.net>\n"
                          "Report bugs in " PACKAGE " to <" PACKAGE_BUGREPORT ">.",
                          "Close",
                          FALSE,
                          G_CALLBACK(gtk_widget_destroy),
                          NULL);
#endif
}

const gchar *upse_fmts[] = { "psf", "minipsf", NULL };

InputPlugin upse_ip =
{
    .description = "UNIX Playstation Sound Emulator",
    .about = upse_aud_about,
    .play = upse_aud_play,
    .stop = upse_aud_stop,
    .pause = upse_aud_pause,
    .mseek = upse_aud_mseek,
    .probe_for_tuple = get_tuple_psf,
    .is_our_file_from_vfs = is_our_fd,
    .vfs_extensions = upse_fmts,
};

InputPlugin *upse_iplist[] = { &upse_ip, NULL };

SIMPLE_INPUT_PLUGIN(upse, upse_iplist);

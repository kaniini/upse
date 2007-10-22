/*
 * UPSE: the unix playstation emulator.
 *
 * Filename: plugin.c
 * Purpose: Glue to Audacious' 1.4 Component API.
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

#include <audacious/plugin.h>
#include <audacious/output.h>
#include <audacious/main.h>
#include <audacious/util.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <upse.h>

#include "../../config.h"

static volatile int seek = 0;

extern InputPlugin upse_ip;
static gboolean audio_error = FALSE;

static upse_psf_t *upse_psf = NULL;
static InputPlayback *playback = NULL;

static gchar *get_title_psf(gchar *fn);

static void *upse_aud_open_impl(gchar *path, gchar *mode) {
    return aud_vfs_fopen(path, mode);
}

static size_t upse_aud_read_impl(void *ptr, size_t sz, size_t nmemb, void *file) {
    return aud_vfs_fread(ptr, sz, nmemb, file);
}

static int upse_aud_seek_impl(void *ptr, long offset, int whence) {
    return aud_vfs_fseek(ptr, offset, whence);
}

static int upse_aud_close_impl(void *ptr) {
    return aud_vfs_fclose(ptr);
}

static upse_iofuncs_t upse_aud_iofuncs = {
    upse_aud_open_impl,
    upse_aud_read_impl,
    upse_aud_seek_impl,
    upse_aud_close_impl
};

static int is_our_fd(gchar *filename, VFSFile *file) {
    gchar magic[4];
    aud_vfs_fread(magic, 1, 4, file);

    // only allow PSF1 for now
    if (!memcmp(magic, "PSF\x01", 4))
        return 1;
    return 0;
}

void upse_aud_update(unsigned char *buffer, long count)
{
    const int mask = ~((((16 / 8) * 2)) - 1);

    while (count > 0)
    {
        int t = playback->output->buffer_free() & mask;
        if (t > count)
            playback->pass_audio(playback,
                          FMT_S16_NE, 2, count, buffer, &playback->playing);
        else
        {
            if (t)
                playback->pass_audio(playback, FMT_S16_NE, 2, t, buffer, &playback->playing);
        }
        count -= t;
        buffer += t;
    }
    if (seek)
    {
        if(upse_seek(seek))
        {
            playback->output->flush(seek);
            seek = 0;
        }
        else  // negative time - must make a C time machine
        {
            upse_stop();
            return;
        }
    }
    if (playback->playing == FALSE)
        upse_stop();
}

static gpointer upse_playloop(gpointer arg)
{
    while (TRUE)
    {
        upse_execute();

        /* we have reached the end of the song or a command was issued */

        playback->output->buffer_free();
        playback->output->buffer_free();

        if (playback->playing == FALSE)
            break;

        if (seek)
        {
            playback->output->flush(seek);
            if(!(upse_psf = upse_load(playback->filename, &upse_aud_iofuncs)))
                break;
            upse_seek(seek); 
            seek = 0;
            continue;
        }

        break;
    }

    playback->output->close_audio();
    playback->playing = FALSE;
    playback->eof = TRUE;
    return NULL;
}

static void upse_aud_play(InputPlayback *data)
{
    playback = data;

    upse_set_audio_callback(upse_aud_update);

    if (!playback->output->open_audio(FMT_S16_NE, 44100, 2))
    {
        audio_error = TRUE;
        return;
    }

    if(!(upse_psf = upse_load(data->filename, &upse_aud_iofuncs)))
    {
        playback->output->close_audio();
    }
    else
    {
        seek = 0;

        gchar *name = get_title_psf(data->filename);
        data->set_params(data, name, upse_psf->length, 44100*2*2*8, 44100, 2);
        g_free(name);

        playback->playing = TRUE;
        data->set_pb_ready(data);
        upse_playloop(NULL);
    }
}

static void upse_aud_stop(InputPlayback * playback)
{
    if (!playback->playing)
        return;

    playback->output->pause(0);
    playback->playing = FALSE;
    g_thread_join(playback->thread);

    upse_free_psf_metadata(upse_psf);
    upse_psf = NULL;
}

static void upse_aud_pause(InputPlayback *playback, short p)
{
    playback->output->pause(p);
}

static void upse_aud_seek(InputPlayback * data, int time)
{
    if (!playback->playing)
        return;

    seek = time * 1000;
}

static void upse_aud_getsonginfo(char *fn, char **title, int *length)
{
    upse_psf_t *tmp;

    if((tmp = upse_get_psf_metadata(fn, &upse_aud_iofuncs))) {
        *length = tmp->length;
        *title = get_title_psf(fn);
        upse_free_psf_metadata(tmp);
    }
}

static Tuple *get_aud_tuple_psf(gchar *fn) {
    Tuple *tuple = NULL;
    upse_psf_t *tmp = upse_get_psf_metadata(fn, &upse_aud_iofuncs);

    if (tmp->length) {
        tuple = aud_tuple_new_from_filename(fn);
	aud_tuple_associate_int(tuple, FIELD_LENGTH, NULL, tmp->length);
	aud_tuple_associate_string(tuple, FIELD_ARTIST, NULL, tmp->artist);
	aud_tuple_associate_string(tuple, FIELD_ALBUM, NULL, tmp->game);
	aud_tuple_associate_string(tuple, -1, "game", tmp->game);
        aud_tuple_associate_string(tuple, FIELD_TITLE, NULL, tmp->title);
        aud_tuple_associate_string(tuple, FIELD_GENRE, NULL, tmp->genre);
        aud_tuple_associate_string(tuple, FIELD_COPYRIGHT, NULL, tmp->copyright);
        aud_tuple_associate_string(tuple, FIELD_QUALITY, NULL, "sequenced");
        aud_tuple_associate_string(tuple, FIELD_CODEC, NULL, "PlayStation Audio");
        aud_tuple_associate_string(tuple, -1, "console", "PlayStation");
        aud_tuple_associate_string(tuple, -1, "dumper", tmp->psfby);
        aud_tuple_associate_string(tuple, FIELD_COMMENT, NULL, tmp->comment);

        upse_free_psf_metadata(tmp);
    }

    return tuple;
}   

static gchar *get_title_psf(gchar *fn) {
    gchar *title = NULL;
    Tuple *tuple = get_aud_tuple_psf(fn);

    if (tuple != NULL) {
        title = aud_tuple_formatter_make_title_string(tuple, aud_get_gentitle_format());
        aud_tuple_free(tuple);
    }
    else
        title = g_path_get_basename(fn);

    return title;
}

static void upse_aud_about(void) {
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
}

gchar *upse_fmts[] = { "psf", "minipsf", NULL };

InputPlugin upse_ip =
{
    .description = "UNIX Playstation Sound Emulator",
    .about = upse_aud_about,
    .play_file = upse_aud_play,
    .stop = upse_aud_stop,
    .pause = upse_aud_pause,
    .seek = upse_aud_seek,
    .get_song_info = upse_aud_getsonginfo,
    .get_song_tuple = get_aud_tuple_psf,
    .is_our_file_from_vfs = is_our_fd,
    .vfs_extensions = upse_fmts,
};

InputPlugin *upse_iplist[] = { &upse_ip, NULL };

SIMPLE_INPUT_PLUGIN(upse, upse_iplist);

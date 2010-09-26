/*
 * UPSE: the unix playstation sound emulator.
 *
 * Filename: psf2dump_main.c
 * Purpose: libupse: psf2dump utility.
 *
 * Copyright (c) 2009 William Pitcock <nenolod@dereferenced.org>
 *
 * UPSE is free software, released under the GNU General Public License,
 * version 2.
 *
 * A copy of the GNU General Public License, version 2, is included in
 * the UPSE source kit as COPYING.
 *
 * UPSE is offered without any warranty of any kind, explicit or implicit.
 */

#include <upse.h>
#include <upse-filesystem.h>

#include <sys/stat.h>
#include <sys/types.h>

#include <errno.h>
#include <strings.h>

static void *
psf2dump_open_impl(const char *path, const char *mode)
{
    return fopen(path, mode);
}

static size_t
psf2dump_read_impl(void *ptr, size_t sz, size_t nmemb, void *file)
{
    FILE *f = (FILE *) ptr;

    return fread(f, sz, nmemb, file);
}

static int
psf2dump_seek_impl(void *ptr, long offset, int whence)
{
    FILE *f = (FILE *) ptr;

    return fseek(f, offset, whence);
}

static int
psf2dump_close_impl(void *ptr)
{
    FILE *f = (FILE *) ptr;

    return fclose(f);
}

static long
psf2dump_tell_impl(void *ptr)
{
    FILE *f = (FILE *) ptr;

    return ftell(f);
}

static upse_iofuncs_t psf2dump_iofuncs = {
    psf2dump_open_impl,
    psf2dump_read_impl,
    psf2dump_seek_impl,
    psf2dump_close_impl,
    psf2dump_tell_impl
};

/* routine taken from dovecot, (c) 2003 timo sirainen */
int
mkdir_parents(const char *path, mode_t mode)
{
    const char *p;
    char *path2;

    if (mkdir(path, mode) < 0 && errno != EEXIST &&
        errno != EISDIR && errno != ENOSYS)
    {
        if (errno != ENOENT)
            return -1;

        p = strrchr(path, '/');
        if (p == NULL || p == path)
	    return -1; /* shouldn't happen */

        path2 = strndup(path, (p - path));
	if (mkdir_parents(path2, mode) < 0) {
            free(path2);
            return -1;
        }

        free(path2);

        /* should work now */
        if (mkdir(path, mode) < 0 && errno != EEXIST && errno != EISDIR)
            return -1;
    }

    return 0;
}

int
main(int argc, const char *argv[])
{
    upse_module_t *mod;
    upse_filesystem_t *fs;
    upse_filesystem_entry_t *iter;

    upse_module_init();
    mod = upse_module_open(argv[1], &psf2dump_iofuncs);
    fs = (upse_filesystem_t *) mod->opaque;

    for (iter = fs->head; iter != NULL; iter = iter->next)
    {
        char *dir, *dir_p;
        printf("x %s\n", &iter->filename[1]);
        FILE *f;

        dir_p = strrchr(&iter->filename[1], '/');
        if (dir_p != NULL)
        {
            dir = strndup(&iter->filename[1], (dir_p - iter->filename) - 1);
            mkdir_parents(dir, 0755);
            free(dir);
        }

        f = fopen(&iter->filename[1], "wb");
        fwrite(iter->data, iter->len, 1, f);
        fclose(f);
    }

    return 0;
}

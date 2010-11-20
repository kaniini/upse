/*
 * UPSE: the unix playstation sound emulator.
 *
 * Filename: reentrancy_test_main.c
 * Purpose: libupse: reentrancy test.
 *
 * Copyright (c) 2010 William Pitcock <nenolod@dereferenced.org>
 *
 * UPSE is free software, released under the GNU General Public License,
 * version 2.
 *
 * A copy of the GNU General Public License, version 2, is included in
 * the UPSE source kit as COPYING.
 *
 * UPSE is offered without any warranty of any kind, explicit or implicit.
 */

#include <pthread.h>
#include <upse.h>

static void *
open_impl(const char *path, const char *mode)
{
    return fopen(path, mode);
}

static size_t
read_impl(void *ptr, size_t sz, size_t nmemb, void *file)
{
    FILE *f = (FILE *) ptr;

    return fread(f, sz, nmemb, file);
}

static int 
seek_impl(void *ptr, long offset, int whence)
{
    FILE *f = (FILE *) ptr;

    return fseek(f, offset, whence);
}

static int
close_impl(void *ptr)
{
    FILE *f = (FILE *) ptr;

    return fclose(f);
}

static long
tell_impl(void *ptr)
{
    FILE *f = (FILE *) ptr;

    return ftell(f);
}

static upse_iofuncs_t iofuncs = {
    open_impl,
    read_impl,
    seek_impl,
    close_impl,
    tell_impl
};

void *
emulator_singleton_thread(void *arg)
{
    upse_module_t *mod;

    mod = upse_module_open(arg, &iofuncs);    
    if (mod == NULL)
    {
        printf("opening %s failed?\n", arg);
        return NULL;
    }

    printf("thread %p is playing mod: %s\n", pthread_self(), mod->metadata->title);
    upse_eventloop_run(mod);
    printf("thread %p has played mod: %s\n", pthread_self(), mod->metadata->title);

    upse_module_close(mod);

    fflush(stdout);

    return NULL;
}

int
main(int argc, const char *argv[])
{
    int y, i = 20;
    const char *file = "./test.psf";
    pthread_t *thbase;

    upse_module_init();

    if (argc > 1)
        i = atoi(argv[1]);

    if (argc > 2)
        file = argv[2];

    printf("Launching %d emulators.\n", i);

    thbase = calloc(sizeof(pthread_t), i);

    for (y = 0; y < i; y++)
        pthread_create(&thbase[y], NULL, emulator_singleton_thread, (void *) file);

    for (y = 0; y < i; y++)
        pthread_join(thbase[y], NULL);

    return EXIT_SUCCESS;
}

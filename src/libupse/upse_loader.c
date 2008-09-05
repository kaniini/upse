/*
 * UPSE: the unix playstation sound emulator.
 *
 * Filename: upse-loader.h
 * Purpose: libupse: Loader management.
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

#include "upse-internal.h"

typedef struct _upse_loader {
    struct _upse_loader *prev, *next;
    const char *bytes;
    int length;
    int offset;
    upse_loader_func_t func;
} upse_loader_t;

static upse_loader_t *loader_list_ = NULL;

void
upse_loader_add_magic(const char *bytes, int length, int offset, upse_loader_func_t func)
{
    upse_loader_t *loader = calloc(sizeof(upse_loader_t), 1);

    loader->bytes = bytes;
    loader->length = length;
    loader->offset = offset;
    loader->func = func;

    loader->next = loader_list_;
    loader->next->prev = loader;

    loader_list_ = loader;
}

void
upse_loader_del_magic(const char *bytes, int length, int offset)
{
    upse_loader_t *iter, *iter2;

    for (iter = loader_list_, iter2 = iter->next; iter2 != NULL; iter = iter2, iter2 = iter2->next)
    {
         if (iter->length != length)
             continue;

         if (iter->offset != offset)
             continue;

         if (!memcmp(iter->bytes, bytes, iter->length))
         {
             if (iter->prev)
                 iter->prev->next = iter->next;

             if (iter->next)
                 iter->next->prev = iter->prev;

             if (loader_list_ == iter)
                 loader_list_ = iter->next;

             free(iter);
         }
    }
}

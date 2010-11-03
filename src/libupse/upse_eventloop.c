/*
 * UPSE: the unix playstation sound emulator.
 *
 * Filename: upse_eventloop.c
 * Purpose: libupse: System-specific event loop management.
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

void
upse_eventloop_run(upse_module_t *mod)
{
    if (mod->evloop_run)
        mod->evloop_run();
}

void
upse_eventloop_stop(upse_module_t *mod)
{
    if (mod->evloop_stop)
        mod->evloop_stop();
}

int
upse_eventloop_render(upse_module_t *mod, s16 **samples)
{
    if (mod->evloop_render)
        return mod->evloop_render(samples);

    return 0;
}

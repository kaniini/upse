/*
 * UPSE: the unix playstation sound emulator.
 *
 * Filename: psfbuild_main.c
 * Purpose: libupse: psfbuild utility.
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

#include <upse.h>
#include <upse-filesystem.h>

#include <zlib.h>
#include <limits.h>

#include <sys/stat.h>
#include <sys/types.h>

#include <errno.h>
#include <strings.h>

int psfbuild_process(const char *src, const char *dst)
{
    FILE *f;
    u8 compbuf[0x300000], uncompbuf[0x300000];
    uLongf ucl, cl, ccrc;
    int r;

    f = fopen(src, "rb");
    if (f == NULL)
        return 0;

    ucl = fread(uncompbuf, 1, sizeof(uncompbuf), f);
    fclose(f);

    cl = 0x300000;
    if ((r = compress2(compbuf, &cl, uncompbuf, ucl, 9)) != Z_OK)
        return 0;

    f = fopen(dst, "wb");
    if (f == NULL)
        return 0;

    fputc('P', f);
    fputc('S', f);
    fputc('F', f);
    fputc(0x1, f);

    fputc(0x0, f);
    fputc(0x0, f);
    fputc(0x0, f);
    fputc(0x0, f);

    fputc(cl >>  0, f);
    fputc(cl >>  8, f);
    fputc(cl >> 16, f);
    fputc(cl >> 24, f);

    ccrc = crc32(crc32(0L, Z_NULL, 0), compbuf, cl);

    fputc(ccrc >>  0, f);
    fputc(ccrc >>  8, f);
    fputc(ccrc >> 16, f);
    fputc(ccrc >> 24, f);

    fwrite(compbuf, 1, cl, f);

    fclose(f);

    return 1;
}

int main(int argc, const char *argv[]) {
    int i;

    for (i = 1; i < argc; i++) {
        int r;
        char buf[PATH_MAX];

        strncpy(buf, argv[i], sizeof buf);
        strncat(buf, ".psf", sizeof buf);

        printf("%s -> %s ..", argv[i], buf);
        r = psfbuild_process(argv[i], buf);
        printf("%s.\n", r == 1 ? "ok" : "error");
    }

    return EXIT_SUCCESS;
}

/*
 * UPSE: the unix playstation sound emulator.
 *
 * Filename: upse_loader_psf2.c
 * Purpose: libupse: PSF2/MiniPSF2 loader.
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

static char *_upse_resolve_path(const char *f, const char *newfile)
{
    static char *ret;
    char *tp1;

#if PSS_STYLE==1
    tp1 = ((char *) strrchr(f, '/'));
#else
    tp1 = ((char *) strrchr(f, '\\'));
#if PSS_STYLE!=3
    {
        char *tp3;

        tp3 = ((char *) strrchr(f, '/'));
        if (tp1 < tp3)
            tp1 = tp3;
    }
#endif
#endif
    if (!tp1)
    {
        ret = malloc(strlen(newfile) + 1);
        strcpy(ret, newfile);
    }
    else
    {
        ret = malloc(tp1 - f + 2 + strlen(newfile));    // 1(NULL), 1(/).
        memcpy(ret, f, tp1 - f);
        ret[tp1 - f] = '/';
        ret[tp1 - f + 1] = 0;
        strcat(ret, newfile);
    }
    return (ret);
}

static u32
upse_psf2_parse_filesystem(upse_filesystem_t *ret, char *curdir, u8 *filesys, u8 *start, u32 len)
{
    int numfiles, i, j;
    u8 *cptr;
    u32 offs, uncomp, bsize, cofs, uofs;
    u32 X;
    uLongf dlength;
    int uerr;
    u32 tlen;
    char tfn[4096];
    u32 buflen = (16 * 1024 * 1024);
    u8 *buf = calloc(sizeof(u8), buflen);

    tlen = len;
    cptr = start + 4;
    numfiles = start[0] | start[1]<<8 | start[2]<<16 | start[3]<<24;

    _DEBUG("beginning parsing of filesystem @%p", filesys);

    for (i = 0; i < numfiles; i++)
    {
         offs = cptr[36] | cptr[37]<<8 | cptr[38]<<16 | cptr[39]<<24;
         uncomp = cptr[40] | cptr[41]<<8 | cptr[42]<<16 | cptr[43]<<24;
         bsize = cptr[44] | cptr[45]<<8 | cptr[46]<<16 | cptr[47]<<24;

         if (bsize > 0)
         {
              X = (uncomp + bsize - 1) / bsize;

              _DEBUG("file %s [filesystem @%p], %d block(s) (size %d, uncompressed %d)", cptr, filesys, X, bsize, uncomp);

              cofs = (offs + (X*4));
              uofs = 0;
              for (j = 0; j < X; j++)
              {
                  u32 usize = filesys[offs+(j*4)] | filesys[offs+1+(j*4)]<<8 | filesys[offs+2+(j*4)]<<16 | filesys[offs+3+(j*4)]<<24;
                  dlength = buflen - uofs;

                  _DEBUG("uncompressing block %d of %d (dlength %d, start %d, size %d)", j + 1, X, dlength, cofs, usize);
                  uerr = uncompress(&buf[uofs], &dlength, &filesys[cofs], usize);
                  if (uerr != Z_OK)
                  {
                      _WARN("uncompress failed, uerr:%d, buf:%s", uerr, &buf[uofs]);
                      return 0;
                  }

                  cofs += usize;
                  uofs += dlength;
              }

              snprintf(tfn, 4096, "%s/%s", curdir, cptr);
              upse_filesystem_attach_path(ret, tfn, buf, uncomp);
         }
         else
         {
              char lcurdir[4096];
              _DEBUG("new subdirectory [filesystem @%p]: %s %d %d %d", filesys, cptr, offs, uncomp, bsize);

              strncpy(lcurdir, curdir, 4096);
              strncat(lcurdir, "/", 4096);
              strncat(lcurdir, (char *) cptr, 4096);

              _DEBUG("parsing directory %s", lcurdir);
              upse_psf2_parse_filesystem(ret, lcurdir, filesys, &filesys[offs], len);
         }

         cptr += 48;
    }

    return (len - (cptr - filesys));
}

upse_module_t *
upse_load_psf2(void *fp, const char *path, upse_iofuncs_t *iofuncs)
{
    upse_xsf_t *xsf;
    u8 *in, *out;
    u32 inlen;
    u64 outlen;
    upse_filesystem_t *fs;
    upse_module_t *ret;
    char curdir[4096] = "";

    in = upse_get_buffer(fp, iofuncs, &inlen);
    xsf = upse_xsf_decode(in, inlen, &out, &outlen);

    if (outlen > 0)
        return NULL;

    _DEBUG("filesystem length %d bytes", xsf->res_size);
    fs = upse_filesystem_new();
    upse_psf2_parse_filesystem(fs, curdir, (u8 *) xsf->res_section, (u8 *) xsf->res_section, xsf->res_size);
    _DEBUG("lib: %s, libaux[0]: %s", xsf->lib, xsf->libaux[0]);
    if (*xsf->lib != '\0')
    {
        upse_xsf_t *lib;
        void *fplib;
        u8 *inlib, *outlib;
        u32 inliblen;
        u64 outliblen;
        char *tmpfn;
        char lcurdir[4096] = "";

        tmpfn = _upse_resolve_path(path, xsf->lib);
        _DEBUG("opening %s", tmpfn);
        fplib = iofuncs->open_impl(tmpfn, "rb");

        inlib = upse_get_buffer(fplib, iofuncs, &inliblen);
        lib = upse_xsf_decode(inlib, inliblen, &outlib, &outliblen);

        _DEBUG("subfilesystem length %d bytes", lib->res_size);
        upse_psf2_parse_filesystem(fs, lcurdir, (u8 *) lib->res_section, (u8 *) lib->res_section, lib->res_size);

        free(inlib);
        free(outlib);
        free(lib);
    }

    free(in);
    free(out);
    free(xsf);

    ret = calloc(sizeof(upse_module_t), 1);
    ret->opaque = fs;

    return ret;
}

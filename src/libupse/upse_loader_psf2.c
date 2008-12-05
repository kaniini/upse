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

static char *_upse_resolve_path(char *f, char *newfile)
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

static upse_filesystem_t *
upse_psf2_parse_filesystem(upse_filesystem_t *ret, u8 *filesys, u32 len)
{
    int numfiles, i, j;
    u8 *cptr;
    u32 offs, uncomp, bsize, cofs, uofs;
    u32 X;
    uLongf dlength;
    int uerr;
    u8 *start;
    u32 tlen;
    char tfn[4096];
    char curdir[4096] = ".";
    u32 buflen = (16 * 1024 * 1024);
    u8 *buf = calloc(sizeof(u8), buflen);

    start = filesys;
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

              _DEBUG("file %s [filesystem @%p], %d blocks (size %d)", cptr, filesys, bsize, X);

              cofs = offs + (X*4);
              uofs = 0;
              for (j = 0; j < X; j++)
              {
                  u32 usize = start[offs+(j*4)] | start[offs+1+(j*4)]<<8 | start[offs+2+(j*4)]<<16 | start[offs+3+(j*4)]<<24;
                  dlength = buflen - uofs;

                  uerr = uncompress(&buf[uofs], &dlength, &start[cofs], usize);
                  if (uerr != Z_OK)
                  {
                      _WARN("filesystem @%p failed to parse!", filesys);
                      free(ret);
                      return NULL; 
                  }

                  cofs += usize;
                  uofs += dlength;
              }

              snprintf(tfn, 4096, "%s/%s", curdir, cptr);
              upse_filesystem_attach_path(ret, curdir, buf, uncomp);
         }
         else
         {
              _DEBUG("new subdirectory [filesystem @%p]: %s", filesys, cptr);
              strncat(curdir, "/", 4096);
              strncat(curdir, (char *) cptr, 4096);
              _DEBUG("current directory %s", curdir);
         }

         cptr += 48;
    }

    return ret;
}

upse_module_t *
upse_load_psf2(void *fp, char *path, upse_iofuncs_t *iofuncs)
{
    upse_xsf_t *xsf;
    u8 *in, *out;
    u32 inlen;
    u64 outlen;
    upse_filesystem_t *fs;
    upse_module_t *ret;

    in = upse_get_buffer(fp, iofuncs, &inlen);
    xsf = upse_xsf_decode(in, inlen, &out, &outlen);

    if (outlen > 0)
        return NULL;

    _DEBUG("filesystem length %d bytes", xsf->res_size);
    fs = upse_filesystem_new();
    upse_psf2_parse_filesystem(fs, (u8 *) xsf->res_section, xsf->res_size);
    if (xsf->lib[0])
    {
        upse_xsf_t *lib;
        void *fplib;
        u8 *inlib, *outlib;
        u32 inliblen;
        u64 outliblen;
        char *tmpfn;

        tmpfn = _upse_resolve_path(path, xsf->lib);
        fplib = iofuncs->open_impl(path, "rb");

        inlib = upse_get_buffer(fplib, iofuncs, &inliblen);
        lib = upse_xsf_decode(inlib, inlen, &outlib, &outliblen);

        _DEBUG("subfilesystem length %d bytes", lib->res_size);
        upse_psf2_parse_filesystem(fs, (u8 *) lib->res_section, lib->res_size);

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

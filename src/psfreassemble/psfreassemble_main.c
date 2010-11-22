/*
 * UPSE: the unix playstation sound emulator.
 *
 * Filename: psfreassemble_main.c
 * Purpose: libupse: psfreassemble utility.
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

#include <zlib.h>

#include <sys/stat.h>
#include <sys/types.h>

#include <errno.h>
#include <strings.h>

#define PSXM(ins, mem)          (ins->upse_ps1_memory_LUT[(mem) >> 16] == 0 ? NULL : (void*)(ins->upse_ps1_memory_LUT[(mem) >> 16] + ((mem) & 0xffff)))
#define PSXMu8(ins, mem)        (*(u8 *)PSXM(ins, mem))

static void *
psfreassemble_open_impl(const char *path, const char *mode)
{
    return fopen(path, mode);
}

static size_t
psfreassemble_read_impl(void *ptr, size_t sz, size_t nmemb, void *file)
{
    FILE *f = (FILE *) ptr;

    return fread(f, sz, nmemb, file);
}

static int
psfreassemble_seek_impl(void *ptr, long offset, int whence)
{
    FILE *f = (FILE *) ptr;

    return fseek(f, offset, whence);
}

static int
psfreassemble_close_impl(void *ptr)
{
    FILE *f = (FILE *) ptr;

    return fclose(f);
}

static long
psfreassemble_tell_impl(void *ptr)
{
    FILE *f = (FILE *) ptr;

    return ftell(f);
}
static upse_iofuncs_t psfreassemble_iofuncs = {
    psfreassemble_open_impl,
    psfreassemble_read_impl,
    psfreassemble_seek_impl,
    psfreassemble_close_impl,
    psfreassemble_tell_impl
};

typedef struct
{
    unsigned char id[8];
    u32 text;
    u32 data;
    u32 pc0;
    u32 gp0;
    u32 t_addr;
    u32 t_size;
    u32 d_addr;
    u32 d_size;
    u32 b_addr;
    u32 b_size;
    u32 s_addr;
    u32 s_size;
    u32 SavedSP;
    u32 SavedFP;
    u32 SavedGP;
    u32 SavedRA;
    u32 SavedS0;
} upse_exe_header_t;

int
main(int argc, const char *argv[])
{
    upse_module_t *mod;
    FILE *f;
    u8 *exebuf, *writer;
    u8 compbuf[0x300000];
    u32 exesize, readaddr;
    upse_exe_header_t *head;
    upse_module_instance_t *ins;
    uLongf cl, ccrc;

    if (argc < 3)
    {
        printf("%s: usage: in.minipsf out.psf\n", argv[0]);
        return -1;
    }

    upse_module_init();
    mod = upse_module_open(argv[1], &psfreassemble_iofuncs);
    if (mod == NULL)
        return -1;

    ins = &mod->instance;

    printf("Reassembling %s into %s:\n\n", argv[1], argv[2]);

    exesize = (ins->highest_addr - ins->lowest_addr) + ins->highest_addr_size;

    printf("Lowest load address : 0x%.8X\n", ins->lowest_addr);
    printf("Highest load address: 0x%.8X\n", ins->highest_addr);
    printf("Total image size    : 0x%.8X\n", exesize);
    printf("Entry point         : 0x%.8X\n", ins->cpustate.pc);
    printf("GP base             : 0x%.8X\n", ins->cpustate.GPR.n.gp);
    printf("Stack base          : 0x%.8X\n", ins->cpustate.GPR.n.sp);

    exebuf = malloc(sizeof(upse_exe_header_t) + 0x800 + exesize);
    head = (upse_exe_header_t *) exebuf;

    printf("\nBuilding PS-X EXE header.\n");

    head->id[0] = 'P';
    head->id[1] = 'S';
    head->id[2] = '-';
    head->id[3] = 'X';
    head->id[4] = ' ';
    head->id[5] = 'E';
    head->id[6] = 'X';
    head->id[7] = 'E';

    head->text = 0;
    head->data = 0;
    head->pc0 = ins->cpustate.pc;
    head->gp0 = ins->cpustate.GPR.n.gp;

    head->t_addr = ins->lowest_addr;
    head->t_size = exesize + 0x4C;

    head->d_addr = 0;
    head->d_size = 0;
    head->b_addr = 0;
    head->b_size = 0;

    head->s_addr = ins->cpustate.GPR.n.sp;
    head->s_size = 0;

    head->SavedSP = 0;
    head->SavedFP = 0;
    head->SavedGP = 0;
    head->SavedRA = 0;
    head->SavedS0 = 0;

    printf("Assembling binary image.\n");
    for (readaddr = ins->lowest_addr, writer = exebuf + 0x800;
         (writer - exebuf) < exesize; readaddr++, writer++)
    {
        *writer = PSXMu8(ins, readaddr);
    }

    printf("PS-X EXE image assembled.\n");

    upse_module_close(mod);
    ins = NULL;

    printf("Compressing PS-X EXE image payload for PSF container.\n");

    cl = 0x300000;
    if (compress2(compbuf, &cl, exebuf, sizeof(upse_exe_header_t) + 0x800 + exesize, 9) != Z_OK)
        return 0;    

    printf("Image compressed successfully, compressed image payload is %ld bytes.\n", cl);

    ccrc = crc32(crc32(0L, Z_NULL, 0), compbuf, cl);

    printf("Calculated image CRC is: %.08lX.\n", ccrc);

    f = fopen(argv[2], "wb");
    if (f == NULL)
    {
        perror(argv[2]);
        return -1;
    }

    printf("\nOpened %s for writing.\n", argv[2]);

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

    fputc(ccrc >>  0, f);
    fputc(ccrc >>  8, f);
    fputc(ccrc >> 16, f);
    fputc(ccrc >> 24, f);

    fwrite(compbuf, 1, cl, f);

    fclose(f);

    printf("Wrote %s successfully.\n", argv[2]);

    return 0;
}

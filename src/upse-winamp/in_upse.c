/*
 * UPSE: the unix playstation sound emulator.
 *
 * Filename: in_upse.c
 * Purpose: Winamp UPSE plugin.
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

#include <windows.h>
#include <mmreg.h>
#include <msacm.h>
#include <math.h>
#include <stdio.h>

#include "in2.h"

#include "../libupse/upse.h"

#define WM_WA_MPEG_EOF WM_USER+2

#pragma warning (disable: 4133)

extern In_Module mod;
char lastfn[MAX_PATH];

int seekTime;
int paused;
DWORD WINAPI __stdcall PlayThread(void *b);

int killDecodeThread=0;
HANDLE thread_handle=INVALID_HANDLE_VALUE;

void config(HWND hwndParent) { }

void about(HWND hwndParent) { }

void init() { }

void quit() { }

int isourfile(char *fn) { return 0; }

static void *upse_winamp_open_impl(char *path, char *mode)
{
    return fopen(path, mode);
}

static size_t upse_winamp_read_impl(void *ptr, size_t sz, size_t nmemb, void *file)
{
    FILE *f = (FILE *) ptr;

    return fread(f, sz, nmemb, file);
}

static int upse_winamp_seek_impl(void *ptr, long offset, int whence)
{
    FILE *f = (FILE *) ptr;

    return fseek(f, offset, whence);
}

static int upse_winamp_close_impl(void *ptr)
{
    FILE *f = (FILE *) ptr;

    return fclose(f);
}

static upse_iofuncs_t upse_winamp_iofuncs = {
    upse_winamp_open_impl,
    upse_winamp_read_impl,
    upse_winamp_seek_impl,
    upse_winamp_close_impl
};

void upse_winamp_update_cb(unsigned char *buffer, long count, void *data);
int play(char *fn) { 
	int maxlatency;
	int tmp;
	strcpy(lastfn,fn);

	killDecodeThread=0;
	thread_handle = (HANDLE) CreateThread(NULL,0,(LPTHREAD_START_ROUTINE) PlayThread,(void *) &lastfn,0,&tmp);
	
	return 0; 
}

void pause() { paused=1; mod.outMod->Pause(1); }
void unpause() { paused=0; mod.outMod->Pause(0); }
int ispaused() { return paused; }

void stop() { 
	if (thread_handle != INVALID_HANDLE_VALUE)
	{
		killDecodeThread=1;
		if (WaitForSingleObject(thread_handle,INFINITE) == WAIT_TIMEOUT)
		{
			MessageBox(mod.hMainWindow,"error asking thread to die!\n","error killing decode thread",0);
			TerminateThread(thread_handle,0);
		}
		CloseHandle(thread_handle);
		thread_handle = INVALID_HANDLE_VALUE;
	}

	mod.outMod->Close();
	mod.SAVSADeInit();
}

int getlength() { return -1000; }
int getoutputtime() { return mod.outMod->GetOutputTime(); }
void setoutputtime(int time_in_ms) { seekTime = time_in_ms / 1000; }

void setvolume(int volume) { mod.outMod->SetVolume(volume); }
void setpan(int pan) { mod.outMod->SetPan(pan); }

int infoDlg(char *fn, HWND hwnd)
{
	return 0;
}

void getfileinfo(char *filename, char *title, int *length_in_ms)
{
	if (!filename || !*filename)
		filename=lastfn;

	if (title) 
	{
		wsprintf(title,"Tag support not yet implemented.");
	}

	if (length_in_ms) *length_in_ms=-1000;
}

void eq_set(int on, char data[10], int preamp) 
{ 
}


In_Module mod = 
{
	IN_VER,
	"Unix Playstation Sound Emulator WinAmp Plugin",
	0,	// hMainWindow
	0,  // hDllInstance
	"PSF\0PlayStation 1 Module Format (*.PSF)\0"
	"MINIPSF\0PlayStation 1 Module Format (*.MiniPSF)\0",
	1,	// is_seekable
	1, // uses output
	config,
	about,
	init,
	quit,
	getfileinfo,
	infoDlg,
	isourfile,
	play,
	pause,
	unpause,
	ispaused,
	stop,
	
	getlength,
	getoutputtime,
	setoutputtime,

	setvolume,
	setpan,

	0,0,0,0,0,0,0,0,0, // vis stuff


	0,0, // dsp

	eq_set,

	NULL,		// setinfo

	0 // out_mod

};

__declspec( dllexport ) In_Module * winampGetInModule2()
{
	return &mod;
}

int _fltused=0;

void upse_winamp_update_cb(unsigned char *buffer, long count, void *data)
{
	const int mask = ~((((16 / 8) * 2)) - 1);

	while (count > 0)
	{
		int t = mod.outMod->CanWrite() & mask;
		
		if (killDecodeThread)
			upse_stop();

		if (t > count)
		{
			int y;

			mod.outMod->Write(buffer, count);

			y = mod.outMod->GetWrittenTime();
			mod.SAAddPCMData(buffer, count / 2, 16, y);
			mod.VSAAddPCMData(buffer, count / 2, 16, y);
		}
		else
		{
			if (t)
			{
				int y;

				mod.outMod->Write(buffer, count);

				y = mod.outMod->GetWrittenTime();
				mod.SAAddPCMData(buffer, count / 2, 16, y);
				mod.VSAAddPCMData(buffer, count / 2, 16, y);
			}

			Sleep((count - t) * 5 / 441 / 2);
		}

		count -= t;
		buffer += t;
	}

	if (seekTime)
	{
		if (upse_seek(seekTime))
		{
			mod.outMod->Flush(seekTime);
			seekTime = 0;
		}
		else
		{
			upse_stop();
			return;
		}
	}

	if (killDecodeThread)
		upse_stop();
}

DWORD WINAPI __stdcall PlayThread(void *b)
{
	int maxlatency;
	upse_psf_t *psf;
	char filename[MAX_PATH];
	strcpy(filename, (char *) b);

	psf = upse_load(filename, &upse_winamp_iofuncs);
	if (!psf)
		return 1;

	upse_set_audio_callback(upse_winamp_update_cb, NULL);

	paused=0;
	
	maxlatency = mod.outMod->Open(44100,1,16, -1,-1);
	if (maxlatency < 0)
	{
		upse_free_psf_metadata(psf);
		return 1;
	}

	mod.SetInfo(0,44,1,1);
	mod.SAVSAInit(maxlatency,44100);
	mod.VSASetInfo(44100,1);
	mod.outMod->SetVolume(-666);		

	for (;;)
	{
		upse_execute();

		if (killDecodeThread)
			break;

		if (seekTime)
		{
			mod.outMod->Flush(seekTime);

			upse_free_psf_metadata(psf);
			if (!(psf = upse_load(lastfn, &upse_winamp_iofuncs)))
				break;

			upse_seek(seekTime);
			seekTime = 0;
			
			continue;
		}

		while (mod.outMod->IsPlaying())
			Sleep(10);
	}

	mod.outMod->Close();

	return 0;
}
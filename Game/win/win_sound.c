#include "win_init.h"
#include <stdio.h>
#include <dsound.h>
#include <mmsystem.h>
#include "file.h"
#include "win_sound.h"
#include "error.h"
#include "utils.h"
#include "mmreg.h"
#include "memcheck.h"
#include <assert.h>


#pragma pack(1)
typedef struct
{
    BYTE        RIFF[4];          /* "RIFF" */
    DWORD       dwSize;           /* Size of data to follow */
    BYTE        WAVE[4];          /* "WAVE" */
    BYTE        fmt_[4];          /* "fmt " */
    DWORD       dw16;             /* 16 */
    WORD        wOne_0;           /* 1 */
    WORD        wChnls;           /* Number of Channels */
    DWORD       dwSRate;          /* Sample Rate */
    DWORD       BytesPerSec;      /* Sample Rate */
    WORD        wBlkAlign;        /* 1 */
    WORD        BitsPerSample;    /* Sample size */
    BYTE        DATA[4];          /* "DATA" */
    DWORD       dwDSize;          /* Number of Samples */
} WaveHeader;
#pragma pack()

#define NUM_SOUNDS		500
#define NUM_CHANNELS	16

int					no_audio;
static LPDIRECTSOUND		gpds;

typedef struct
{
	IDirectSoundBuffer	*g_lpSounds[NUM_CHANNELS]; /* Sound buffers */
	U8					type;
	U32					size;
} WinSound;

typedef struct
{
	int			pan,vol;
	WinSound	*win_sound;
} ChanParms;

ChanParms chan_parms[NUM_CHANNELS];

enum
{
	SOUND_OK,
	SOUND_MCI,
	SOUND_BAD,
};

/* Function CreateSoundBuffer()
 *	Create a DirectSound buffer.
 *
 *
 */
static IDirectSoundBuffer *CreateSoundBuffer(int dwBufSize, int dwFreq, int dwBitsPerSample,int dwBlkAlign, int bStereo)
{
	PCMWAVEFORMAT		pcmwf;
	DSBUFFERDESC		dsbdesc;
	int					hr;
	IDirectSoundBuffer	*dwBuf;

    /* Set up wave format structure. */
    memset( &pcmwf, 0, sizeof(PCMWAVEFORMAT) );
    pcmwf.wf.wFormatTag         = WAVE_FORMAT_PCM; // WAVE_FORMAT_MPEG
    pcmwf.wf.nChannels          = bStereo ? 2 : 1;
    pcmwf.wf.nSamplesPerSec     = dwFreq;
    pcmwf.wf.nBlockAlign        = (WORD)dwBlkAlign;
//Layer 1: nBlockAlign = 4*(int)(12*BitRate/SamplingFreq)
//Layers 2 and 3: nBlockAlign = (int)(144*BitRate/SamplingFreq)
    pcmwf.wf.nAvgBytesPerSec    = pcmwf.wf.nSamplesPerSec * pcmwf.wf.nBlockAlign; // 128k/8
    pcmwf.wBitsPerSample        = (WORD)dwBitsPerSample; // 0

    /* Set up DSBUFFERDESC structure. */
    memset(&dsbdesc, 0, sizeof(DSBUFFERDESC));  /* Zero it out.  */
    dsbdesc.dwSize              = sizeof(DSBUFFERDESC);
    dsbdesc.dwFlags             = DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME | DSBCAPS_STICKYFOCUS;
 	dsbdesc.dwBufferBytes       = dwBufSize; 
    dsbdesc.lpwfxFormat         = (LPWAVEFORMATEX)&pcmwf;

	if ((hr = gpds->lpVtbl->CreateSoundBuffer(gpds,
              &dsbdesc,&dwBuf,NULL )) != 0)
    {
		switch(hr)
		{
			case DSERR_ALLOCATED : printf("allocated\n"); break;
			case DSERR_BADFORMAT  : printf("bad format\n"); break;
			case DSERR_INVALIDPARAM  : printf("invalid parm\n"); break;
			case DSERR_NOAGGREGATION  : printf("no aggr\n"); break;
			case DSERR_OUTOFMEMORY  : printf("out of mem\n"); break;
			case DSERR_UNINITIALIZED  : printf("uninit\n"); break;
			case DSERR_UNSUPPORTED  : printf("unsup\n"); break;
			default: printf("unkown error\n");
		}
		return 0;
	}
	return dwBuf;
}

static int ReadData(IDirectSoundBuffer *dwBuf, FILE* pFile, int dwSize) 
{
    LPVOID pData1;
    DWORD  dwData1Size;
    LPVOID pData2;
    DWORD  dwData2Size;
    HRESULT rval;

    /* Lock data in buffer for writing */
    rval = dwBuf->lpVtbl->Lock(dwBuf,0, dwSize, &pData1,
											&dwData1Size, &pData2, &dwData2Size,
											DSBLOCK_FROMWRITECURSOR);
    if (rval != DS_OK)
    {
        return FALSE;
    }

    /* Read in first chunk of data */
    if (dwData1Size > 0) 
    {
        rval=fread(pData1,  dwData1Size,  1,pFile);
		if (rval != 1)
        {          
			printf("%d  %d  Data1 : %d, dwdata: %d, pFile: %d\n",rval,ftell(pFile),pData1,dwData1Size,pFile);
            return FALSE;
        }
    }

    /* read in second chunk if necessary */
    if (dwData2Size > 0) 
    {
        if (fread(pData2, dwData2Size, 1, pFile) != 1) 
        {
            return FALSE;
        }
    }

    /* Unlock data in buffer */
    rval = dwBuf->lpVtbl->Unlock(dwBuf,pData1, dwData1Size,
												pData2, dwData2Size);
    if (rval != DS_OK)
    {
        return FALSE;
    }

    dwBuf->lpVtbl->SetVolume(dwBuf,0);
    dwBuf->lpVtbl->SetPan(dwBuf,0);
 
    return TRUE;
}

static IDirectSoundBuffer *LoadWaveFile(FILE *pFile,int *size)
{
	WaveHeader	wavHdr;
	int			dwSize,bStereo;
	IDirectSoundBuffer	*dwBuf;

    /* Read in the wave header           */
    if (fread(&wavHdr, sizeof(wavHdr), 1, pFile) != 1) 
    {
        return 0;
    }

    /* Figure out the size of the data region */
    dwSize = wavHdr.dwDSize;

	if (dwSize > 100000000)
		return 0;
    /* Is this a stereo or mono file? */
    bStereo = wavHdr.wChnls > 1 ? TRUE : FALSE;

    /* Create the sound buffer for the wave file */
    dwBuf = CreateSoundBuffer(dwSize, wavHdr.dwSRate, wavHdr.BitsPerSample, wavHdr.wBlkAlign, bStereo);
	if (!dwBuf)
    {
         return 0;
    }

    /* Read the data for the wave file into the sound buffer */
    if (!ReadData(dwBuf, pFile, dwSize)) 
    {
        return 0;
    }
	*size = dwSize;
    return dwBuf;
}

static void X_MciStop(int channel)
{
	char	buf[1000];

	sprintf(buf, "Close channel%d", channel);
	mciSendString (buf, 0, 0, 0);
}

static int X_MciPlayFile(int channel,char *fname,int loop)
{
	int		a;
	char	buf[1000];
	static int is_playing;

	if (is_playing)
	{
		X_MciStop(channel);
	}
	sprintf(buf, "open \"%s\" type mpegvideo Alias channel%d", fname, channel);
	a = mciSendString(buf, 0, 0, 0);
	if (a)
		return 0;
	sprintf(buf, "Play channel%d from 0 %s", channel,loop ? "repeat" : "");
	a = mciSendString (buf, 0, 0, 0);
	if (a)
		return 0;
	is_playing = 1;
	return 1;
}

static void X_MciVolumePan(int channel,int volume,int pan)
{
	int		a,left_vol,right_vol;
	char	buf[1000];
	F32		ratio;

	ratio = pan * 1.f / 256;
	left_vol = volume * 4 * (1-ratio);
	right_vol = volume * 4 * ratio;
	sprintf(buf, "setaudio channel%d right volume to %d", channel, right_vol);
	a = mciSendString (buf, 0, 0, 0);
	sprintf(buf, "setaudio channel%d left volume to %d", channel, left_vol);
	a = mciSendString (buf, 0, 0, 0);
}

static int X_MciPlaying(int channel)
{
	char	buf[100],result[100];
	int		a;

	sprintf(buf,"status channel%d mode",channel);
	a = mciSendString (buf, result,sizeof(result)-1, 0);
	if (stricmp(result,"playing")==0)
		return 1;
	return 0;
}

void *X_LoadAudioFile(char *fname)
{
	WinSound	*win_sound;
	FILE		*file;
	char		*s;
	int			j,size;

	if (no_audio)
		return 0;
	file = fileOpen(fname,"rb");
	if (!file)
		return 0;
	win_sound = calloc(sizeof(WinSound),1);
	s = strrchr(fname,'.');
	if (strnicmp(s,".wav",4)==0)
	{
		win_sound->g_lpSounds[0] = LoadWaveFile(file,&size);
		if (win_sound->g_lpSounds[0])
		{
			win_sound->type = SOUND_OK;
			win_sound->size = size;
			for(j=1;j<NUM_CHANNELS;j++)
			{
				gpds->lpVtbl->DuplicateSoundBuffer(gpds,win_sound->g_lpSounds[0],&win_sound->g_lpSounds[j]);
			}
		}
		else
			win_sound->type = SOUND_BAD;
	}
	else
	{
		win_sound->type = SOUND_MCI;
	}
	fclose(file);
	return win_sound;
}

void X_RewindSound(int channel,void *sound_data)
{
	WinSound	*win_sound = sound_data;

	if (!win_sound)
		return;

	if (win_sound->type == SOUND_OK)
		IDirectSoundBuffer_SetCurrentPosition(win_sound->g_lpSounds[channel], 0);
	if (win_sound->type == SOUND_MCI)
		;
}

void X_UpdateSound(int channel,int volume,int pan)
{
	WinSound	*win_sound;

	if (no_audio)
		return;
	if (chan_parms[channel].vol == volume && chan_parms[channel].pan == pan)
		return;
	//printf("channel %d, volume %d pan %d\n",channel,volume,pan);
	win_sound = chan_parms[channel].win_sound;
	chan_parms[channel].vol = volume;
	chan_parms[channel].pan = pan;
	if (win_sound->type == SOUND_MCI)
		X_MciVolumePan(channel,volume,pan);
	if (win_sound->type == SOUND_OK)
	{
		// input is 0..255
		/* 0 = full volume, -10000 = silent */
		if (!volume)
			volume = -10000;
		else
		{
			F32		hundredths_of_dB,fraction;

			fraction = volume / 255.f;
			hundredths_of_dB = 2000.0 * log10(fraction);
			volume = hundredths_of_dB;
		}

		// input is 0..255
		/* -10000 left, 10000 right */
		pan = (pan - 128) * 40;

		win_sound->g_lpSounds[channel]->lpVtbl->SetVolume(win_sound->g_lpSounds[channel], volume);
 		win_sound->g_lpSounds[channel]->lpVtbl->SetPan(win_sound->g_lpSounds[channel], pan);
	}
}

void X_PlaySound(int channel,void *sound_data,int volume,int pan,int loop,char *name)
{
	int			hr;
	WinSound	*win_sound = sound_data;

	if(no_audio)
		return;
	
	chan_parms[channel].win_sound = sound_data;
	chan_parms[channel].vol = 99999;
	chan_parms[channel].pan = 99999;
	if (win_sound->type == SOUND_MCI)
	{
		char	buf[1000];

		assert(name);
		sprintf(buf,"%s\\%s",fileDataDir(),name);
		X_MciPlayFile(channel,buf,loop);
		X_UpdateSound(channel,volume,pan);
	}
	else
	{
		IDirectSoundBuffer_SetCurrentPosition(win_sound->g_lpSounds[channel], 0);
		X_UpdateSound(channel,volume,pan);
 		hr = win_sound->g_lpSounds[channel]->lpVtbl->Play(win_sound->g_lpSounds[channel], 0, 0, loop ? DSBPLAY_LOOPING : 0 );
	}
}

void X_StopSound(int channel)
{
	WinSound	*win_sound = chan_parms[channel].win_sound;

	if (!win_sound)
		return;
	if (win_sound->type == SOUND_OK)
	  win_sound->g_lpSounds[channel]->lpVtbl->Stop(win_sound->g_lpSounds[channel]);
	if (win_sound->type == SOUND_MCI)
		X_MciStop(channel);
}

int X_SoundPlaying(int channel,float time)
{
	int			status;
	WinSound	*win_sound = chan_parms[channel].win_sound;

	if (!win_sound)
		return 0;
	if (win_sound->type == SOUND_OK)
	{
		IDirectSoundBuffer_GetStatus(win_sound->g_lpSounds[channel],&status);

		if (status & DSBSTATUS_PLAYING)
			return 1;
	}
	if (win_sound->type == SOUND_MCI)
		return X_MciPlaying(channel);
	return 0;
}

void X_UpdateAudio()
{
}


void X_InitAudio()
{
	LPDIRECTSOUNDBUFFER		lpDSb;
	DSBUFFERDESC			dsbdesc;
	WAVEFORMATEX			wfm;

    if(FAILED(DirectSoundCreate(NULL,&gpds,NULL)))
	{
		no_audio = 1;
		return;
		FatalErrorf("Couldn't start DirectSound!");
	}
	else
	{
		if(FAILED(IDirectSound_SetCooperativeLevel(gpds, hwnd, DSSCL_PRIORITY)))
			FatalErrorf("Couldn't set DirectSound cooperative level!");
		memset(&dsbdesc,0,sizeof(DSBUFFERDESC));
		dsbdesc.dwSize = sizeof(DSBUFFERDESC);
		dsbdesc.dwFlags = DSBCAPS_PRIMARYBUFFER;
		dsbdesc.dwBufferBytes = 0;
		dsbdesc.lpwfxFormat = NULL;
		memset(&wfm,0,sizeof(WAVEFORMATEX));
		wfm.wFormatTag = WAVE_FORMAT_PCM;
		wfm.nChannels = 2;
		wfm.nSamplesPerSec = 44100;
		wfm.wBitsPerSample = 16;
		wfm.nBlockAlign = wfm.wBitsPerSample / 8 * wfm.nChannels;
		wfm.nAvgBytesPerSec = wfm.nSamplesPerSec * wfm.nBlockAlign;

		if(FAILED(IDirectSound_CreateSoundBuffer(gpds,&dsbdesc,&lpDSb,NULL)))
			FatalErrorf("Couldn't create primary soundbuffer");

		if(FAILED(IDirectSoundBuffer_SetFormat(lpDSb,&wfm)))
			FatalErrorf("Couldn't set soundbuffer format");
	}
}

void PlayCDTrack(int track)
{
	char	answer[200];
	char	buf[200];

	track++;
	answer[0] = 0;
	mciSendString("open cdaudio",answer,200,NULL);
	mciSendString("set cdaudio time format tmsf",answer,200,NULL);
	sprintf(buf,"play cdaudio from %d to %d",track,track + 1);
	mciSendString(buf,answer,200,NULL);
	mciSendString("close cdaudio",answer,200,NULL);
}

void X_ExitAudio()
{
	if (gpds)
		IDirectSound_Release(gpds);
}

void X_FreeAudioFile(void *sound_data)
{
	WinSound	*win_sound = sound_data;
	int			j;

	if (!win_sound)
		return;

	if (win_sound->type == SOUND_OK)
	{
		for(j=0;j<NUM_CHANNELS;j++)
		{
			if (win_sound->g_lpSounds[j])
				win_sound->g_lpSounds[j]->lpVtbl->Release(win_sound->g_lpSounds[j]);
			win_sound->g_lpSounds[j] = 0;
		}
	}
	free(win_sound);
}

int X_AudioBytes(void *sound_data)
{
	WinSound	*win_sound = sound_data;

	if (!win_sound)
		return 0;
	return win_sound->size;
}

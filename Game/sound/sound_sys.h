#ifndef _SOUND_SYS_H
#define _SOUND_SYS_H

#include "stdtypes.h"
#include "sound.h"

#define MAX_EVER_SOUND_CHANNELS		128

typedef struct SoundFileInfo
{
	char	*data;
	int		length;
	char	name[128];
	int		ref_count;
	int		last_used;

	// used for pcm cache
	char*	pcm_data;
	int		pcm_len;
	int		frequency;
	int		num_channels;
	int		recent_play_count;
	int		loadIndex;
} SoundFileInfo;

typedef struct DecodeState DecodeState;

typedef struct CodecFuncs{
	int		(*init)		(DecodeState *decode);
	int		(*decode)	(DecodeState *decode);
	void	(*reset)	(DecodeState *decode);
	void	(*rewind)	(DecodeState *decode);
} CodecFuncs;

typedef struct DecodeState
{
	U8				*decode_buffer;
	int				decode_count;	// bytes currently decoded
	int				decode_len;		// max size of decode buffer
	int				num_channels;
	int				frequency;
	SoundFileInfo	*info;
	void			*codec_state; // data needed by the particular codec
	int				pcm_len;		// total length of pcm data
	CodecFuncs		*codec;
} DecodeState;

typedef struct DSParams
{
	double 	innerRadius;
	double 	outerRadius;
	double 	innerAngle;
	double 	outerAngle;
	double 	volume;
	int 	gets3D;
} DSParams;

extern AudioState g_audio_state;

int sndSysChannelGets3D(int channel);
void sndSysUpdateChannel(int channel,F32 volume,Vec3 pos,F32 pan,int doppler);
void sndSysPlay(int channel,void *sound_data,F32 volume,Vec3 pos,F32 pan,int loop,DSParams * params);
void sndSysRewind(int channel,void *sound_data,DSParams * params);
void sndSysStop(int channel);
int sndSysPlaying(int channel);
void sndSysInit(void);
void sndSysUninit(void);
void sndSysExit(void);
void sndSysCleanChannel(int channel);
void sndSysFree(void *sound_data);
int sndSysBytes(void *sound_data);
char *sndSysFormat(void *sound_data);
void *sndSysLoad(char *fname);
void sndSysPositionListener(Mat4 mat,Vec3 vel);
void sndSysCommitSettings(void);
void sndSysTick(void);
void testCompressor(void);
#endif


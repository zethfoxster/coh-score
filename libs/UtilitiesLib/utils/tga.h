#ifndef _TGA_H
#define _TGA_H

// Bit flags
typedef enum {
		// Leave the raw data BGRA.
	kTgaReadOpt_DontSwapToRGBA				= 0x0001,
		// When expanding < 4 bytes-per-pixel to RGBA, if the image data
		// is all zeros, set the alpha channel to 0 rather than 255.
	tTgaReadOpt_AlphaToZeroOnEmptyImage		= 0x0002,
} tTgaReadOpts;

char *tgaLoadEx(FILE *file,int *wp,int *hp, unsigned int read_opts /* tTgaReadOpts */ );
#define tgaLoad(file,wp,hp)	tgaLoadEx(file,wp,hp,0)
char *tgaLoadFromMemory(char *memory,int size,int *wp,int *hp);
void tgaSave(char *filename,char *data,int width,int height,int nchannels);
char *tgaSaveToMem(char *data,int width,int height,int nchannels, int *datalen);

#endif

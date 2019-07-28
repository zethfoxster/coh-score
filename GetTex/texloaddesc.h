#ifndef _TEXLOADDESC_H
#define _TEXLOADDESC_H

#include "tex.h"
typedef struct TexInf
{
	char	*name;
	TextureFileHeader tfh;
} TexInf;

extern TexInf	*tex_infs;
extern int		tex_inf_count,tex_inf_max;

void texLoadAllInfo(int reloadcallback);
TexInf *getTexInf(const char *name);

#endif

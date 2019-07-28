#ifndef _TIFF_H
#define _TIFF_H

void tiffSave(const char * filename, const void * data, int width, int height, int nchannels, int bytesperchannel,
              int bottomfirst, int isfloat, int storealpha);

void * tiffLoad(const char * filename, int * width, int * height, int * nchannels, int * bytesperchannel,
                int * bottomfirst, int * isfloat, int * hasalpha);

#endif

#ifndef DXTLIBWRAPPER_H
#define DXTLIBWRAPPER_H

#include <nvdxt_options.h>

// These nvDXTcompress functions are *much* slower than just sending it to the video card and
//  having it do it.

HRESULT nvDXTcompressC(unsigned char * raw_data, // pointer to data (24 or 32 bit)
		unsigned long w, // width in texels
		unsigned long h, // height in texels
		DWORD byte_pitch,
		CompressionOptions * options,
		DWORD planes, // 3 or 4
		MIPcallback callback,  // callback for generated levels
		RECT * rect);   // subrect to operate on, NULL is whole image

HRESULT nvDXTcompressVolumeC(unsigned char * raw_data, // pointer to data (24 or 32 bit)
		unsigned long w, // width in texels
		unsigned long h, // height in texels
		unsigned long depth, // depth of volume texture
		DWORD byte_pitch,
		CompressionOptions * options,
		DWORD planes, // 3 or 4
		MIPcallback callback,  // callback for generated levels
		RECT * rect);   // subrect to operate on, NULL is whole image

// floating point input
HRESULT nvDXTcompress32FC(fpPixel * raw_data, // pointer to data (24 or 32 bit)
		unsigned long w, // width in texels
		unsigned long h, // height in texels
		DWORD pitch, // in fpPixels
		CompressionOptions * options,
		DWORD planes, // 3 or 4
		MIPcallback callback,  // callback for generated levels
		RECT * rect);   // subrect to operate on, NULL is whole image

unsigned char * nvDXTdecompressC(OUT int * w, OUT int * h, OUT int * depth, OUT int * total_width, OUT int * rowBytes, OUT int * src_format,
								int SpecifiedMipMaps, unsigned char *data);

void initNVDXTCriticalSection(void);

#endif

#ifndef _JPEG_H
#define _JPEG_H

typedef struct TexReadInfo TexReadInfo;

#ifdef __cplusplus
extern "C" {
#endif
int jpegLoad(char *mem,int size,TexReadInfo *info);

void jpgSave( char * name, U8 * pixbuf, int bpp, int x, int y );
void jpgSaveEx( char * name, U8 * pixbuf, int bpp, int x, int y, char *extraJpegData, int extraJpegDataLen);
#ifdef __cplusplus
 }
#endif

#endif

/******************************************************************************
 A .ANIMX is the text file format exported by the animation exporter in 3DS MAX.
 The purpose of this code is to parse that source data into something we
 can operate on. The name is derived from: ANIMation Export.
*****************************************************************************/
#ifndef _IMPORT_ANIMX_H
#define _IMPORT_ANIMX_H

typedef void* NodeAnimHandle;	// handle to anim data for a particular node

unsigned int LoadAnimation( const char* sourcepath );
void FreeAnimation( void );

NodeAnimHandle GetNodeAnimHandle( const char* node_name );

float*	GetNodeFrameTranslation( NodeAnimHandle hNode, int iFrame );
float*	GetNodeFrameAxisAngle( NodeAnimHandle hNode, int iFrame );
float*	GetNodeFrameScale( NodeAnimHandle hNode, int iFrame );

#endif // _IMPORT_ANIMX_H

/******************************************************************************
 A .SKELX is the text file format exporter by the animation exporter in 3DS MAX
 The purpose of this code is to convert that source data into the game runtime
 skeleton hierarchy format. The name is derived from: SKELeton eXport.
*****************************************************************************/
#ifndef _PROCESS_SKELX_H
#define _PROCESS_SKELX_H

///////////////////////////////////////////////////////////////////////////////
// SKELX Processing
///////////////////////////////////////////////////////////////////////////////

Node* LoadSkeletonSKELX( char * sourcepath );

///////////////////////////////////////////////////////////////////////////////
#endif // _PROCESS_SKELX_H

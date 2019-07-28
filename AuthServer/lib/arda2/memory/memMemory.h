/*****************************************************************************
	created:	2002/08/19
	copyright:	2002, NCSoft. All Rights Reserved
	author(s):	Ryan M. Prescott
	
	purpose:	put your generalized allocation stuff here...
*****************************************************************************/

#ifndef   INCLUDED_memMemory_h
#define   INCLUDED_memMemory_h
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000



void* MallocAligned(size_t SizeInBytes, size_t AlignmentInBytes );
void  FreeAligned(void* pMemblock);


#endif // INCLUDED_memMemory_h

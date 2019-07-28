/*****************************************************************************
	created:	2002/04/03
	copyright:	2002, NCSoft. All Rights Reserved
	author(s):	Tom Gambill
	
	purpose:	Debug overloads for new and new[] that call the debug version
				of themselves and supply the actual source line info for 	
				output later using _CrtSetDbgFlag()

*****************************************************************************/
	
#ifndef   INCLUDED_memMemoryDbg
#define   INCLUDED_memMemoryDbg


#if CORE_DEBUG

//* asserts if the heap is corrupted
int MEMDBG_VERIFY_HEAP();

//* Define our special new
//void * __cdecl operator new(size_t tSize, char *szFile, int iLine);
//void * __cdecl operator new[](size_t tSize, char *szFile, int iLine);
//#define new new(__FILE__, __LINE__)

#endif // CORE_DEBUG


#endif // INCLUDED_memMemoryDbg

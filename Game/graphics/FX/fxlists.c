#include <stdio.h>
#include <string.h>
#include <stdlib.h> 
#include "error.h"
#include "memcheck.h"
#include "assert.h" 
#include "utils.h"
#include "assert.h"   
#include "font.h"
#include "ReferenceList.h"

//#define USE_REFERENCELIST  defining this will disable all of this file and just pass through to ReferenceList.c/h


/*Basic idea: Use handles instead of pointers for things that could die without you knowing.

1. When you create something, give it's pointer to this module for safe keeping (hdlAssignHandle) 
and get back a handle you can give to others instead of a pointer. 
2. Retrieve the pointer with (hdlGetPtrFromHandle), returns 0 if the object has been destroyed.
3. When you destroy the thing, you call (hdlClearHandle).

(In a perfect world, owners use the handle and accessor funtions and
never get their hands on the pointer at all. Fx are like that, sequencers aren't yet)

These are the four bread and butter functions the others are odd balls 

int hdlAssignHandle(void * ptr)
void * hdlGetPtrFromHandle(int handle)  
void hdlClearHandle(int handle)

//TO DO: hdl_id_to_ptrs needs to know how to automatically grow. (hard limit is 64k things with valid handles)
//TO DO: rename file to handle.c/.h and moved to utils or common
*/

//(to the outside world a Handle is just an int.)
typedef struct Handle
{
	S16	id;
	U16	idx;
} Handle;

typedef struct IDtoPtr
{
	void * ptr;
	S16	id;
} IDtoPtr;

////////// Data for handle management ///////////////////////////
static IDtoPtr	* hdl_id_to_ptrs;	//big array of all handle - pointer matches
static int		max_handles;	//number of entries in hdl_id_to_ptrs array
static int		curr_hdl_idx;		//place to start searching for a free slot in hdl_id_to_ptrs

static ReferenceList fxReferenceList;

int hdlAssignHandle(void * ptr)
{
#ifdef USE_REFERENCELIST
	return referenceListAddElement(fxReferenceList, ptr);
#else
	Handle handle;
	int start;

	//Get right curridx idx (Find a spot in the array for the next fx (almost always be the first place it looks))
	assert(curr_hdl_idx >= 0 && curr_hdl_idx < max_handles);
	start = curr_hdl_idx;
	while( hdl_id_to_ptrs[curr_hdl_idx].id >= 0 ) //if this gets too expensive, figure out a way to hash it or something
	{
		curr_hdl_idx++;
		if(curr_hdl_idx >= max_handles)
			curr_hdl_idx = 0;
		if( curr_hdl_idx == start )
		{
			printToScreenLog(1, "\nToo many things to keep track of!\n");
			return 0;
		}
	}
	assert(curr_hdl_idx >= 0 && curr_hdl_idx < max_handles);
	//Assign these values 
	hdl_id_to_ptrs[curr_hdl_idx].ptr = ptr;
	// Update a new ID
	assert(hdl_id_to_ptrs[curr_hdl_idx].id < 0 );
	hdl_id_to_ptrs[curr_hdl_idx].id = -hdl_id_to_ptrs[curr_hdl_idx].id + 1;
	if (hdl_id_to_ptrs[curr_hdl_idx].id <= 0) // Wrap 
		hdl_id_to_ptrs[curr_hdl_idx].id = 1;
	handle.id  = hdl_id_to_ptrs[curr_hdl_idx].id;
	handle.idx = curr_hdl_idx;

	return *((int*)&handle);
#endif
}

void hdlClearHandle(int handle)
{
#ifdef USE_REFERENCELIST
	referenceListRemoveElement(fxReferenceList, handle);
#else
	assert( ((Handle*)&handle)->idx >= 0 && ((Handle*)&handle)->idx < max_handles );
	hdl_id_to_ptrs[((Handle*)&handle)->idx].id = -hdl_id_to_ptrs[((Handle*)&handle)->idx].id;
	hdl_id_to_ptrs[((Handle*)&handle)->idx].ptr = 0;
#endif
}


void * hdlGetPtrFromHandle(int handle)
{
#ifdef USE_REFERENCELIST
	return referenceListFindByRef(fxReferenceList, handle);
#else
	assert( ((Handle*)&handle)->idx >= 0 && ((Handle*)&handle)->idx < max_handles );
	if( hdl_id_to_ptrs[((Handle*)&handle)->idx].id == ((Handle*)&handle)->id )
		return hdl_id_to_ptrs[((Handle*)&handle)->idx].ptr; 
	return 0;
#endif
}

/*If you have a pointer and a handle, and just want to be sure its ok before using it.  Really kind of silly*/
int hdlGetHandleFromPtr(void * ptr, int handle)
{
	if( ptr && ptr == hdlGetPtrFromHandle(handle) )
		return handle;
	return 0;
}

/*You should only be doing this when nothing is using the handles, otherwise some will be stranded.
TO DO, the handle array size should automatically resize itself, so you can set it low to start with.
*/
void hdlInitHandles(int initial_max_handles)
{
#ifdef USE_REFERENCELIST
	fxReferenceList = createReferenceList();
#else
	int i;

	assert( initial_max_handles < 65536 ); //two bytes allocated for a handle idx
	max_handles = initial_max_handles;

	hdl_id_to_ptrs = malloc( max_handles * sizeof( IDtoPtr ) );

	//could do a memset to speed it up
	for( i = 0 ; i < max_handles ; i++)
	{
		hdl_id_to_ptrs[i].ptr = 0;
		hdl_id_to_ptrs[i].id = -1;
	}
#endif
}

/*Specialty thing respawnfx uses.
*/
void hdlMoveHandlePtr(int tohandle, int fromhandle)
{
#ifdef USE_REFERENCELIST
	referenceListMoveElement(fxReferenceList, tohandle, fromhandle);
#else
	assert( ((Handle*)&tohandle)->idx >= 0 && ((Handle*)&tohandle)->idx < max_handles );
	assert( ((Handle*)&fromhandle)->idx >= 0 && ((Handle*)&fromhandle)->idx < max_handles );
	hdl_id_to_ptrs[((Handle*)&tohandle)->idx].ptr = hdl_id_to_ptrs[((Handle*)&fromhandle)->idx].ptr;
	hdl_id_to_ptrs[((Handle*)&tohandle)->idx].id = ((Handle*)&tohandle)->id;
#endif
}

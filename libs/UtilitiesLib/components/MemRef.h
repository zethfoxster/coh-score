#ifndef MEMREF_H
#define MEMREF_H

//////////////////////////////////////////////////////////////////////////
// Reference counted memory
// Scheme for reference counting memory so that multiple threads which use
//  data can make sure exactly one of the (the last one) frees it.
// Could also be used for other purposes

void *memrefAllocInternal(size_t size MEM_DBG_PARMS); // Sets reference count to 1
#define memrefAlloc(size) memrefAllocInternal(size MEM_DBG_PARMS_INIT);
int memrefIncrement(void *data);
int memrefDecrement(void *data); // Will free the data if releasing the last reference

#endif

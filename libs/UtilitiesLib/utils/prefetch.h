/* prefetch.h
 */

#ifndef _PREFETCH_H
#define _PREFETCH_H

// NOTE: These are for X86 only

// Normal prefetch.  On P4, should fetch 128bytes
__forceinline void Prefetch(const void * mem)
{
	__asm mov ecx, mem
	__asm prefetcht0 [ecx];
}

// NTA for Non-temporal access.
// This is to try and not pollute the cache
// (access a small amount of data without changing it and
// don't need any of the other data near it)
__forceinline void PrefetchNTA(const void * mem)
{
	__asm mov ecx, mem
	__asm prefetchnta [ecx];
}


#endif // _PREFETCH_H


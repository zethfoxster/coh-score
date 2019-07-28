#ifndef	_MEMORY_H_
#define _MEMORY_H_

class MemoryObject
{
public:
#if 0
	void *operator new(size_t nSize) { return malloc(nSize); }
	void operator delete(void* pData) { free(pData); }
#endif
};

// #define DEBUG_REFCOUNT
#if defined(_DEBUG) && defined(DEBUG_REFCOUNT)
#define AddRef() AddRefD(__FILE__, __LINE__);
#define Release() ReleaseD(__FILE__, __LINE__);
#endif

#endif // _MEMORY_H_

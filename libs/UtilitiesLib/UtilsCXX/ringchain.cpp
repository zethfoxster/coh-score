#include "ringchain.hpp"
#include "threadedpoolallocator.hpp"
#include "futex.hpp"

#define RINGPOINTERS_PER_PAGE 512
#define RINGPAGE_SIZE (sizeof(PointerRingPage) + RINGPOINTERS_PER_PAGE * sizeof(void*))

static ThreadedPoolAllocator * s_ringChainPageAllocator = NULL;

void PointerRingChain::Initialize()
{
	if(!s_ringChainPageAllocator)
		s_ringChainPageAllocator = new ThreadedPoolAllocator(RINGPAGE_SIZE, true, 64, 4);
}

void PointerRingChain::Shutdown()
{
	if (s_ringChainPageAllocator)
		delete s_ringChainPageAllocator;
	s_ringChainPageAllocator = NULL;
}

PointerRingPage * PointerRingChain::alloc_page()
{
	PointerRingPage * page = (PointerRingPage*)s_ringChainPageAllocator->talloc();
	page->set_buffer((void**)(page+1), RINGPOINTERS_PER_PAGE);
	page->next = NULL;
	return page;
}

void PointerRingChain::free_page(PointerRingPage * page)
{
	page->zero_header();
	s_ringChainPageAllocator->tfreenozero(page);
}

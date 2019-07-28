#include "stdtypes.h"

// Thread-safe ring buffer:
//   Statically allocated buffer
//   All allocs and frees must happen in order
//   All allocs must happen in the same thread to be thread-safe
//   All frees must happen in the same thread to be thread-safe
//   All allocations will succeed (will use malloc/free if the buffer is exceeded)

typedef struct RingBuffer {
	U8 *buffer;
	size_t ringbuffer_start;
	size_t ringbuffer_end;
	size_t ringbuffer_size;

	// Performance stats
	int numAllocations; // Total number of allocations
	int numMallocs; // Number of allocations which didn't fit
} RingBuffer;

RingBuffer *createRingBuffer(size_t size);
void ringBufferInit(RingBuffer *rb, size_t size);
void destroyRingBuffer(RingBuffer *rb);

U8 *ringBufferAlloc(RingBuffer *rb, size_t len);
void ringBufferFree(RingBuffer *rb, U8 *data);
char *ringBufferStrdup(RingBuffer *rb, const char *str);

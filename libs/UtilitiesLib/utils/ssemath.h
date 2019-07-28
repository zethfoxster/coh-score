
#ifndef _SSEMATH_H_
#define _SSEMATH_H_

#include "mathutil.h"

void determineIfSSEAvailable(void);

// Bits for testing various SSE support
#define SSE_1   0x01
#define SSE_2   0x02
#define SSE_3   0x04
#define SSE_3_s 0x08
#define SSE_4_1 0x10
#define SSE_4_2 0x20

extern int sseAvailable;

#define _INC_MALLOC
#include <xmmintrin.h>
#undef _INC_MALLOC

// HYPNOS_AKI - reworked
#if 0
typedef union
{
	__m128 m;
	Vec3 v;
} _sseVec3;

typedef union
{
	__m128 m;
	Vec4 v;
} _sseVec4;

#define sseVec3 __declspec(align(16)) _sseVec3
#define sseVec4 __declspec(align(16)) _sseVec4

#define sse_init4(vec, f1, f2, f3, f4) {(vec).v[0] = f1; (vec).v[1] = f2; (vec).v[2] = f3; (vec).v[3] = f4;}

#define declSSEop(op,num) \
__forceinline void sse_ ## op ## num (_sseVec ## num *a, _sseVec ## num *b, _sseVec ## num *res) \
{ \
	res->m = _mm_ ## op ## _ps((a)->m, (b)->m); \
}

declSSEop(min,3)
declSSEop(min,4)
declSSEop(max,3)
declSSEop(max,4)
declSSEop(add,3)
declSSEop(add,4)
declSSEop(sub,3)
declSSEop(sub,4)
declSSEop(mul,3)
declSSEop(mul,4)

#undef declSSEop
#else
#include <mmintrin.h>
#include <emmintrin.h>

#define sseVec4 __m128

#define sse_init4(vec, f1, f2, f3, f4) vec = _mm_set_ps(f4, f3, f2, f1)

#define sse_init4_all(vec, value) vec = _mm_set1_ps(value)

#define sse_load4(vec, ptr) vec = _mm_load_ps((float*)ptr)
#define sse_load4_unaligned(vec, ptr) vec = _mm_loadu_ps((float*)ptr)

#define sse_store4(vec, ptr) _mm_store_ps((float*)ptr, vec)
#define sse_store4_unaligned(vec, ptr) _mm_storeu_ps((float*)ptr, vec)

#define sse_min4(a, b, res) res = _mm_min_ps(a, b)
#define sse_max4(a, b, res) res = _mm_max_ps(a, b)
#define sse_add4(a, b, res) res = _mm_add_ps(a, b)
#define sse_sub4(a, b, res) res = _mm_sub_ps(a, b)
#define sse_mul4(a, b, res) res = _mm_mul_ps(a, b)

#endif

#endif//_SSEMATH_H_


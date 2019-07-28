#include "ssemath.h"

int sseAvailable = 0;

void determineIfSSEAvailable(void)
{
	int cpuid_regs[4];

	sseAvailable = 0;

	__cpuid(cpuid_regs, 1);

	if(cpuid_regs[3] & (1<<25)) {
		sseAvailable |= SSE_1;
	}

	if(cpuid_regs[3] & (1<<26)) {
		sseAvailable |= SSE_2;
	}

	if(cpuid_regs[2] & (1<<0)) {
		sseAvailable |= SSE_3;
	}

	if(cpuid_regs[2] & (1<<9)) {
		sseAvailable |= SSE_3_s;
	}

	if(cpuid_regs[2] & (1<<19)) {
		sseAvailable |= SSE_4_1;
	}

	if(cpuid_regs[2] & (1<<20)) {
		sseAvailable |= SSE_4_2;
	}
}

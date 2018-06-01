#pragma once
#include <stdint.h>

#if defined(__GNUC__)
	#include <cpuid.h>
#elif defined(_MSC_VER)
	#include <intrin.h>
#endif

#include "api.h"


#if defined(__GNUC__) || defined(_MSC_VER)
static inline struct cpuid_regs cpuid(uint32_t eax) {
	struct cpuid_regs regs;
	#if defined(__GNUC__)
		__cpuid(eax, regs.eax, regs.ebx, regs.ecx, regs.edx);
	#else
		int regs_array[4];
		__cpuid(regs_array, (int) eax);
		regs.eax = regs_array[0];
		regs.ebx = regs_array[1];
		regs.ecx = regs_array[2];
		regs.edx = regs_array[3];
	#endif
	return regs;
}

static inline struct cpuid_regs cpuidex(uint32_t eax, uint32_t ecx) {
	struct cpuid_regs regs;
	#if defined(__GNUC__)
		__cpuid_count(eax, ecx, regs.eax, regs.ebx, regs.ecx, regs.edx);
	#else
		int regs_array[4];
		__cpuidex(regs_array, (int) eax, (int) ecx);
		regs.eax = regs_array[0];
		regs.ebx = regs_array[1];
		regs.ecx = regs_array[2];
		regs.edx = regs_array[3];
	#endif
	return regs;
}
#endif

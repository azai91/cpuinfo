#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif


#include <cpuinfo.h>
#include "./api.h"

#ifdef __APPLE__
#include "TargetConditionals.h"
#endif


#ifdef _WIN32
static INIT_ONCE init_guard = INIT_ONCE_STATIC_INIT;
#else
static pthread_once_t init_guard = PTHREAD_ONCE_INIT;
#endif

#include <stdint.h>
#include <string.h>

#include <cpuinfo.h>
#include "cpuid.h"
#include "api.h"
#include "utils.h"
#include "log.h"



bool CPUINFO_ABI cpuinfo_initialize(void) {
#if CPUINFO_ARCH_X86 || CPUINFO_ARCH_X86_64
	#if defined(__MACH__) && defined(__APPLE__)
		pthread_once(&init_guard, &cpuinfo_x86_mach_init);
	#elif defined(__linux__)
		pthread_once(&init_guard, &cpuinfo_x86_linux_init);
	#elif defined(_WIN32)
		InitOnceExecuteOnce(&init_guard, &cpuinfo_x86_windows_init, NULL, NULL);
	#else
		cpuinfo_log_error("operating system is not supported in cpuinfo");
	#endif
#else
	cpuinfo_log_error("processor architecture is not supported in cpuinfo");
#endif
	return cpuinfo_is_initialized;
}

void CPUINFO_ABI cpuinfo_deinitialize(void) {
}



void cpuinfo_x86_init_processor(struct cpuinfo_x86_processor* processor) {
	const struct cpuid_regs leaf0 = cpuid(0);
	const uint32_t max_base_index = leaf0.eax;
//	const enum cpuinfo_vendor vendor = processor->vendor =
//																				 cpuinfo_x86_decode_vendor(leaf0.ebx, leaf0.ecx, leaf0.edx);

	const struct cpuid_regs leaf0x80000000 = cpuid(UINT32_C(0x80000000));
	const uint32_t max_extended_index =
			leaf0x80000000.eax >= UINT32_C(0x80000000) ? leaf0x80000000.eax : 0;

	const struct cpuid_regs leaf0x80000001 = max_extended_index >= UINT32_C(0x80000001) ?
																					 cpuid(UINT32_C(0x80000001)) : (struct cpuid_regs) { 0, 0, 0, 0 };

	if (max_base_index >= 1) {
		const struct cpuid_regs leaf1 = cpuid(1);
		processor->cpuid = leaf1.eax;

//		const struct cpuinfo_x86_model_info model_info = cpuinfo_x86_decode_model_info(leaf1.eax);

		/*
		 * Topology extensions support:
		 * - AMD: ecx[bit 22] in extended info (reserved bit on Intel CPUs).
		 */
//		const bool amd_topology_extensions = !!(leaf0x80000001.ecx & UINT32_C(0x00400000));

//		cpuinfo_x86_detect_cache(
//				max_base_index, max_extended_index, amd_topology_extensions, vendor, &model_info,
//				&processor->cache,
//				&processor->tlb.itlb_4KB,
//				&processor->tlb.itlb_2MB,
//				&processor->tlb.itlb_4MB,
//				&processor->tlb.dtlb0_4KB,
//				&processor->tlb.dtlb0_2MB,
//				&processor->tlb.dtlb0_4MB,
//				&processor->tlb.dtlb_4KB,
//				&processor->tlb.dtlb_2MB,
//				&processor->tlb.dtlb_4MB,
//				&processor->tlb.dtlb_1GB,
//				&processor->tlb.stlb2_4KB,
//				&processor->tlb.stlb2_2MB,
//				&processor->tlb.stlb2_1GB,
//				&processor->topology.core_bits_length);

		cpuinfo_x86_detect_topology(max_base_index, max_extended_index, leaf1, &processor->topology);

	}
	if (max_extended_index >= UINT32_C(0x80000004)) {
		struct cpuid_regs brand_string[3];
		for (uint32_t i = 0; i < 3; i++) {
			brand_string[i] = cpuid(UINT32_C(0x80000002) + i);
		}
		memcpy(processor->brand_string, brand_string, sizeof(processor->brand_string));
		cpuinfo_log_debug("raw CPUID brand string: \"%48s\"", processor->brand_string);
	}
}
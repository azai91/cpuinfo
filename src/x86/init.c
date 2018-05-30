#include <stdint.h>
#include <string.h>

#include <cpuinfo.h>
#include <x86/cpuid.h>
#include <x86/api.h>
#include <utils.h>
#include <log.h>



void cpuinfo_x86_init_processor(struct cpuinfo_x86_processor* processor) {
	const struct cpuid_regs leaf0 = cpuid(0);
	const uint32_t max_base_index = leaf0.eax;

	const struct cpuid_regs leaf0x80000000 = cpuid(UINT32_C(0x80000000));
	const uint32_t max_extended_index =
		leaf0x80000000.eax >= UINT32_C(0x80000000) ? leaf0x80000000.eax : 0;

	const struct cpuid_regs leaf0x80000001 = max_extended_index >= UINT32_C(0x80000001) ?
		cpuid(UINT32_C(0x80000001)) : (struct cpuid_regs) { 0, 0, 0, 0 };

	if (max_base_index >= 1) {
		const struct cpuid_regs leaf1 = cpuid(1);
		processor->cpuid = leaf1.eax;

		cpuinfo_x86_detect_topology(max_base_index, max_extended_index, leaf1, &processor->topology);

	}
}

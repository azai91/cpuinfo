#pragma once

#include <stdbool.h>
#include <stdint.h>

#include <cpuinfo.h>

#ifdef _WIN32
#include <windows.h>
#endif


struct cpuid_regs {
	uint32_t eax;
	uint32_t ebx;
	uint32_t ecx;
	uint32_t edx;
};


struct cpuinfo_x86_topology {
	uint32_t apic_id;
	uint32_t thread_bits_offset;
	uint32_t thread_bits_length;
	uint32_t core_bits_offset;
	uint32_t core_bits_length;
};

struct cpuinfo_x86_processor {
	uint32_t cpuid;
	struct cpuinfo_x86_topology topology;
};

void cpuinfo_x86_init_processor(struct cpuinfo_x86_processor* processor);

//enum cpuinfo_vendor cpuinfo_x86_decode_vendor(uint32_t ebx, uint32_t ecx, uint32_t edx);
//struct cpuinfo_x86_model_info cpuinfo_x86_decode_model_info(uint32_t eax);
//enum cpuinfo_uarch cpuinfo_x86_decode_uarch(
//	enum cpuinfo_vendor vendor,
//	const struct cpuinfo_x86_model_info* model_info);

//struct cpuinfo_x86_isa cpuinfo_x86_detect_isa(
//	const struct cpuid_regs basic_info, const struct cpuid_regs extended_info,
//	uint32_t max_base_index, uint32_t max_extended_index,
//	enum cpuinfo_vendor vendor, enum cpuinfo_uarch uarch);
//struct cpuinfo_x86_isa cpuinfo_x86_nacl_detect_isa(void);

void cpuinfo_x86_detect_topology(
	uint32_t max_base_index,
	uint32_t max_extended_index,
	struct cpuid_regs leaf1,
	struct cpuinfo_x86_topology* topology);


extern bool cpuinfo_is_initialized;
extern struct cpuinfo_processor* cpuinfo_processors;
extern struct cpuinfo_core* cpuinfo_cores;
extern uint32_t cpuinfo_cores_count;

void cpuinfo_x86_mach_init(void);
void cpuinfo_x86_linux_init(void);
#ifdef _WIN32
BOOL CALLBACK cpuinfo_x86_windows_init(PINIT_ONCE init_once, PVOID parameter, PVOID* context);
#endif
void cpuinfo_arm_mach_init(void);
void cpuinfo_arm_linux_init(void);

typedef void (*cpuinfo_processor_callback)(uint32_t);


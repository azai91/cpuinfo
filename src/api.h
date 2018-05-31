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

struct cpuinfo_x86_cache {
	uint32_t size;
	uint32_t associativity;
	uint32_t sets;
	uint32_t partitions;
	uint32_t line_size;
	uint32_t flags;
	uint32_t apic_bits;
};

struct cpuinfo_x86_caches {
	struct cpuinfo_trace_cache trace;
	struct cpuinfo_x86_cache l1i;
	struct cpuinfo_x86_cache l1d;
	struct cpuinfo_x86_cache l2;
	struct cpuinfo_x86_cache l3;
	struct cpuinfo_x86_cache l4;
	uint32_t prefetch_size;
};

struct cpuinfo_x86_model_info {
	uint32_t model;
	uint32_t family;

	uint32_t base_model;
	uint32_t base_family;
	uint32_t stepping;
	uint32_t extended_model;
	uint32_t extended_family;
	uint32_t processor_type;
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
	enum cpuinfo_vendor vendor;
	enum cpuinfo_uarch uarch;
#ifdef __linux__
	int linux_id;
#endif
	struct cpuinfo_x86_caches cache;
	struct {
		struct cpuinfo_tlb itlb_4KB;
		struct cpuinfo_tlb itlb_2MB;
		struct cpuinfo_tlb itlb_4MB;
		struct cpuinfo_tlb dtlb0_4KB;
		struct cpuinfo_tlb dtlb0_2MB;
		struct cpuinfo_tlb dtlb0_4MB;
		struct cpuinfo_tlb dtlb_4KB;
		struct cpuinfo_tlb dtlb_2MB;
		struct cpuinfo_tlb dtlb_4MB;
		struct cpuinfo_tlb dtlb_1GB;
		struct cpuinfo_tlb stlb2_4KB;
		struct cpuinfo_tlb stlb2_2MB;
		struct cpuinfo_tlb stlb2_1GB;
	} tlb;
	struct cpuinfo_x86_topology topology;
	char brand_string[CPUINFO_PACKAGE_NAME_MAX];
};

void cpuinfo_x86_init_processor(struct cpuinfo_x86_processor* processor);

//enum cpuinfo_vendor cpuinfo_x86_decode_vendor(uint32_t ebx, uint32_t ecx, uint32_t edx);
//struct cpuinfo_x86_model_info cpuinfo_x86_decode_model_info(uint32_t eax);
//enum cpuinfo_uarch cpuinfo_x86_decode_uarch(
//	enum cpuinfo_vendor vendor,
//	const struct cpuinfo_x86_model_info* model_info);

struct cpuinfo_x86_isa cpuinfo_x86_detect_isa(
	const struct cpuid_regs basic_info, const struct cpuid_regs extended_info,
	uint32_t max_base_index, uint32_t max_extended_index,
	enum cpuinfo_vendor vendor, enum cpuinfo_uarch uarch);
struct cpuinfo_x86_isa cpuinfo_x86_nacl_detect_isa(void);

void cpuinfo_x86_detect_topology(
	uint32_t max_base_index,
	uint32_t max_extended_index,
	struct cpuid_regs leaf1,
	struct cpuinfo_x86_topology* topology);


uint32_t cpuinfo_x86_normalize_brand_string(
	const char raw_name[48],
	char normalized_name[48]);

uint32_t cpuinfo_x86_format_package_name(
	enum cpuinfo_vendor vendor,
	const char normalized_brand_string[48],
	char package_name[CPUINFO_PACKAGE_NAME_MAX]);

enum cpuinfo_cache_level {
	cpuinfo_cache_level_1i  = 0,
	cpuinfo_cache_level_1d  = 1,
	cpuinfo_cache_level_2   = 2,
	cpuinfo_cache_level_3   = 3,
	cpuinfo_cache_level_4   = 4,
	cpuinfo_cache_level_max = 5,
};

extern bool cpuinfo_is_initialized;
extern struct cpuinfo_processor* cpuinfo_processors;
extern struct cpuinfo_core* cpuinfo_cores;
extern struct cpuinfo_cache* cpuinfo_cache[cpuinfo_cache_level_max];
extern uint32_t cpuinfo_cores_count;

void cpuinfo_x86_mach_init(void);
void cpuinfo_x86_linux_init(void);
#ifdef _WIN32
BOOL CALLBACK cpuinfo_x86_windows_init(PINIT_ONCE init_once, PVOID parameter, PVOID* context);
#endif
void cpuinfo_arm_mach_init(void);
void cpuinfo_arm_linux_init(void);

typedef void (*cpuinfo_processor_callback)(uint32_t);


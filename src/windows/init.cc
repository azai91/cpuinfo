#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include <cpuinfo.h>
#include "../api.h"
#include "./api.h"
#include "../log.h"

#include <windows.h>

static inline uint32_t bit_mask(uint32_t bits) {
	return (UINT32_C(1) << bits) - UINT32_C(1);
}

static inline uint32_t low_index_from_kaffinity(KAFFINITY kaffinity) {
	#if defined(_M_X64) || defined(_M_AMD64)
		unsigned long index;
		_BitScanForward64(&index, (unsigned __int64) kaffinity);
		return (uint32_t) index;
	#elif defined(_M_IX86)
		unsigned long index;
		_BitScanForward(&index, (unsigned long) kaffinity);
		return (uint32_t) index;
	#else
		#error Platform-specific implementation required
	#endif
}

BOOL CALLBACK cpuinfo_x86_windows_init(PINIT_ONCE init_once, PVOID parameter, PVOID* context) {
	struct cpuinfo_processor* processors = NULL;
//	struct cpuinfo_core* cores = NULL;
//	struct cpuinfo_cluster* clusters = NULL;
//	struct cpuinfo_package* packages = NULL;
//	struct cpuinfo_cache* l1i = NULL;
//	struct cpuinfo_cache* l1d = NULL;
//	struct cpuinfo_cache* l2 = NULL;
//	struct cpuinfo_cache* l3 = NULL;
//	struct cpuinfo_cache* l4 = NULL;
	PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX processor_infos = NULL;

	HANDLE heap = GetProcessHeap();

	struct cpuinfo_x86_processor x86_processor;
	ZeroMemory(&x86_processor, sizeof(x86_processor));
	cpuinfo_x86_init_processor(&x86_processor);
	char brand_string[48];
	cpuinfo_x86_normalize_brand_string(x86_processor.brand_string, brand_string);

//	const uint32_t thread_bits_mask = bit_mask(x86_processor.topology.thread_bits_length);
//	const uint32_t core_bits_mask   = bit_mask(x86_processor.topology.core_bits_length);
//	const uint32_t package_bits_offset = max(
//		x86_processor.topology.thread_bits_offset + x86_processor.topology.thread_bits_length,
//		x86_processor.topology.core_bits_offset + x86_processor.topology.core_bits_length);

	const uint32_t max_group_count = (uint32_t) GetMaximumProcessorGroupCount();
	cpuinfo_log_debug("detected % processor groups", max_group_count);

	uint32_t processors_count = 0;
	uint32_t* processors_per_group = (uint32_t*) _alloca(max_group_count * sizeof(uint32_t));
	for (uint32_t i = 0; i < max_group_count; i++) {
		processors_per_group[i] = GetMaximumProcessorCount((WORD) i);
		cpuinfo_log_debug("detected % processors in group %",
			processors_per_group[i], i);
		processors_count += processors_per_group[i];
	}

	uint32_t* processors_before_group = (uint32_t*) _alloca(max_group_count * sizeof(uint32_t));
	for (uint32_t i = 0, count = 0; i < max_group_count; i++) {
		processors_before_group[i] = count;
		cpuinfo_log_debug("detected % processors before group %",
			processors_before_group[i], i);
		count += processors_per_group[i];
	}

	processors = HeapAlloc(heap, HEAP_ZERO_MEMORY, processors_count * sizeof(struct cpuinfo_processor));
	if (processors == NULL) {
		cpuinfo_log_error("failed to allocate %zu bytes for descriptions of % logical processors",
			processors_count * sizeof(struct cpuinfo_processor), processors_count);
		goto cleanup;
	}

	DWORD cores_info_size = 0;
	if (GetLogicalProcessorInformationEx(RelationProcessorCore, NULL, &cores_info_size) == FALSE) {
		const DWORD last_error = GetLastError();
		if (last_error != ERROR_INSUFFICIENT_BUFFER) {
			cpuinfo_log_error("failed to query size of processor cores information: error %",
				(uint32_t) last_error);
			goto cleanup;
		}
	}

	DWORD packages_info_size = 0;
	if (GetLogicalProcessorInformationEx(RelationProcessorPackage, NULL, &packages_info_size) == FALSE) {
		const DWORD last_error = GetLastError();
		if (last_error != ERROR_INSUFFICIENT_BUFFER) {
			cpuinfo_log_error("failed to query size of processor packages information: error %",
				(uint32_t) last_error);
			goto cleanup;
		}
	}

	DWORD max_info_size = max(cores_info_size, packages_info_size);

	processor_infos = HeapAlloc(heap, 0, max_info_size);
	if (processor_infos == NULL) {
		cpuinfo_log_error("failed to allocate % bytes for logical processor information",
			(uint32_t) max_info_size);
		goto cleanup;
	}

	if (GetLogicalProcessorInformationEx(RelationProcessorPackage, processor_infos, &max_info_size) == FALSE) {
		cpuinfo_log_error("failed to query processor packages information: error %",
			(uint32_t) GetLastError());
		goto cleanup;
	}

//	uint32_t packages_count = 0;
//	PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX packages_info_end =
//		(PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX) ((uintptr_t) processor_infos + packages_info_size);
//	for (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX package_info = processor_infos;
//		package_info < packages_info_end;
//		package_info = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX) ((uintptr_t) package_info + package_info->Size))
//	{
//		if (package_info->Relationship != RelationProcessorPackage) {
//			cpuinfo_log_warning("unexpected processor info type (%) for processor package information",
//				(uint32_t) package_info->Relationship);
//			continue;
//		}
//
//		/* We assume that packages are reported in APIC order */
//		const uint32_t package_id = packages_count++;
//		/* Reconstruct package part of APIC ID */
//		const uint32_t package_apic_id = package_id << package_bits_offset;
//		/* Iterate processor groups and set the package part of APIC ID */
//		for (uint32_t i = 0; i < package_info->Processor.GroupCount; i++) {
//			const uint32_t group_id = package_info->Processor.GroupMask[i].Group;
//			/* Global index of the first logical processor belonging to this group */
//			const uint32_t group_processors_start = processors_before_group[group_id];
//			/* Bitmask representing processors in this group belonging to this package */
//			KAFFINITY group_processors_mask = package_info->Processor.GroupMask[i].Mask;
//			while (group_processors_mask != 0) {
//				const uint32_t group_processor_id = low_index_from_kaffinity(group_processors_mask);
//				const uint32_t processor_id = group_processors_start + group_processor_id;
//				processors[processor_id].package = (const struct cpuinfo_package*) NULL + package_id;
//				processors[processor_id].windows_group_id = (uint16_t) group_id;
//				processors[processor_id].windows_processor_id = (uint16_t) group_processor_id;
//				processors[processor_id].apic_id = package_apic_id;
//
//				/* Reset the lowest bit in affinity mask */
//				group_processors_mask &= (group_processors_mask - 1);
//			}
//		}
//	}

	uint32_t cores_count = 0;
	/* Index (among all cores) of the the first core on the current package */
	uint32_t package_core_start = 0;
	uint32_t current_package_apic_id = 0;
	PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX cores_info_end =
		(PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX) ((uintptr_t) processor_infos + cores_info_size);
	for (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX core_info = processor_infos;
		core_info < cores_info_end;
		core_info = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX) ((uintptr_t) core_info + core_info->Size))
	{
		if (core_info->Relationship != RelationProcessorCore) {
			cpuinfo_log_warning("unexpected processor info type (%) for processor core information",
				(uint32_t) core_info->Relationship);
			continue;
		}

		/* We assume that cores and logical processors are reported in APIC order */
		const uint32_t core_id = cores_count++;

	}

	cpuinfo_cores_count = cores_count;


	MemoryBarrier();

	cpuinfo_is_initialized = true;

	return TRUE;
}

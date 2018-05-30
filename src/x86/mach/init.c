#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <cpuinfo.h>
#include <x86/api.h>
#include <mach/api.h>
#include <api.h>
#include <log.h>


static inline uint32_t max(uint32_t a, uint32_t b) {
	return a > b ? a : b;
}

static inline uint32_t bit_mask(uint32_t bits) {
	return (UINT32_C(1) << bits) - UINT32_C(1);
}

void cpuinfo_x86_mach_init(void) {

	struct cpuinfo_core* cores = NULL;
	struct cpuinfo_mach_topology mach_topology = cpuinfo_mach_detect_topology();
	cores = calloc(mach_topology.cores, sizeof(struct cpuinfo_core));
	if (cores == NULL) {
		cpuinfo_log_error("failed to allocate %zu bytes for descriptions of %"PRIu32" cores",
			mach_topology.cores * sizeof(struct cpuinfo_core), mach_topology.cores);
		goto cleanup;
	}
	cpuinfo_cores_count = mach_topology.cores;
	__sync_synchronize();
	cpuinfo_is_initialized = true;
	cores = NULL;

cleanup:
	free(cores);
}

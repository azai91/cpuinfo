#include <stddef.h>

#include <cpuinfo.h>
#include "api.h"

uint32_t cpuinfo_cores_count = 0;

uint32_t cpuinfo_get_cores_count(void) {
  return cpuinfo_cores_count;
}

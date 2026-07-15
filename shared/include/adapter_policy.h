#ifndef _WSL_SHARED_INC_ADAPTER_POLICY_H_
#define _WSL_SHARED_INC_ADAPTER_POLICY_H_

#include <cstdint>
#include <cstdio>
#include <strings.h>

namespace wsl {
namespace thunk {
namespace adapter_policy {

struct AdapterInfoFallback {
  uint32_t device_id;
  int major;
  int minor;
  int stepping;
  uint32_t compute_unit_count;
};

// Known WSL adapter parse gaps. These defaults only backfill zero-valued
// metadata for adapters that the user explicitly opted into. The stored
// major/minor/stepping is each chip's true gfx ISA; the reported ISA the
// runtime actually uses remains user-controlled through
// HSA_OVERRIDE_GFX_VERSION.
//
// This table is limited to the RDNA2 device IDs exercised by the downstream
// fork. Some PCI IDs cover multiple products, so the CU count is necessarily a
// conservative/default choice when the Windows adapter query returns zero.
inline constexpr AdapterInfoFallback kKnownAdapterInfoFallbacks[] = {
    // Navi 21 (gfx1030)
    {0x73BF, 10, 3, 0, 72},  // RX 6800/6800 XT/early 6900 XT; default 6800 XT
    {0x73AF, 10, 3, 0, 80},  // RX 6900 XT
    {0x73A5, 10, 3, 0, 80},  // RX 6950 XT
    // Navi 22 (gfx1031): RX 6700 family and RX 6800M family
    {0x73DF, 10, 3, 1, 40},
    // Navi 23 (gfx1032)
    {0x73E3, 10, 3, 2, 28},  // Radeon PRO W6600
    {0x73EF, 10, 3, 2, 32},  // RX 6650 XT / RX 6700S / RX 6800S
    {0x73FF, 10, 3, 2, 28},  // RX 6600/6600 XT/6600M; default RX 6600
    // Navi 24 (gfx1034)
    {0x743F, 10, 3, 4, 16},  // RX 6400/6500 XT/6500M; default 6500 XT/M
    {0x7424, 10, 3, 4, 12},  // RX 6300
};

inline const AdapterInfoFallback *FindAdapterInfoFallback(uint32_t device_id) {
  for (const auto &fallback : kKnownAdapterInfoFallbacks) {
    if (fallback.device_id == device_id)
      return &fallback;
  }
  return nullptr;
}

inline bool ParseGfxOverrideValue(const char *value, uint32_t *major_out,
                                  uint32_t *minor_out,
                                  uint32_t *stepping_out) {
  if (!value || !value[0])
    return false;

  char dummy = '\0';
  uint32_t major = 0, minor = 0, stepping = 0;
  if (std::sscanf(value, "%u.%u.%u%c", &major, &minor, &stepping, &dummy) !=
          3 ||
      major > 63 || minor > 255 || stepping > 255)
    return false;

  if (major_out)
    *major_out = major;
  if (minor_out)
    *minor_out = minor;
  if (stepping_out)
    *stepping_out = stepping;
  return true;
}

inline bool HasValidGfxOverrideValue(const char *value) {
  return ParseGfxOverrideValue(value, nullptr, nullptr, nullptr);
}

inline bool IsEnabledValue(const char *value) {
  if (!value || !value[0])
    return false;

  return !strcasecmp(value, "1") || !strcasecmp(value, "true") ||
         !strcasecmp(value, "yes") || !strcasecmp(value, "on");
}

inline bool ShouldAllowUnsupportedAdapter(uint32_t vendor_id,
                                          uint32_t device_id,
                                          bool has_gfx_override,
                                          bool enable_unsupported_adapters) {
  if (vendor_id != 0x1002 || !FindAdapterInfoFallback(device_id))
    return false;
  return has_gfx_override || enable_unsupported_adapters;
}

template <typename T>
inline T BackfillIfZero(T parsed, T fallback) {
  return parsed == 0 ? fallback : parsed;
}

} // namespace adapter_policy
} // namespace thunk
} // namespace wsl

#endif

#include <cstdio>
#include <cstdlib>
#include <iterator>

#include "shared/include/adapter_policy.h"
#include "shared/include/utils.h"

namespace {

void Check(bool condition, const char *message) {
  if (!condition) {
    std::fprintf(stderr, "%s\n", message);
    std::exit(1);
  }
}

} // namespace

int main() {
  using namespace wsl::thunk::adapter_policy;

  struct ExpectedFallback {
    uint32_t device_id;
    int major;
    int minor;
    int stepping;
    uint32_t compute_unit_count;
  };
  constexpr ExpectedFallback expected[] = {
      {0x73BF, 10, 3, 0, 72}, {0x73AF, 10, 3, 0, 80},
      {0x73A5, 10, 3, 0, 80}, {0x73DF, 10, 3, 1, 40},
      {0x73E3, 10, 3, 2, 28}, {0x73EF, 10, 3, 2, 32},
      {0x73FF, 10, 3, 2, 28}, {0x743F, 10, 3, 4, 16},
      {0x7424, 10, 3, 4, 12},
  };

  for (const auto &entry : expected) {
    const auto *actual = FindAdapterInfoFallback(entry.device_id);
    Check(actual != nullptr, "missing known RDNA2 adapter fallback");
    Check(actual->major == entry.major && actual->minor == entry.minor &&
              actual->stepping == entry.stepping,
          "incorrect RDNA2 adapter gfx ISA");
    Check(actual->compute_unit_count == entry.compute_unit_count,
          "incorrect RDNA2 adapter CU count");
  }
  Check(FindAdapterInfoFallback(0xFFFF) == nullptr,
        "unknown adapter unexpectedly has a fallback");

  for (size_t i = 0; i < std::size(kKnownAdapterInfoFallbacks); ++i) {
    for (size_t j = i + 1; j < std::size(kKnownAdapterInfoFallbacks); ++j) {
      Check(kKnownAdapterInfoFallbacks[i].device_id !=
                kKnownAdapterInfoFallbacks[j].device_id,
            "duplicate RDNA2 adapter device ID");
    }
  }

  uint32_t major = 0, minor = 0, stepping = 0;
  Check(ParseGfxOverrideValue("10.3.0", &major, &minor, &stepping),
        "valid gfx override rejected");
  Check(major == 10 && minor == 3 && stepping == 0,
        "gfx override parsed incorrectly");
  Check(!HasValidGfxOverrideValue("10.3"),
        "short gfx override accepted");
  Check(!HasValidGfxOverrideValue("64.3.0"),
        "out-of-range gfx override accepted");

  Check(IsEnabledValue("1") && IsEnabledValue("YES") &&
            IsEnabledValue("true"),
        "enabled environment value rejected");
  Check(!IsEnabledValue("0") && !IsEnabledValue(nullptr),
        "disabled environment value accepted");

  Check(ShouldAllowUnsupportedAdapter(0x1002, 0x73EF, true, false),
        "override did not admit known AMD adapter");
  Check(ShouldAllowUnsupportedAdapter(0x1002, 0x73EF, false, true),
        "explicit opt-in did not admit known AMD adapter");
  Check(!ShouldAllowUnsupportedAdapter(0x1002, 0xFFFF, true, true),
        "unknown AMD adapter was admitted");
  Check(!ShouldAllowUnsupportedAdapter(0x10DE, 0x73EF, true, true),
        "non-AMD adapter was admitted");
  Check(!ShouldAllowUnsupportedAdapter(0x1002, 0x73EF, false, false),
        "known adapter was admitted without opt-in");

  Check(BackfillIfZero(0, 10) == 10,
        "zero adapter metadata was not backfilled");
  Check(BackfillIfZero(9, 10) == 9,
        "nonzero adapter metadata was overwritten");

  // Platform admission is not sufficient after the shared-device refactor:
  // thunk_proxy also resolves the device through LookupGfxipEntry.
  setenv("HSA_OVERRIDE_GFX_VERSION", "10.3.0", 1);
  wsl::thunk::GfxipTable gfxip{};
  Check(wsl::thunk::LookupGfxipEntry(0x73DF, &gfxip),
        "opted-in RDNA2 adapter missing from shared gfx lookup");
  Check(gfxip.major == 10 && gfxip.minor == 3 && gfxip.stepping == 1,
        "shared RDNA2 gfx lookup returned the wrong ISA");
  return 0;
}

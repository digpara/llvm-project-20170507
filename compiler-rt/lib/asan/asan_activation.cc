//===-- asan_activation.cc --------------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file is a part of AddressSanitizer, an address sanity checker.
//
// ASan activation/deactivation logic.
//===----------------------------------------------------------------------===//

#include "asan_activation.h"
#include "asan_allocator.h"
#include "asan_flags.h"
#include "asan_internal.h"
#include "asan_poisoning.h"
#include "asan_stack.h"
#include "sanitizer_common/sanitizer_flags.h"

namespace __asan {

static struct AsanDeactivatedFlags {
  AllocatorOptions allocator_options;
  int malloc_context_size;
  bool poison_heap;
  bool coverage;
  const char *coverage_dir;

  void OverrideFromActivationFlags() {
    Flags f;
    CommonFlags cf;

    // Copy the current activation flags.
    allocator_options.CopyTo(&f, &cf);
    cf.malloc_context_size = malloc_context_size;
    f.poison_heap = poison_heap;

    // Check if activation flags need to be overriden.
    // FIXME: Add diagnostic to check that activation flags string doesn't
    // contain any other flags.
    if (const char *env = GetEnv("ASAN_ACTIVATION_OPTIONS")) {
      cf.ParseFromString(env);
      ParseFlagsFromString(&f, env);
    }

    // Override from getprop asan.options.
    char buf[100];
    GetExtraActivationFlags(buf, sizeof(buf));
    cf.ParseFromString(buf);
    ParseFlagsFromString(&f, buf);

    allocator_options.SetFrom(&f, &cf);
    malloc_context_size = cf.malloc_context_size;
    poison_heap = f.poison_heap;
    coverage = cf.coverage;
    coverage_dir = cf.coverage_dir;
  }

  void Print() {
    Report(
        "quarantine_size_mb %d, max_redzone %d, poison_heap %d, "
        "malloc_context_size %d, alloc_dealloc_mismatch %d, "
        "allocator_may_return_null %d, coverage %d, coverage_dir %s\n",
        allocator_options.quarantine_size_mb, allocator_options.max_redzone,
        poison_heap, malloc_context_size,
        allocator_options.alloc_dealloc_mismatch,
        allocator_options.may_return_null, coverage, coverage_dir);
  }
} asan_deactivated_flags;

static bool asan_is_deactivated;

void AsanDeactivate() {
  CHECK(!asan_is_deactivated);
  VReport(1, "Deactivating ASan\n");

  // Stash runtime state.
  GetAllocatorOptions(&asan_deactivated_flags.allocator_options);
  asan_deactivated_flags.malloc_context_size = GetMallocContextSize();
  asan_deactivated_flags.poison_heap = CanPoisonMemory();
  asan_deactivated_flags.coverage = common_flags()->coverage;
  asan_deactivated_flags.coverage_dir = common_flags()->coverage_dir;

  // Deactivate the runtime.
  SetCanPoisonMemory(false);
  SetMallocContextSize(1);
  ReInitializeCoverage(false, nullptr);

  AllocatorOptions disabled = asan_deactivated_flags.allocator_options;
  disabled.quarantine_size_mb = 0;
  disabled.min_redzone = 16;  // Redzone must be at least 16 bytes long.
  disabled.max_redzone = 16;
  disabled.alloc_dealloc_mismatch = false;
  disabled.may_return_null = true;
  ReInitializeAllocator(disabled);

  asan_is_deactivated = true;
}

void AsanActivate() {
  if (!asan_is_deactivated) return;
  VReport(1, "Activating ASan\n");

  asan_deactivated_flags.OverrideFromActivationFlags();

  SetCanPoisonMemory(asan_deactivated_flags.poison_heap);
  SetMallocContextSize(asan_deactivated_flags.malloc_context_size);
  ReInitializeCoverage(asan_deactivated_flags.coverage,
                       asan_deactivated_flags.coverage_dir);
  ReInitializeAllocator(asan_deactivated_flags.allocator_options);

  asan_is_deactivated = false;
  if (common_flags()->verbosity) {
    Report("Activated with flags:\n");
    asan_deactivated_flags.Print();
  }
}

}  // namespace __asan

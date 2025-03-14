// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_VIZ_COMMON_RESOURCES_PEAK_GPU_MEMORY_CALLBACK_H_
#define COMPONENTS_VIZ_COMMON_RESOURCES_PEAK_GPU_MEMORY_CALLBACK_H_

#include "base/containers/flat_map.h"
#include "base/functional/callback_forward.h"
#include "components/viz/common/resources/peak_gpu_memory_tracker.h"
#include "components/viz/common/viz_common_export.h"
#include "gpu/ipc/common/gpu_peak_memory.h"

namespace viz {

// Callback provided to the GpuService, which will be notified of the
// |peak_memory| used since GpuService started tracking GPU memory.
//
// This callback reports the peak memory usage in UMA Histograms, categorizing
// it by the provided |usage| type.
//
// Parameters:
// - |usage|: Indicates the category of GPU memory usage being tracked.
// - |testing_callback|: (Optional) Closure used by some tests to synchronize
//                       with the work done here on the UI thread.
// - |peak_memory|: The total peak GPU memory usage in bytes.
// - |allocation_per_source|: A breakdown of the peak memory usage, showing how
//                            much was allocated by each source.
void VIZ_COMMON_EXPORT PeakGpuMemoryCallback(
    PeakGpuMemoryTracker::Usage usage,
    base::OnceClosure testing_callback,
    const uint64_t peak_memory,
    const base::flat_map<gpu::GpuPeakMemoryAllocationSource, uint64_t>&
        allocation_per_source);
}  // namespace viz

#endif  // COMPONENTS_VIZ_COMMON_RESOURCES_PEAK_GPU_MEMORY_CALLBACK_H_

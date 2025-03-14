// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/browser/preloading/preload_pipeline_info.h"

namespace content {

PreloadPipelineInfo::PreloadPipelineInfo(
    PreloadingType planned_max_preloading_type)
    : id_(base::UnguessableToken::Create()),
      planned_max_preloading_type_(planned_max_preloading_type) {}

PreloadPipelineInfo::~PreloadPipelineInfo() = default;

void PreloadPipelineInfo::SetPrefetchEligibility(
    PreloadingEligibility eligibility) {
  prefetch_eligibility_ = eligibility;
}

void PreloadPipelineInfo::SetPrefetchStatus(PrefetchStatus prefetch_status) {
  prefetch_status_ = prefetch_status;
}

}  // namespace content

// Copyright 2015 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_PROFILE_METRICS_COUNTS_H_
#define COMPONENTS_PROFILE_METRICS_COUNTS_H_

#include "base/metrics/histogram_base.h"

namespace profile_metrics {

struct Counts {
  base::HistogramBase::Sample32 total = 0;
  base::HistogramBase::Sample32 signedin = 0;
  base::HistogramBase::Sample32 supervised = 0;
  base::HistogramBase::Sample32 active = 0;
  base::HistogramBase::Sample32 unused = 0;
};

// Logs metrics related to |counts|.
void LogProfileMetricsCounts(const Counts& counts);

}  // namespace profile_metrics

#endif  // COMPONENTS_PROFILE_METRICS_COUNTS_H_

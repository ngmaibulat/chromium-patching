// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/public/common/interest_group/ad_auction_constants.h"

#include <algorithm>

#include "base/feature_list.h"
#include "base/time/time.h"
#include "third_party/blink/public/common/features.h"

namespace blink {

size_t MaxAdAuctionAdComponents() {
  if (base::FeatureList::IsEnabled(
          features::kFledgeCustomMaxAuctionAdComponents)) {
    size_t custom_limit =
        features::kFledgeCustomMaxAuctionAdComponentsValue.Get();
    custom_limit = std::min(custom_limit, kMaxAdAuctionAdComponentsConfigLimit);
    return custom_limit;
  } else {
    return kMaxAdAuctionAdComponentsDefault;
  }
}

base::TimeDelta MaxInterestGroupLifetime() {
  return base::FeatureList::IsEnabled(features::kFledgeMaxGroupLifetimeFeature)
             ? features::kFledgeMaxGroupLifetime.Get()
             : base::Days(30);
}

base::TimeDelta MaxInterestGroupLifetimeForMetadata() {
  return base::FeatureList::IsEnabled(features::kFledgeMaxGroupLifetimeFeature)
             ? features::kFledgeMaxGroupLifetimeForMetadata.Get()
             : base::Days(30);
}

}  // namespace blink

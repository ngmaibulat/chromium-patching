// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/bookmarks/common/bookmark_features.h"

#include "base/feature_list.h"

namespace bookmarks {

// This feature controls the default visibility for permanent folders when
// empty. It effectively swaps in "other bookmarks" as the default-visible
// empty folder on mobile. This flag has no effect for desktop.
BASE_FEATURE(kAllBookmarksBaselineFolderVisibility,
             "AllBookmarksBaselineFolderVisibility",
             base::FEATURE_DISABLED_BY_DEFAULT);

// Determines which container is used internally in `TitledUrlIndex`,
// leading to slightly different performance characteristics. If enabled,
// a std::set is used per indexed term instead of base::flat_set.
BASE_FEATURE(kBookmarksUseBinaryTreeInTitledUrlIndex,
             "BookmarksUseBinaryTreeInTitledUrlIndex",
             base::FEATURE_DISABLED_BY_DEFAULT);

}  // namespace bookmarks

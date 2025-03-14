// Copyright 2015 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/renderer/platform/loader/fetch/integrity_metadata.h"

namespace blink {

IntegrityMetadata::IntegrityMetadata(WTF::String digest,
                                     IntegrityAlgorithm algorithm)
    : digest_(digest), algorithm_(algorithm) {}

IntegrityMetadata::IntegrityMetadata(IntegrityMetadataPair pair)
    : digest_(pair.first), algorithm_(pair.second) {}

IntegrityMetadataPair IntegrityMetadata::ToPair() const {
  return IntegrityMetadataPair(digest_, algorithm_);
}

}  // namespace blink

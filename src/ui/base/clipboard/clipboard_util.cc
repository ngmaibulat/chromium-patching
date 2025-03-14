// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/base/clipboard/clipboard_util.h"

#include <string_view>
#include <vector>

#include "base/threading/thread_restrictions.h"
#include "third_party/skia/include/core/SkBitmap.h"
#include "ui/base/ui_base_features.h"
#include "ui/gfx/codec/png_codec.h"

namespace ui::clipboard_util {

namespace {

std::vector<uint8_t> EncodeBitmapToPngImpl(const SkBitmap& bitmap) {
  // Prefer faster image encoding, even if it results in a PNG with a worse
  // compression ratio.
  std::optional<std::vector<uint8_t>> data =
      gfx::PNGCodec::FastEncodeBGRASkBitmap(bitmap,
                                            /*discard_transparency=*/false);
  return data.value_or(std::vector<uint8_t>());
}

}  // namespace

std::vector<uint8_t> EncodeBitmapToPng(const SkBitmap& bitmap) {
  // Encoding a PNG can be a long CPU operation.
  base::AssertLongCPUWorkAllowed();

  return EncodeBitmapToPngImpl(bitmap);
}

std::vector<uint8_t> EncodeBitmapToPngAcceptJank(const SkBitmap& bitmap) {
  return EncodeBitmapToPngImpl(bitmap);
}

bool ShouldSkipBookmark(std::u16string_view title, std::string_view url) {
  return url.empty() ||
         (!base::FeatureList::IsEnabled(features::kWriteBookmarkWithoutTitle) &&
          title.empty());
}

}  // namespace ui::clipboard_util

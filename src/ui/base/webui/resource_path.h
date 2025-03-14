// Copyright 2021 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_BASE_WEBUI_RESOURCE_PATH_H_
#define UI_BASE_WEBUI_RESOURCE_PATH_H_

#include <optional>

namespace webui {
struct ResourcePath {
  const char* const path;
  int id;
  std::optional<const char* const> filepath;
};
}  // namespace webui

#endif  // UI_BASE_WEBUI_RESOURCE_PATH_H_

// Copyright 2021 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/reporting/storage/storage_uploader_interface.h"

#include <array>

namespace reporting {

UploaderInterface::UploaderInterface() = default;
UploaderInterface::~UploaderInterface() = default;

// static
std::string_view UploaderInterface::ReasonToString(UploadReason reason) {
  static std::array<const char*,
                    static_cast<uint32_t>(UploadReason::MAX_REASON)>
      reason_to_string = {
          "UNKNOWN",         "MANUAL",        "KEY_DELIVERY",     "PERIODIC",
          "IMMEDIATE_FLUSH", "FAILURE_RETRY", "INCOMPLETE_RETRY", "INIT_RESUME",
      };

  return reason < UploadReason::MAX_REASON
             ? reason_to_string[static_cast<uint32_t>(reason)]
             : "ILLEGAL";
}

}  // namespace reporting

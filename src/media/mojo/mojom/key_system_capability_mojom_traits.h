// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MEDIA_MOJO_MOJOM_KEY_SYSTEM_CAPABILITY_MOJOM_TRAITS_H_
#define MEDIA_MOJO_MOJOM_KEY_SYSTEM_CAPABILITY_MOJOM_TRAITS_H_

#include "media/base/cdm_capability.h"
#include "media/base/key_system_capability.h"
#include "media/mojo/mojom/key_system_support.mojom.h"

namespace mojo {

template <>
struct StructTraits<media::mojom::KeySystemCapabilityDataView,
                    media::KeySystemCapability> {
  static const std::optional<media::CdmCapability> sw_secure_capability(
      const media::KeySystemCapability& input) {
    return input.sw_cdm_capability_or_status.has_value()
               ? std::optional(input.sw_cdm_capability_or_status.value())
               : std::nullopt;
  }

  static const std::optional<media::CdmCapability> hw_secure_capability(
      const media::KeySystemCapability& input) {
    return input.hw_cdm_capability_or_status.has_value()
               ? std::optional(input.hw_cdm_capability_or_status.value())
               : std::nullopt;
  }

  static const std::optional<media::CdmCapabilityQueryStatus>
  sw_secure_capability_query_status(const media::KeySystemCapability& input) {
    return input.sw_cdm_capability_or_status.has_value()
               ? std::nullopt
               : std::optional(input.sw_cdm_capability_or_status.error());
  }

  static const std::optional<media::CdmCapabilityQueryStatus>
  hw_secure_capability_query_status(const media::KeySystemCapability& input) {
    return input.hw_cdm_capability_or_status.has_value()
               ? std::nullopt
               : std::optional(input.hw_cdm_capability_or_status.error());
  }

  static bool Read(media::mojom::KeySystemCapabilityDataView input,
                   media::KeySystemCapability* output);
};

}  // namespace mojo

#endif  // MEDIA_MOJO_MOJOM_KEY_SYSTEM_CAPABILITY_MOJOM_TRAITS_H_

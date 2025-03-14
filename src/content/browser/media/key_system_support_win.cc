// Copyright 2021 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/browser/media/key_system_support_win.h"

#include "base/logging.h"
#include "base/types/expected.h"
#include "content/browser/media/service_factory.h"
#include "media/base/cdm_capability.h"
#include "media/base/key_system_capability.h"
#include "media/cdm/win/media_foundation_cdm.h"
#include "media/mojo/mojom/key_system_support.mojom.h"
#include "media/mojo/mojom/media_foundation_service.mojom.h"
#include "mojo/public/cpp/bindings/callback_helpers.h"
#include "url/gurl.h"

namespace content {

namespace {

template <typename T>
base::flat_set<T> VectorToSet(const std::vector<T>& v) {
  return base::flat_set<T>(v.begin(), v.end());
}

void OnKeySystemCapability(
    bool is_hw_secure,
    media::CdmCapabilityCB cdm_capability_cb,
    bool is_supported,
    const media::KeySystemCapability& key_system_capability) {
  DVLOG(1) << __func__ << ": is_hw_secure=" << is_hw_secure;

  const media::CdmCapabilityOrStatus& cdm_capability_or_status =
      is_hw_secure ? key_system_capability.hw_cdm_capability_or_status
                   : key_system_capability.sw_cdm_capability_or_status;

  // Key system must support at least 1 video codec, 1 encryption scheme,
  // and 1 encryption scheme to be considered. Support for audio codecs is
  // optional.
  if (!is_supported || !cdm_capability_or_status.has_value()) {
    std::move(cdm_capability_cb).Run(cdm_capability_or_status);
    return;
  }

  std::optional<media::CdmCapability> capability =
      cdm_capability_or_status.value();
  if (!capability || capability->video_codecs.empty() ||
      capability->encryption_schemes.empty() ||
      capability->session_types.empty()) {
    std::move(cdm_capability_cb)
        .Run(base::unexpected(
            media::CdmCapabilityQueryStatus::kUnexpectedEmptyCapability));
    return;
  }

  std::move(cdm_capability_cb).Run(capability.value());
}

}  // namespace

void GetMediaFoundationServiceCdmCapability(
    const std::string& key_system,
    const media::CdmType& cdm_type,
    const base::FilePath& cdm_path,
    bool is_hw_secure,
    media::CdmCapabilityCB cdm_capability_cb) {
  DVLOG(3) << __func__ << ": key_system=" << key_system
           << ", is_hw_secure=" << is_hw_secure;
  if (!media::MediaFoundationCdm::IsAvailable()) {
    DVLOG(1) << "MediaFoundationCdm not available!";
    std::move(cdm_capability_cb)
        .Run(base::unexpected(
            media::CdmCapabilityQueryStatus::kMediaFoundationCdmNotSupported));
    return;
  }

  // CDM capability is global, use a generic BrowserContext and Site to query.
  auto& mf_service =
      GetMediaFoundationService(cdm_type, nullptr, GURL(), cdm_path);
  mf_service.IsKeySystemSupported(
      key_system,
      mojo::WrapCallbackWithDefaultInvokeIfNotRun(
          base::BindOnce(&OnKeySystemCapability, is_hw_secure,
                         std::move(cdm_capability_cb)),
          false,
          media::KeySystemCapability(
              base::unexpected(
                  media::CdmCapabilityQueryStatus::kDisconnectionError),
              base::unexpected(
                  media::CdmCapabilityQueryStatus::kDisconnectionError))));
}

}  // namespace content

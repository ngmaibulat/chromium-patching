// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_WEBAUTHN_LOCAL_CREDENTIAL_MANAGEMENT_MAC_H_
#define CHROME_BROWSER_WEBAUTHN_LOCAL_CREDENTIAL_MANAGEMENT_MAC_H_

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "base/containers/span.h"
#include "base/functional/callback_forward.h"
#include "chrome/browser/webauthn/local_credential_management.h"
#include "device/fido/discoverable_credential_metadata.h"
#include "device/fido/mac/authenticator_config.h"

class Profile;

// LocalCredentialManagementMac is the MacOS implementation of
// LocalCredentialManagement.
class LocalCredentialManagementMac : public LocalCredentialManagement {
 public:
  explicit LocalCredentialManagementMac(
      device::fido::mac::AuthenticatorConfig config);

  // LocalCredentialManagement:
  void HasCredentials(base::OnceCallback<void(bool)> callback) override;
  void Enumerate(
      base::OnceCallback<void(
          std::optional<std::vector<device::DiscoverableCredentialMetadata>>)>
          callback) override;
  void Delete(base::span<const uint8_t> credential_id,
              base::OnceCallback<void(bool)> callback) override;
  void Edit(base::span<uint8_t> credential_id,
            std::string new_username,
            base::OnceCallback<void(bool)> callback) override;

 private:
  device::fido::mac::AuthenticatorConfig config_;
};

#endif  // CHROME_BROWSER_WEBAUTHN_LOCAL_CREDENTIAL_MANAGEMENT_MAC_H_
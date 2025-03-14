// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chromeos/ash/experiences/arc/net/cert_manager.h"

#ifndef CHROMEOS_ASH_EXPERIENCES_ARC_TEST_FAKE_CERT_MANAGER_H_
#define CHROMEOS_ASH_EXPERIENCES_ARC_TEST_FAKE_CERT_MANAGER_H_

namespace arc {
// A fake CertManager that fakes method to import private key and cert.
class FakeCertManager : public CertManager {
 public:
  ~FakeCertManager() override = default;
  void ImportPrivateKeyAndCert(
      const std::string& key_pem,
      const std::string& cert_pem,
      ImportPrivateKeyAndCertCallback callback) override {}
};

}  // namespace arc

#endif  // CHROMEOS_ASH_EXPERIENCES_ARC_TEST_FAKE_CERT_MANAGER_H_

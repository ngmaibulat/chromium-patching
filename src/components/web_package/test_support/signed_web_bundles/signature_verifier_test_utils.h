// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_WEB_PACKAGE_TEST_SUPPORT_SIGNED_WEB_BUNDLES_SIGNATURE_VERIFIER_TEST_UTILS_H_
#define COMPONENTS_WEB_PACKAGE_TEST_SUPPORT_SIGNED_WEB_BUNDLES_SIGNATURE_VERIFIER_TEST_UTILS_H_

#include "base/functional/callback_forward.h"
#include "base/functional/callback_helpers.h"
#include "base/task/sequenced_task_runner.h"
#include "components/cbor/values.h"
#include "components/web_package/signed_web_bundles/signed_web_bundle_integrity_block.h"
#include "components/web_package/signed_web_bundles/signed_web_bundle_signature_verifier.h"
#include "components/web_package/signed_web_bundles/types.h"
#include "testing/gmock/include/gmock/gmock.h"

namespace web_package::test {

class MockSignatureVerifier : public SignedWebBundleSignatureVerifier {
 public:
  MockSignatureVerifier();
  ~MockSignatureVerifier() override;
  MOCK_METHOD(void,
              VerifySignatures,
              (base::File file,
               SignedWebBundleIntegrityBlock integrity_block,
               SignatureVerificationCallback callback),
              (const, override));
};

mojom::BundleIntegrityBlockSignatureStackEntryPtr MakeSignatureStackEntry(
    const PublicKey& public_key,
    base::span<const uint8_t> signature,
    base::span<const uint8_t> attributes_cbor);

SignedWebBundleIntegrityBlock ParseIntegrityBlockFromValue(
    const cbor::Value& integrity_block);

SignedWebBundleIntegrityBlock ParseIntegrityBlock(
    base::span<const uint8_t> swbn);

base::expected<void, SignedWebBundleSignatureVerifier::Error> VerifySignatures(
    const SignedWebBundleSignatureVerifier& signature_verifier,
    const base::File& file,
    const SignedWebBundleIntegrityBlock& integrity_block);

web_package::IntegrityBlockAttributes GetAttributesForSignedWebBundleId(
    const std::string& signed_web_bundle_id);
}  // namespace web_package::test

#endif  // COMPONENTS_WEB_PACKAGE_TEST_SUPPORT_SIGNED_WEB_BUNDLES_SIGNATURE_VERIFIER_TEST_UTILS_H_

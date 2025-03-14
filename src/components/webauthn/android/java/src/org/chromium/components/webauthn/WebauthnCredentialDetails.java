// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.components.webauthn;

import org.chromium.build.annotations.NullMarked;
import org.chromium.build.annotations.Nullable;

/** Describes a WebAuthn credential available on the authenticator. */
@NullMarked
public final class WebauthnCredentialDetails {
    /** Username associated with the credential. */
    public @Nullable String mUserName;

    /** Display name associated with the credential. */
    public @Nullable String mUserDisplayName;

    /** Unique identifier associated with the user account that the credential signs in to. */
    public byte @Nullable [] mUserId;

    /** Identifier for the credential itself. */
    public byte @Nullable [] mCredentialId;

    /** Whether the credential is discoverable. */
    public boolean mIsDiscoverable;

    /** Whether the credential is enabled for Secure Payment Confirmation. */
    public boolean mIsPayment;

    public WebauthnCredentialDetails() {}
}

// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.password_manager;

import android.content.Context;
import android.content.Intent;

import org.chromium.build.annotations.NullMarked;

/**
 * Functional interface to start a Chrome Custom Tab for the given intent, e.g. by using {@link
 * org.chromium.chrome.browser.LaunchIntentDispatcher#createCustomTabActivityIntent}.
 *
 * <p>TODO(crbug.com/40751023): Update when LaunchIntentDispatcher is (partially-)modularized.
 */
@NullMarked
public interface CustomTabIntentHelper {
    /**
     * @see org.chromium.chrome.browser.LaunchIntentDispatcher#createCustomTabActivityIntent
     */
    Intent createCustomTabActivityIntent(Context context, Intent intent);
}

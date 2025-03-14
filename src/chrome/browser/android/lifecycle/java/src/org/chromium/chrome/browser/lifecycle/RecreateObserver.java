// Copyright 2020 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.lifecycle;

import org.chromium.build.annotations.NullMarked;

/**
 * Implement this interface and register in {@link ActivityLifecycleDispatcher} to receive activity
 * recreate events.
 *
 */
@NullMarked
public interface RecreateObserver extends LifecycleObserver {
    /** Called when the activity is going to recreate. */
    void onRecreate();
}

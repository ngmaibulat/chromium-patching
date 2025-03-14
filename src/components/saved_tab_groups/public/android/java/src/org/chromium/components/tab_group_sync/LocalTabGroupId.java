// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.components.tab_group_sync;

import org.chromium.base.Token;
import org.chromium.build.annotations.NullMarked;
import org.chromium.build.annotations.Nullable;

import java.util.Objects;

/**
 * LocalTabGroupId is a convenient class to contain all the information needed to uniquely identify
 * a the local tab group.
 */
@NullMarked
public class LocalTabGroupId {
    // Stable ID of the tab group. This should be used going forward.
    public final Token tabGroupId;

    /**
     * Constructor.
     *
     * @param tabGroupId The stable ID of the tab group in {@link TabModel}.
     */
    public LocalTabGroupId(Token tabGroupId) {
        assert tabGroupId != null;
        this.tabGroupId = tabGroupId;
    }

    @Override
    public boolean equals(@Nullable Object o) {
        if (this == o) return true;
        if (!(o instanceof LocalTabGroupId)) return false;

        LocalTabGroupId other = (LocalTabGroupId) o;
        return Objects.equals(tabGroupId, other.tabGroupId);
    }

    @Override
    public int hashCode() {
        return Objects.hash(tabGroupId);
    }

    @Override
    public String toString() {
        return tabGroupId.toString();
    }
}

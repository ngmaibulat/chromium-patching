// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.components.segmentation_platform;

import org.chromium.build.annotations.NullMarked;
import org.chromium.build.annotations.Nullable;
import org.chromium.components.segmentation_platform.prediction_status.PredictionStatus;

import java.util.List;

@NullMarked
public class ClassificationResult {
    public final @PredictionStatus int status;

    public final @Nullable List<String> orderedLabels;

    public ClassificationResult(int status, String[] orderedLabels) {
        this.status = status;
        this.orderedLabels = orderedLabels == null ? null : List.of(orderedLabels);
    }
}

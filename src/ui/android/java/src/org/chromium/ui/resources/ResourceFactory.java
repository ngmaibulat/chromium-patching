// Copyright 2017 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.ui.resources;

import android.graphics.Rect;

import org.jni_zero.JNINamespace;
import org.jni_zero.NativeMethods;

import org.chromium.build.annotations.NullMarked;
import org.chromium.build.annotations.Nullable;
import org.chromium.ui.resources.statics.NinePatchData;

/** A utility class for creating native resources. */
@JNINamespace("ui")
@NullMarked
public class ResourceFactory {
    public static long createBitmapResource(@Nullable NinePatchData ninePatchData) {
        return ninePatchData == null
                ? ResourceFactoryJni.get().createBitmapResource()
                : createNinePatchBitmapResource(
                        ninePatchData.getPadding(), ninePatchData.getAperture());
    }

    private static long createNinePatchBitmapResource(Rect padding, Rect aperture) {
        return ResourceFactoryJni.get()
                .createNinePatchBitmapResource(
                        padding.left,
                        padding.top,
                        padding.right,
                        padding.bottom,
                        aperture.left,
                        aperture.top,
                        aperture.right,
                        aperture.bottom);
    }

    @NativeMethods
    public interface Natives {
        long createBitmapResource();

        long createNinePatchBitmapResource(
                int paddingLeft,
                int paddingTop,
                int paddingRight,
                int paddingBottom,
                int apertureLeft,
                int apertureTop,
                int apertureRight,
                int apertureBottom);
    }
}

// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.components.embedder_support.delegate;

import android.content.Context;
import android.util.AttributeSet;
import android.view.View;
import android.widget.GridView;

import org.chromium.build.annotations.NullMarked;

/** This class is to have the constructors for the suggestions view */
@NullMarked
public class ColorPickerSuggestionsView extends GridView {
    public ColorPickerSuggestionsView(Context context) {
        super(context);
    }

    public ColorPickerSuggestionsView(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    public ColorPickerSuggestionsView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
    }

    public ColorPickerSuggestionsView(
            Context context, AttributeSet attrs, int defStyleAttr, int defStyleRes) {
        super(context, attrs, defStyleAttr, defStyleRes);
    }

    @Override
    public void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        // GridView does not work well in a ScrollView when it uses WRAP_CONTENT.
        // Instead, force it to use AT_MOST.
        // https://stackoverflow.com/questions/4523609/grid-of-images-inside-scrollview
        int heightSpec;
        if (getLayoutParams().height == LayoutParams.WRAP_CONTENT) {
            heightSpec =
                    MeasureSpec.makeMeasureSpec(
                            Integer.MAX_VALUE & View.MEASURED_SIZE_MASK, MeasureSpec.AT_MOST);
        } else {
            heightSpec = heightMeasureSpec;
        }

        super.onMeasure(widthMeasureSpec, heightSpec);
    }
}

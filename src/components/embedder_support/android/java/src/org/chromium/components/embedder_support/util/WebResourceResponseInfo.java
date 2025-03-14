// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.components.embedder_support.util;

import org.jni_zero.CalledByNative;
import org.jni_zero.JNINamespace;

import org.chromium.build.annotations.NullMarked;
import org.chromium.build.annotations.Nullable;

import java.io.InputStream;
import java.util.Map;

/** The response information that is to be returned for a particular resource fetch. */
@JNINamespace("embedder_support")
@NullMarked
public class WebResourceResponseInfo {
    private String mMimeType;
    private String mCharset;
    private InputStream mData;
    private int mStatusCode;
    private @Nullable String mReasonPhrase;
    private @Nullable Map<String, String> mResponseHeaders;
    private String @Nullable [] mResponseHeaderNames;
    private String @Nullable [] mResponseHeaderValues;

    public WebResourceResponseInfo(String mimeType, String encoding, InputStream data) {
        mMimeType = mimeType;
        mCharset = encoding;
        mData = data;
    }

    public WebResourceResponseInfo(
            String mimeType,
            String encoding,
            InputStream data,
            int statusCode,
            String reasonPhrase,
            Map<String, String> responseHeaders) {
        this(mimeType, encoding, data);

        mStatusCode = statusCode;
        mReasonPhrase = reasonPhrase;
        mResponseHeaders = responseHeaders;
    }

    private void fillInResponseHeaderNamesAndValuesIfNeeded() {
        if (mResponseHeaders == null || mResponseHeaderNames != null) return;
        mResponseHeaderNames = new String[mResponseHeaders.size()];
        mResponseHeaderValues = new String[mResponseHeaders.size()];
        int i = 0;
        for (Map.Entry<String, String> entry : mResponseHeaders.entrySet()) {
            mResponseHeaderNames[i] = entry.getKey();
            mResponseHeaderValues[i] = entry.getValue();
            i++;
        }
    }

    @CalledByNative
    public String getMimeType() {
        return mMimeType;
    }

    @CalledByNative
    public String getCharset() {
        return mCharset;
    }

    @CalledByNative
    public InputStream getData() {
        return mData;
    }

    @CalledByNative
    public int getStatusCode() {
        return mStatusCode;
    }

    @CalledByNative
    public @Nullable String getReasonPhrase() {
        return mReasonPhrase;
    }

    public @Nullable Map<String, String> getResponseHeaders() {
        return mResponseHeaders;
    }

    @CalledByNative
    private String @Nullable [] getResponseHeaderNames() {
        fillInResponseHeaderNamesAndValuesIfNeeded();
        return mResponseHeaderNames;
    }

    @CalledByNative
    private String @Nullable [] getResponseHeaderValues() {
        fillInResponseHeaderNamesAndValuesIfNeeded();
        return mResponseHeaderValues;
    }
}

// Copyright 2021 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.robolectric.annotation.Config;

import org.chromium.base.test.BaseRobolectricTestRunner;

/** Tests for {@link ChromeLocalizationUtils}. */
@RunWith(BaseRobolectricTestRunner.class)
@Config(manifest = Config.NONE)
public class ChromeLocalizationUtilsTest {
    /** Test the return value for getUiAvailabilityStatus. */
    @Test
    public void testGetUiAvailabilityStatus() {
        // The boolean flags in order are: isOverridden, isTopAndroidLanguageAvailable,
        // isDefaultLanguage
        @ChromeLocalizationUtils.UiAvailableTypes
        int status = ChromeLocalizationUtils.getUiAvailabilityStatus(true, false, false);
        Assert.assertEquals(ChromeLocalizationUtils.UiAvailableTypes.OVERRIDDEN, status);

        status = ChromeLocalizationUtils.getUiAvailabilityStatus(true, false, true);
        Assert.assertEquals(ChromeLocalizationUtils.UiAvailableTypes.OVERRIDDEN, status);

        status = ChromeLocalizationUtils.getUiAvailabilityStatus(false, false, false);
        Assert.assertEquals(ChromeLocalizationUtils.UiAvailableTypes.NONE_AVAILABLE, status);

        status = ChromeLocalizationUtils.getUiAvailabilityStatus(false, false, true);
        Assert.assertEquals(
                ChromeLocalizationUtils.UiAvailableTypes.ONLY_DEFAULT_AVAILABLE, status);

        status = ChromeLocalizationUtils.getUiAvailabilityStatus(false, true, true);
        Assert.assertEquals(ChromeLocalizationUtils.UiAvailableTypes.TOP_AVAILABLE, status);

        // If top is available it shouldn't mater what if default is available.
        status = ChromeLocalizationUtils.getUiAvailabilityStatus(false, true, false);
        Assert.assertEquals(ChromeLocalizationUtils.UiAvailableTypes.TOP_AVAILABLE, status);
    }

    /** Test the return value for getUiCorrectnessStatus */
    @Test
    public void testGetUiCorrectnessStatus() {
        // The boolean flags in order are: noLanguageAvailable, isJavaUiCorrect, isNativeUiCorrect
        @ChromeLocalizationUtils.UiCorrectTypes
        int status = ChromeLocalizationUtils.getUiCorrectnessStatus(true, false, false);
        Assert.assertEquals(ChromeLocalizationUtils.UiCorrectTypes.NOT_AVAILABLE, status);

        status = ChromeLocalizationUtils.getUiCorrectnessStatus(true, true, false);
        Assert.assertEquals(ChromeLocalizationUtils.UiCorrectTypes.NOT_AVAILABLE, status);

        status = ChromeLocalizationUtils.getUiCorrectnessStatus(false, false, false);
        Assert.assertEquals(ChromeLocalizationUtils.UiCorrectTypes.INCORRECT, status);

        status = ChromeLocalizationUtils.getUiCorrectnessStatus(false, true, false);
        Assert.assertEquals(ChromeLocalizationUtils.UiCorrectTypes.ONLY_JAVA_CORRECT, status);

        status = ChromeLocalizationUtils.getUiCorrectnessStatus(false, true, true);
        Assert.assertEquals(ChromeLocalizationUtils.UiCorrectTypes.CORRECT, status);
    }

    /** Test the return value for getNoOverrideUiCorrectStatus */
    @Test
    public void testGetNoOverrideUiCorrectStatus() {
        // The boolean flags in order are: noLanguageAvailable, isCorrect
        @ChromeLocalizationUtils.UiCorrectTypes
        int status = ChromeLocalizationUtils.getNoOverrideUiCorrectStatus(true, false);
        Assert.assertEquals(ChromeLocalizationUtils.UiCorrectTypes.NOT_AVAILABLE, status);

        status = ChromeLocalizationUtils.getNoOverrideUiCorrectStatus(true, true);
        Assert.assertEquals(ChromeLocalizationUtils.UiCorrectTypes.NOT_AVAILABLE, status);

        status = ChromeLocalizationUtils.getNoOverrideUiCorrectStatus(false, true);
        Assert.assertEquals(ChromeLocalizationUtils.UiCorrectTypes.CORRECT, status);

        status = ChromeLocalizationUtils.getNoOverrideUiCorrectStatus(false, false);
        Assert.assertEquals(ChromeLocalizationUtils.UiCorrectTypes.INCORRECT, status);
    }

    /** Test the return value for getOverrideUiCorrectStatus */
    @Test
    public void testGetOverrideUiCorrectStatus() {
        // The boolean flags in order are: isJavaUiCorrect, isNativeUiCorrect
        @ChromeLocalizationUtils.UiCorrectTypes
        int status = ChromeLocalizationUtils.getOverrideUiCorrectStatus(true, true);
        Assert.assertEquals(ChromeLocalizationUtils.UiCorrectTypes.CORRECT, status);

        status = ChromeLocalizationUtils.getOverrideUiCorrectStatus(true, false);
        Assert.assertEquals(ChromeLocalizationUtils.UiCorrectTypes.ONLY_JAVA_CORRECT, status);

        status = ChromeLocalizationUtils.getOverrideUiCorrectStatus(false, false);
        Assert.assertEquals(ChromeLocalizationUtils.UiCorrectTypes.INCORRECT, status);

        status = ChromeLocalizationUtils.getOverrideUiCorrectStatus(false, true);
        Assert.assertEquals(ChromeLocalizationUtils.UiCorrectTypes.INCORRECT, status);
    }

    /** Test the return value for getLocaleUpdateStatus */
    @Test
    public void testGetLocaleUpdateStatus() {
        // First run checks
        @ChromeLocalizationUtils.LocaleUpdateStatus
        int status = ChromeLocalizationUtils.getLocaleUpdateStatus(null, "en-US,en,es", true);
        Assert.assertEquals(ChromeLocalizationUtils.LocaleUpdateStatus.FIRST_RUN, status);

        status = ChromeLocalizationUtils.getLocaleUpdateStatus("", "en-US,en,es", false);
        Assert.assertEquals(ChromeLocalizationUtils.LocaleUpdateStatus.FIRST_RUN, status);

        status = ChromeLocalizationUtils.getLocaleUpdateStatus("en-US,en", null, false);
        Assert.assertEquals(ChromeLocalizationUtils.LocaleUpdateStatus.FIRST_RUN, status);

        status = ChromeLocalizationUtils.getLocaleUpdateStatus("en-US,en", "", true);
        Assert.assertEquals(ChromeLocalizationUtils.LocaleUpdateStatus.FIRST_RUN, status);

        // Override true checks
        status = ChromeLocalizationUtils.getLocaleUpdateStatus("en-US,af,zu", "en-US,zu,af", true);
        Assert.assertEquals(
                ChromeLocalizationUtils.LocaleUpdateStatus.OVERRIDDEN_OTHERS_CHANGED, status);

        status = ChromeLocalizationUtils.getLocaleUpdateStatus("en-US,af,zu", "en-US", true);
        Assert.assertEquals(
                ChromeLocalizationUtils.LocaleUpdateStatus.OVERRIDDEN_OTHERS_CHANGED, status);

        status =
                ChromeLocalizationUtils.getLocaleUpdateStatus(
                        "af,en-US,af,zu", "en-US,af,zu", true);
        Assert.assertEquals(
                ChromeLocalizationUtils.LocaleUpdateStatus.OVERRIDDEN_TOP_CHANGED, status);

        status = ChromeLocalizationUtils.getLocaleUpdateStatus("af", "en", true);
        Assert.assertEquals(
                ChromeLocalizationUtils.LocaleUpdateStatus.OVERRIDDEN_TOP_CHANGED, status);

        status = ChromeLocalizationUtils.getLocaleUpdateStatus("af,en,zu", "af,en,zu", true);
        Assert.assertEquals(ChromeLocalizationUtils.LocaleUpdateStatus.NO_CHANGE, status);

        // Override false checks
        status = ChromeLocalizationUtils.getLocaleUpdateStatus("af,en,zu", "af,en,zu", false);
        Assert.assertEquals(ChromeLocalizationUtils.LocaleUpdateStatus.NO_CHANGE, status);

        status = ChromeLocalizationUtils.getLocaleUpdateStatus("af,en,zu", "af,zu", false);
        Assert.assertEquals(
                ChromeLocalizationUtils.LocaleUpdateStatus.NO_OVERRIDE_OTHERS_CHANGED, status);

        status = ChromeLocalizationUtils.getLocaleUpdateStatus("as,en,zu", "af,en,zu", false);
        Assert.assertEquals(
                ChromeLocalizationUtils.LocaleUpdateStatus.NO_OVERRIDE_TOP_CHANGED, status);
    }
}

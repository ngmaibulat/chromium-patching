// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.tasks.tab_management;

import static androidx.test.espresso.Espresso.closeSoftKeyboard;
import static androidx.test.espresso.Espresso.onView;
import static androidx.test.espresso.action.ViewActions.click;
import static androidx.test.espresso.matcher.ViewMatchers.withId;

import android.content.Context;

import androidx.test.platform.app.InstrumentationRegistry;

import org.chromium.chrome.browser.searchwidget.SearchActivity;
import org.chromium.chrome.test.ChromeTabbedActivityTestRule;
import org.chromium.chrome.test.R;
import org.chromium.chrome.test.transit.page.WebPageStation;
import org.chromium.chrome.test.util.ActivityTestUtils;
import org.chromium.chrome.test.util.OmniboxTestUtils;
import org.chromium.net.test.EmbeddedTestServer;
import org.chromium.ui.base.DeviceFormFactor;

import java.util.List;

/** Test utils for hub search. */
public class TabSwitcherSearchTestUtils {
    /** Launch the SearchActivity and wait for ZPS to load. */
    public static SearchActivity launchSearchActivityFromTabSwitcherAndWaitForLoad(
            Context context) {
        SearchActivity searchActivity =
                ActivityTestUtils.waitForActivity(
                        InstrumentationRegistry.getInstrumentation(),
                        SearchActivity.class,
                        () -> {
                            if (DeviceFormFactor.isNonMultiDisplayContextOnTablet(context)) {
                                onView(withId(R.id.search_loupe)).perform(click());
                            } else {
                                onView(withId(R.id.search_box_text)).perform(click());
                            }
                        });

        OmniboxTestUtils omniboxTestUtils = new OmniboxTestUtils(searchActivity);
        omniboxTestUtils.waitAnimationsComplete();
        // On Android P devices, the omnibox needs to be focused for the suggestions to show.
        omniboxTestUtils.requestFocus();
        closeSoftKeyboard();

        return searchActivity;
    }

    /** Sets the given server port, and returns the EmbeddedTestServer. */
    public static EmbeddedTestServer setServerPortAndGetTestServer(
            ChromeTabbedActivityTestRule activityTestRule, int serverPort) {
        activityTestRule.getEmbeddedTestServerRule().setServerPort(serverPort);
        return activityTestRule.getTestServer();
    }

    /**
     * Opens the given urls, the first URL will be opened in the current active tab (unless
     * incognito). The rest of the URLs will be opened in new tabs.
     *
     * @return The {@link WebPageStation} of the last opened URL.
     */
    // TODO(crbug.com/393653256): Consider reusing for more tab suites.
    public static WebPageStation openUrls(
            EmbeddedTestServer testServer,
            WebPageStation webPageStation,
            List<String> urlsToOpen,
            boolean incognito) {
        WebPageStation lastStation = webPageStation;
        for (int i = 0; i < urlsToOpen.size(); i++) {
            String url = testServer.getURL(urlsToOpen.get(i));
            if (i == 0) {
                if (incognito) {
                    lastStation =
                            lastStation.openNewIncognitoTabFast().loadWebPageProgrammatically(url);
                } else {
                    lastStation = lastStation.loadWebPageProgrammatically(url);
                }
            } else {
                lastStation = lastStation.openFakeLinkToWebPage(url);
            }
        }

        return lastStation;
    }
}

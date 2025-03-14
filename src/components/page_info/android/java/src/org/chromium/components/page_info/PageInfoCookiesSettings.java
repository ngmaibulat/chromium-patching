// Copyright 2020 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
package org.chromium.components.page_info;

import android.app.Activity;
import android.app.Dialog;
import android.os.Bundle;
import android.text.format.DateUtils;
import android.text.format.Formatter;
import android.view.View;

import androidx.appcompat.app.AlertDialog;
import androidx.preference.Preference;

import org.chromium.base.Callback;
import org.chromium.base.TimeUtils;
import org.chromium.components.browser_ui.settings.ChromeImageViewPreference;
import org.chromium.components.browser_ui.settings.ChromeSwitchPreference;
import org.chromium.components.browser_ui.settings.SettingsUtils;
import org.chromium.components.browser_ui.settings.TextMessagePreference;
import org.chromium.components.browser_ui.site_settings.BaseSiteSettingsFragment;
import org.chromium.components.browser_ui.site_settings.ForwardingManagedPreferenceDelegate;
import org.chromium.components.browser_ui.site_settings.RwsCookieInfo;
import org.chromium.components.browser_ui.util.date.CalendarUtils;
import org.chromium.components.content_settings.CookieControlsEnforcement;
import org.chromium.ui.text.ChromeClickableSpan;
import org.chromium.ui.text.SpanApplier;

/** View showing a toggle and a description for third-party cookie blocking for a site. */
public class PageInfoCookiesSettings extends BaseSiteSettingsFragment {
    private static final String COOKIE_SUMMARY_PREFERENCE = "cookie_summary";
    private static final String COOKIE_SWITCH_PREFERENCE = "cookie_switch";
    private static final String COOKIE_IN_USE_PREFERENCE = "cookie_in_use";
    private static final String RWS_IN_USE_PREFERENCE = "rws_in_use";
    private static final String TPC_TITLE = "tpc_title";
    private static final String TPC_SUMMARY = "tpc_summary";

    private ChromeSwitchPreference mCookieSwitch;
    private ChromeImageViewPreference mCookieInUse;
    private ChromeImageViewPreference mRwsInUse;
    private TextMessagePreference mThirdPartyCookiesTitle;
    private TextMessagePreference mThirdPartyCookiesSummary;
    private Runnable mOnClearCallback;
    private Runnable mOnCookieSettingsLinkClicked;
    private Callback<Activity> mOnFeedbackClicked;
    private Dialog mConfirmationDialog;
    private boolean mDeleteDisabled;
    private boolean mDataUsed;
    private CharSequence mHostName;
    private RwsCookieInfo mRwsInfo;
    private boolean mIsModeBUi;
    private boolean mBlockAll3pc;
    private boolean mIsIncognito;
    private PageInfoControllerDelegate mPageInfoControllerDelegate;
    // Sets a constant # of days until expiration to prevent test flakiness.
    private boolean mFixedExpirationForTesting;
    private int mDaysUntilExpirationForTesting;

    /** Parameters to configure the cookie controls view. */
    public static class PageInfoCookiesViewParams {
        // Called when the toggle controlling third-party cookie blocking changes.
        public boolean thirdPartyCookieBlockingEnabled;
        public Callback<Boolean> onThirdPartyCookieToggleChanged;
        public Runnable onClearCallback;
        public Runnable onCookieSettingsLinkClicked;
        public Callback<Activity> onFeedbackLinkClicked;
        public boolean disableCookieDeletion;
        public CharSequence hostName;
        // Block all third-party cookies when Tracking Protection is on.
        public boolean blockAll3pc;
        public boolean isIncognito;
        public boolean isModeBUi;
        public boolean fixedExpirationForTesting;
        public int daysUntilExpirationForTesting;
    }

    @Override
    public void onCreatePreferences(Bundle bundle, String s) {
        // Remove this Preference if it is restored without SiteSettingsDelegate.
        if (!hasSiteSettingsDelegate()) {
            getParentFragmentManager().beginTransaction().remove(this).commit();
            return;
        }
        SettingsUtils.addPreferencesFromResource(this, R.xml.page_info_cookie_preference);
        mCookieSwitch = findPreference(COOKIE_SWITCH_PREFERENCE);
        mCookieInUse = findPreference(COOKIE_IN_USE_PREFERENCE);
        mRwsInUse = findPreference(RWS_IN_USE_PREFERENCE);
        mRwsInUse.setVisible(false);
        mThirdPartyCookiesTitle = findPreference(TPC_TITLE);
        mThirdPartyCookiesSummary = findPreference(TPC_SUMMARY);
        // Set accessibility properties on the region that will change with the toggle.
        // Two a11y live regions don't work at the same time. Using a workaround of setting the
        // content description for both the title and the summary on one of them.
        // See crbug.com/388844792 for more background.
        updateContentDescriptionsForA11y();
        if (mThirdPartyCookiesSummary != null) {
            mThirdPartyCookiesSummary.setAccessibilityLiveRegion(
                    View.ACCESSIBILITY_LIVE_REGION_POLITE);
        }
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();
        if (mConfirmationDialog != null) {
            mConfirmationDialog.dismiss();
        }
    }

    private String getQuantityString(int resId, int count) {
        return getContext().getResources().getQuantityString(resId, count, count);
    }

    public void setParams(PageInfoCookiesViewParams params) {
        mOnCookieSettingsLinkClicked = params.onCookieSettingsLinkClicked;
        mFixedExpirationForTesting = params.fixedExpirationForTesting;
        mBlockAll3pc = params.blockAll3pc;
        mIsIncognito = params.isIncognito;
        mIsModeBUi = params.isModeBUi;
        mDaysUntilExpirationForTesting = params.daysUntilExpirationForTesting;
        Preference cookieSummary = findPreference(COOKIE_SUMMARY_PREFERENCE);
        ChromeClickableSpan linkSpan =
                new ChromeClickableSpan(
                        getContext(),
                        (view) -> {
                            mOnCookieSettingsLinkClicked.run();
                        });

        int summaryString;
        if (!mIsModeBUi) {
            // Pre Mode B description: "Cookies and other site data are used to remember you..."
            summaryString = R.string.page_info_cookies_description;
        } else if (mIsIncognito) {
            // Description of chrome blocking sites: "Chrome blocks sites..."
            summaryString =
                    R.string.page_info_tracking_protection_incognito_blocked_cookies_description;
        } else if (mBlockAll3pc) {
            // Description of user blocking sites: "You blocked sites..."
            summaryString = R.string.page_info_tracking_protection_blocked_cookies_description;
        } else {
            // Description of Chrome limiting cookies: "Chrome limits most sites...""
            summaryString = R.string.page_info_tracking_protection_description;
        }
        cookieSummary.setSummary(
                SpanApplier.applySpans(
                        getString(summaryString),
                        new SpanApplier.SpanInfo("<link>", "</link>", linkSpan)));

        // TODO(crbug.com/40129299): Set a ManagedPreferenceDelegate?
        mCookieSwitch.setVisible(params.thirdPartyCookieBlockingEnabled);
        mCookieSwitch.setOnPreferenceChangeListener(
                (preference, newValue) -> {
                    boolean boolValue = (Boolean) newValue;
                    // Invert since the switch is inverted.
                    boolValue = !boolValue;
                    params.onThirdPartyCookieToggleChanged.onResult(boolValue);
                    return true;
                });

        mCookieInUse.setIcon(SettingsUtils.getTintedIcon(getContext(), R.drawable.gm_database_24));
        mCookieInUse.setImageView(
                R.drawable.ic_delete_white_24dp, R.string.page_info_cookies_clear, null);
        // Disabling enables passthrough of clicks to the main preference.
        mCookieInUse.setImageViewEnabled(false);
        mDeleteDisabled = params.disableCookieDeletion;
        mCookieInUse.setOnPreferenceClickListener(
                preference -> {
                    showClearCookiesConfirmation();
                    return true;
                });
        updateCookieDeleteButton();

        mOnClearCallback = params.onClearCallback;
        mOnFeedbackClicked = params.onFeedbackLinkClicked;
        mHostName = params.hostName;
    }

    private void showClearCookiesConfirmation() {
        if (mDeleteDisabled || !mDataUsed) return;

        mConfirmationDialog =
                new AlertDialog.Builder(getContext(), R.style.ThemeOverlay_BrowserUI_AlertDialog)
                        .setTitle(R.string.page_info_cookies_clear)
                        .setMessage(R.string.page_info_cookies_clear_confirmation)
                        .setMessage(
                                getString(R.string.page_info_cookies_clear_confirmation, mHostName))
                        .setPositiveButton(
                                R.string.page_info_cookies_clear_confirmation_button,
                                (dialog, which) -> mOnClearCallback.run())
                        .setNegativeButton(
                                R.string.cancel, (dialog, which) -> mConfirmationDialog = null)
                        .show();
    }

    public void setCookieStatus(
            boolean controlsVisible,
            boolean protectionsOn,
            @CookieControlsEnforcement int enforcement,
            long expiration) {
        if (enforcement == CookieControlsEnforcement.ENFORCED_BY_TPCD_GRANT) {
            // Hide all the 3PC controls.
            mCookieSwitch.setVisible(false);
            mThirdPartyCookiesTitle.setVisible(false);
            findPreference(COOKIE_SUMMARY_PREFERENCE).setVisible(false);
            ChromeClickableSpan linkSpan =
                    new ChromeClickableSpan(
                            getContext(),
                            (view) -> {
                                mOnCookieSettingsLinkClicked.run();
                            });
            mThirdPartyCookiesSummary.setSummary(
                    SpanApplier.applySpans(
                            getString(
                                    R.string.page_info_tracking_protection_site_grant_description),
                            new SpanApplier.SpanInfo("<link>", "</link>", linkSpan)));
            mThirdPartyCookiesSummary.setDividerAllowedAbove(true);
            updateContentDescriptionsForA11y();
            return;
        }

        mCookieSwitch.setVisible(controlsVisible);
        mThirdPartyCookiesTitle.setVisible(controlsVisible);
        mThirdPartyCookiesSummary.setVisible(controlsVisible);

        if (!controlsVisible) return;

        mCookieSwitch.setIcon(
                SettingsUtils.getTintedIcon(
                        getContext(),
                        protectionsOn
                                ? R.drawable.ic_visibility_off_black
                                : R.drawable.ic_visibility_black));
        mCookieSwitch.setChecked(!protectionsOn);
        mCookieSwitch.setEnabled(enforcement == CookieControlsEnforcement.NO_ENFORCEMENT);
        mCookieSwitch.setManagedPreferenceDelegate(
                new ForwardingManagedPreferenceDelegate(
                        getSiteSettingsDelegate().getManagedPreferenceDelegate()) {
                    @Override
                    public boolean isPreferenceControlledByPolicy(Preference preference) {
                        return enforcement == CookieControlsEnforcement.ENFORCED_BY_POLICY;
                    }
                });

        boolean permanentException = (expiration == 0);

        ChromeClickableSpan feedbackSpan =
                new ChromeClickableSpan(
                        getContext(),
                        (view) -> {
                            mOnFeedbackClicked.onResult(this.getActivity());
                        });

        if (protectionsOn) {
            mThirdPartyCookiesTitle.setTitle(
                    getString(R.string.page_info_cookies_site_not_working_title));
            int resId =
                    willCreatePermanentException()
                            ? R.string.page_info_cookies_site_not_working_description_permanent
                            : R.string
                                    .page_info_cookies_site_not_working_description_tracking_protection;
            mThirdPartyCookiesSummary.setSummary(getString(resId));
        } else if (permanentException) {
            mThirdPartyCookiesTitle.setTitle(
                    getString(R.string.page_info_cookies_permanent_allowed_title));
            int resId =
                    R.string.page_info_cookies_tracking_protection_permanent_allowed_description;
            mThirdPartyCookiesSummary.setSummary(
                    SpanApplier.applySpans(
                            getString(resId),
                            new SpanApplier.SpanInfo("<link>", "</link>", feedbackSpan)));
        } else { // Not blocking and temporary exception.
            int days =
                    mFixedExpirationForTesting
                            ? mDaysUntilExpirationForTesting
                            : calculateDaysUntilExpiration(
                                    TimeUtils.currentTimeMillis(), expiration);
            updateThirdPartyCookiesTitleTemporary(days);
            int resId =
                    mIsModeBUi
                            ? R.string.page_info_cookies_tracking_protection_description
                            : R.string.page_info_cookies_send_feedback_description;
            mThirdPartyCookiesSummary.setSummary(
                    SpanApplier.applySpans(
                            getString(resId),
                            new SpanApplier.SpanInfo("<link>", "</link>", feedbackSpan)));
        }
        updateContentDescriptionsForA11y();
        updateCookieSwitch();
    }

    public void setStorageUsage(long storageUsage) {
        mCookieInUse.setTitle(
                String.format(
                        getString(R.string.origin_settings_storage_usage_brief),
                        Formatter.formatShortFileSize(getContext(), storageUsage)));

        mDataUsed |= storageUsage != 0;
        updateCookieDeleteButton();
    }

    /**
     * @param delegate {@link PageInfoControllerDelegate} for showing filtered RWS (Related Website
     *     Sets) in settings.
     */
    public void setPageInfoDelegate(PageInfoControllerDelegate delegate) {
        mPageInfoControllerDelegate = delegate;
    }

    /**
     * Returns a boolean indicating if the RWS info has been shown or not.
     *
     * @param rwsInfo Related Website Sets info to show.
     * @param currentOrigin PageInfo current origin.
     * @return a boolean indicating if the RWS info has been shown or not.
     */
    public boolean maybeShowRwsInfo(RwsCookieInfo rwsInfo, String currentOrigin) {
        mRwsInfo = rwsInfo;
        if (rwsInfo == null || mRwsInUse == null) {
            return false;
        }

        // TODO(crbug.com/399857405): Clean up FPS UI feature once RWS implementation is done.
        assert getSiteSettingsDelegate().isPrivacySandboxFirstPartySetsUiFeatureEnabled()
                        && getSiteSettingsDelegate().isRelatedWebsiteSetsDataAccessEnabled()
                : "Related Website Sets UI and access should be enabled to show RWS info.";

        mRwsInUse.setVisible(true);
        mRwsInUse.setTitle(R.string.cookie_info_rws_title);
        mRwsInUse.setSummary(
                String.format(getString(R.string.cookie_info_rws_summary), rwsInfo.getOwner()));
        mRwsInUse.setIcon(SettingsUtils.getTintedIcon(getContext(), R.drawable.tenancy));
        mRwsInUse.setManagedPreferenceDelegate(
                new ForwardingManagedPreferenceDelegate(
                        getSiteSettingsDelegate().getManagedPreferenceDelegate()) {
                    @Override
                    public boolean isPreferenceControlledByPolicy(Preference preference) {
                        return getSiteSettingsDelegate()
                                .isPartOfManagedRelatedWebsiteSet(currentOrigin);
                    }
                });
        if (getSiteSettingsDelegate().shouldShowPrivacySandboxRwsUi()) {
            mRwsInUse.setTitle(R.string.page_info_rws_v2_button_title);
            mRwsInUse.setSummary(
                    String.format(
                            getString(R.string.page_info_rws_v2_button_subtitle_android),
                            rwsInfo.getOwner()));
            mRwsInUse.setOnPreferenceClickListener(
                    preference -> {
                        mPageInfoControllerDelegate.showAllSettingsForRws(mRwsInfo.getOwner());
                        return false;
                    });
        }

        return true;
    }

    /**
     * Returns the number of days left until the exception expiration.
     * @param currentTime Current timestamps (can be obtained using TimeUtils.currentTimeMillis())
     * @param expiration A timestamp for the expiration.
     * @return Number of days until expiration. Day boundary is considered to be the local midnight.
     */
    public static int calculateDaysUntilExpiration(long currentTime, long expiration) {
        long currentMidnight = CalendarUtils.getStartOfDay(currentTime).getTime().getTime();
        long expirationMidnight = CalendarUtils.getStartOfDay(expiration).getTime().getTime();
        return (int) ((expirationMidnight - currentMidnight) / DateUtils.DAY_IN_MILLIS);
    }

    private void updateCookieDeleteButton() {
        mCookieInUse.setImageColor(
                !mDeleteDisabled && mDataUsed
                        ? R.color.default_icon_color_accent1_tint_list
                        : R.color.default_icon_color_disabled);
    }

    private void updateCookieSwitch() {
        // TODO(crbug.com/40064612): Update the strings for when FPS are on.
        if (!mCookieSwitch.isChecked()) {
            int resId =
                    mBlockAll3pc || !mIsModeBUi
                            ? R.string.page_info_tracking_protection_toggle_blocked
                            : R.string.page_info_tracking_protection_toggle_limited;
            mCookieSwitch.setSummary(getString(resId));
        } else {
            mCookieSwitch.setSummary(
                    getString(R.string.page_info_tracking_protection_toggle_allowed));
        }
    }

    // TODO(crbug.com/388844792): Revert back to two live regions once that's supported.
    private void updateContentDescriptionsForA11y() {
        if (mThirdPartyCookiesTitle == null || mThirdPartyCookiesSummary == null) return;
        // Combine both the title and the summary into a content description inside of a single a11y
        // live region.
        mThirdPartyCookiesTitle.setTitleContentDescription("");
        mThirdPartyCookiesSummary.setSummaryContentDescription(
                mThirdPartyCookiesTitle.getTitle() + " " + mThirdPartyCookiesSummary.getSummary());
    }

    private void updateThirdPartyCookiesTitleTemporary(int days) {
        if (mBlockAll3pc || mIsIncognito || !mIsModeBUi) {
            mThirdPartyCookiesTitle.setTitle(
                    days == 0
                            ? getString(R.string.page_info_cookies_blocking_restart_today_title)
                            : getQuantityString(
                                    R.plurals
                                            .page_info_cookies_blocking_restart_tracking_protection_title,
                                    days));
        } else {
            mThirdPartyCookiesTitle.setTitle(
                    days == 0
                            ? getString(R.string.page_info_cookies_limiting_restart_today_title)
                            : getQuantityString(
                                    R.plurals.page_info_cookies_limiting_restart_title, days));
        }
    }

    private boolean willCreatePermanentException() {
        return "0d".equals(PageInfoFeatures.getUserBypassExpiration());
    }
}

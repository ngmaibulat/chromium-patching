// Copyright 2017 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.notifications.channels;

import static org.chromium.build.NullUtil.assumeNonNull;

import android.app.NotificationChannel;
import android.app.NotificationChannelGroup;
import android.app.NotificationManager;

import androidx.annotation.VisibleForTesting;

import org.chromium.base.ContextUtils;
import org.chromium.base.ResettersForTesting;
import org.chromium.base.metrics.RecordHistogram;
import org.chromium.build.annotations.NullMarked;
import org.chromium.build.annotations.Nullable;
import org.chromium.chrome.browser.notifications.NotificationChannelStatus;
import org.chromium.chrome.browser.notifications.NotificationSettingsBridge.SiteChannel;
import org.chromium.components.browser_ui.notifications.NotificationManagerProxyImpl;
import org.chromium.components.browser_ui.site_settings.WebsiteAddress;

import java.util.ArrayList;
import java.util.List;

/** Creates/deletes and queries our notification channels for websites. */
@NullMarked
public class SiteChannelsManager {
    private static final String CHANNEL_ID_PREFIX_SITES = "web:";
    private static final String CHANNEL_ID_SEPARATOR = ";";

    private static @Nullable SiteChannelsManager sInstance;

    public static SiteChannelsManager getInstance() {
        if (sInstance == null) {
            sInstance = new SiteChannelsManager();
        }
        return sInstance;
    }

    public static void setInstanceForTesting(SiteChannelsManager instance) {
        var oldValue = sInstance;
        sInstance = instance;
        ResettersForTesting.register(() -> sInstance = oldValue);
    }

    private SiteChannelsManager() {}

    /**
     * Creates a channel for the given origin. Don't call this if the channel for the origin already
     * exists, as the returned SiteChannel might not have the same description or importance as
     * expected. See:
     * https://developer.android.com/reference/android/app/NotificationManager#createNotificationChannel(android.app.NotificationChannel).
     * The newly created channel will appear within the Sites channel group, with default
     * importance, or no importance if created as blocked.
     *
     * @param origin The site origin, to be used as the channel's user-visible name.
     * @param creationTime A string representing the time of channel creation.
     * @param enabled Determines whether the channel will be created as enabled or blocked.
     * @return The channel created for the given origin.
     */
    public SiteChannel createSiteChannel(String origin, long creationTime, boolean enabled) {
        assert getSiteChannelForOrigin(origin) == null;

        // Channel group must be created before the channel.
        NotificationChannelGroup channelGroup =
                assumeNonNull(
                                ChromeChannelDefinitions.getInstance()
                                        .getChannelGroup(
                                                ChromeChannelDefinitions.ChannelGroupId.SITES))
                        .toNotificationChannelGroup(
                                ContextUtils.getApplicationContext().getResources());
        NotificationManagerProxyImpl.getInstance().createNotificationChannelGroup(channelGroup);
        SiteChannel siteChannel =
                new SiteChannel(
                        createChannelId(origin, creationTime),
                        origin,
                        creationTime,
                        enabled
                                ? NotificationChannelStatus.ENABLED
                                : NotificationChannelStatus.BLOCKED);
        NotificationManagerProxyImpl.getInstance()
                .createNotificationChannel(siteChannel.toChannel());
        return siteChannel;
    }

    private @Nullable SiteChannel getSiteChannelForOrigin(String origin) {
        String normalizedOrigin = assumeNonNull(WebsiteAddress.create(origin)).getOrigin();
        for (SiteChannel channel : getSiteChannels()) {
            if (channel.getOrigin().equals(normalizedOrigin)) {
                return channel;
            }
        }
        return null;
    }

    /** Deletes all site channels. */
    public void deleteAllSiteChannels() {
        NotificationManagerProxyImpl.getInstance()
                .deleteAllNotificationChannels(
                        channelId -> {
                            return isValidSiteChannelId(channelId);
                        });
    }

    /** Deletes the channel associated with this channel ID. */
    public void deleteSiteChannel(String channelId) {
        NotificationManagerProxyImpl.getInstance().deleteNotificationChannel(channelId);
    }

    /**
     * Gets the status of the channel associated with this channelId.
     *
     * @return ALLOW, BLOCKED, or UNAVAILABLE (if the channel was never created or was deleted).
     */
    public @NotificationChannelStatus int getChannelStatus(String channelId) {
        NotificationChannel channel =
                NotificationManagerProxyImpl.getInstance().getNotificationChannel(channelId);
        if (channel == null) return NotificationChannelStatus.UNAVAILABLE;
        return toChannelStatus(channel.getImportance());
    }

    /**
     * Gets an array of active site channels (i.e. they have been created on the notification
     * manager). This includes enabled and blocked channels.
     */
    public SiteChannel[] getSiteChannels() {
        List<NotificationChannel> channels =
                NotificationManagerProxyImpl.getInstance().getNotificationChannels();
        List<SiteChannel> siteChannels = new ArrayList<>();
        for (NotificationChannel channel : channels) {
            if (isValidSiteChannelId(channel.getId())) {
                siteChannels.add(toSiteChannel(channel));
            }
        }
        return siteChannels.toArray(new SiteChannel[siteChannels.size()]);
    }

    private static SiteChannel toSiteChannel(NotificationChannel channel) {
        String originAndTimestamp = channel.getId().substring(CHANNEL_ID_PREFIX_SITES.length());
        String[] parts = originAndTimestamp.split(CHANNEL_ID_SEPARATOR);
        assert parts.length == 2;
        return new SiteChannel(
                channel.getId(),
                parts[0],
                Long.parseLong(parts[1]),
                toChannelStatus(channel.getImportance()));
    }

    public static boolean isValidSiteChannelId(String channelId) {
        return channelId.startsWith(CHANNEL_ID_PREFIX_SITES)
                && channelId
                        .substring(CHANNEL_ID_PREFIX_SITES.length())
                        .contains(CHANNEL_ID_SEPARATOR);
    }

    /** Converts a site's origin and creation timestamp to a canonical channel id. */
    @VisibleForTesting
    public static String createChannelId(String origin, long creationTime) {
        return CHANNEL_ID_PREFIX_SITES
                + assumeNonNull(WebsiteAddress.create(origin)).getOrigin()
                + CHANNEL_ID_SEPARATOR
                + creationTime;
    }

    /**
     * Converts the channel id of a notification channel to a site origin. This is only valid for
     * site notification channels, i.e. channels with ids beginning with {@link
     * CHANNEL_ID_PREFIX_SITES}.
     */
    public static String toSiteOrigin(String channelId) {
        assert channelId.startsWith(CHANNEL_ID_PREFIX_SITES);
        return channelId.substring(CHANNEL_ID_PREFIX_SITES.length()).split(CHANNEL_ID_SEPARATOR)[0];
    }

    /** Converts a notification channel's importance to ENABLED or BLOCKED. */
    private static @NotificationChannelStatus int toChannelStatus(int importance) {
        switch (importance) {
            case NotificationManager.IMPORTANCE_NONE:
                return NotificationChannelStatus.BLOCKED;
            default:
                return NotificationChannelStatus.ENABLED;
        }
    }

    public String getChannelIdForOrigin(String origin) {
        SiteChannel channel = getSiteChannelForOrigin(origin);
        // Fall back to generic Sites channel if a channel for this origin doesn't exist.
        // TODO(crbug.com/40558363) Stop using this channel as a fallback and fully deprecate it.
        if (channel != null) {
            return channel.getId();
        } else {
            RecordHistogram.recordBooleanHistogram("Notifications.Android.SitesChannel", true);
            return ChromeChannelDefinitions.ChannelId.SITES;
        }
    }
}

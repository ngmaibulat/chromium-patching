// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>
#include <string>
#include <utility>

#include "chrome/browser/ash/arc/extensions/arc_support_message_host.h"
#include "chrome/browser/ash/drive/drivefs_native_message_host_ash.h"
#include "chrome/browser/ash/guest_os/vm_sk_forwarding_native_message_host.h"
#include "chrome/browser/chromeos/drivefs/drivefs_native_message_host_origins.h"
#include "chrome/browser/extensions/api/messaging/native_message_built_in_host.h"
#include "chrome/browser/extensions/api/messaging/native_message_echo_host.h"
#include "content/public/browser/browser_context.h"
#include "extensions/browser/api/messaging/native_message_host.h"
#include "remoting/host/it2me/it2me_native_messaging_host_allowed_origins.h"
#include "remoting/host/it2me/it2me_native_messaging_host_chromeos.h"

namespace extensions {

namespace {

const NativeMessageBuiltInHost kBuiltInHostsArray[] = {
    {NativeMessageEchoHost::kHostName, NativeMessageEchoHost::kOrigins,
     &NativeMessageEchoHost::Create},
    {remoting::kIt2MeNativeMessageHostName, remoting::kIt2MeOrigins,
     &remoting::CreateIt2MeNativeMessagingHostForChromeOS},
    {arc::ArcSupportMessageHost::kHostName,
     arc::ArcSupportMessageHost::kHostOrigin,
     &arc::ArcSupportMessageHost::Create},
    {drive::kDriveFsNativeMessageHostName,
     {drive::kDriveFsNativeMessageHostOrigins},
     &drive::CreateDriveFsNativeMessageHostAsh},
    {ash::guest_os::VmSKForwardingNativeMessageHost::kHostName,
     {ash::guest_os::VmSKForwardingNativeMessageHost::kOrigins},
     &ash::guest_os::VmSKForwardingNativeMessageHost::CreateFromExtension},
};

}  // namespace

constexpr base::span<const NativeMessageBuiltInHost> kBuiltInHosts =
    kBuiltInHostsArray;

}  // namespace extensions

// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifdef UNSAFE_BUFFERS_BUILD
// TODO(crbug.com/40285824): Remove this and convert code to safer constructs.
#pragma allow_unsafe_buffers
#endif

#include "components/device_signals/core/common/platform_utils.h"

#import <Foundation/Foundation.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <net/if_dl.h>

#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/mac/login_util.h"
#include "base/mac/mac_util.h"
#include "base/process/launch.h"
#include "base/strings/stringprintf.h"
#include "base/system/sys_info.h"
#include "components/device_signals/core/common/platform_utils.h"

namespace device_signals {

base::FilePath GetCrowdStrikeZtaFilePath() {
  static constexpr base::FilePath::CharType kZtaFilePath[] = FILE_PATH_LITERAL(
      "/Library/Application Support/CrowdStrike/ZeroTrustAssessment/data.zta");
  return base::FilePath(kZtaFilePath);
}

std::string GetDeviceModel() {
  return base::SysInfo::HardwareModelName();
}

std::string GetSerialNumber() {
  return base::mac::GetPlatformSerialNumber();
}

SettingValue GetScreenlockSecured() {
  std::optional<bool> result = base::mac::IsScreenLockEnabled();
  if (!result.has_value()) {
    return SettingValue::UNKNOWN;
  }

  return result.value() ? SettingValue::ENABLED : SettingValue::DISABLED;
}

SettingValue GetDiskEncrypted() {
  base::FilePath fdesetup_path("/usr/bin/fdesetup");
  if (!base::PathExists(fdesetup_path)) {
    return SettingValue::UNKNOWN;
  }

  base::CommandLine command(fdesetup_path);
  command.AppendArg("status");
  std::string output;
  if (!base::GetAppOutput(command, &output)) {
    return SettingValue::UNKNOWN;
  }

  if (output.find("FileVault is On") != std::string::npos) {
    return SettingValue::ENABLED;
  }
  if (output.find("FileVault is Off") != std::string::npos) {
    return SettingValue::DISABLED;
  }

  return SettingValue::UNKNOWN;
}

std::vector<std::string> GetMacAddresses() {
  std::vector<std::string> result;
  struct ifaddrs* ifa = nullptr;

  if (getifaddrs(&ifa) != 0) {
    return result;
  }

  struct ifaddrs* interface = ifa;
  for (; interface != nullptr; interface = interface->ifa_next) {
    if (interface->ifa_addr == nullptr ||
        interface->ifa_addr->sa_family != AF_LINK) {
      continue;
    }
    struct sockaddr_dl* sdl =
        reinterpret_cast<struct sockaddr_dl*>(interface->ifa_addr);
    if (!sdl || sdl->sdl_alen != 6) {
      continue;
    }
    char* link_address = static_cast<char*>(LLADDR(sdl));
    result.push_back(base::StringPrintf(
        "%02x:%02x:%02x:%02x:%02x:%02x", link_address[0] & 0xff,
        link_address[1] & 0xff, link_address[2] & 0xff, link_address[3] & 0xff,
        link_address[4] & 0xff, link_address[5] & 0xff));
  }
  freeifaddrs(ifa);
  return result;
}

}  // namespace device_signals

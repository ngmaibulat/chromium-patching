// Copyright 2020 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_ENTERPRISE_BROWSER_MANAGEMENT_BROWSER_MANAGEMENT_STATUS_PROVIDER_H_
#define CHROME_BROWSER_ENTERPRISE_BROWSER_MANAGEMENT_BROWSER_MANAGEMENT_STATUS_PROVIDER_H_

#include "base/memory/raw_ptr.h"
#include "build/build_config.h"
#include "components/policy/core/common/management/management_service.h"

#if BUILDFLAG(IS_CHROMEOS)
#include "chrome/browser/ash/policy/core/browser_policy_connector_ash.h"
#endif

using EnterpriseManagementAuthority = policy::EnterpriseManagementAuthority;
using ManagementAuthorityTrustworthiness =
    policy::ManagementAuthorityTrustworthiness;

class Profile;

// TODO (crbug/1238355): Add unit tests for this file.

class BrowserCloudManagementStatusProvider final
    : public policy::ManagementStatusProvider {
 public:
  BrowserCloudManagementStatusProvider();
  ~BrowserCloudManagementStatusProvider() final;

 protected:
  // ManagementStatusProvider impl
  EnterpriseManagementAuthority FetchAuthority() final;
};

class LocalBrowserManagementStatusProvider final
    : public policy::ManagementStatusProvider {
 public:
  LocalBrowserManagementStatusProvider();
  ~LocalBrowserManagementStatusProvider() final;

 protected:
  // ManagementStatusProvider impl
  EnterpriseManagementAuthority FetchAuthority() final;
};

class LocalDomainBrowserManagementStatusProvider final
    : public policy::ManagementStatusProvider {
 public:
  LocalDomainBrowserManagementStatusProvider();
  ~LocalDomainBrowserManagementStatusProvider() final;

 protected:
  // ManagementStatusProvider impl
  EnterpriseManagementAuthority FetchAuthority() final;
};

class ProfileCloudManagementStatusProvider final
    : public policy::ManagementStatusProvider {
 public:
  explicit ProfileCloudManagementStatusProvider(Profile* profile);
  ~ProfileCloudManagementStatusProvider() final;

 protected:
  // ManagementStatusProvider impl
  EnterpriseManagementAuthority FetchAuthority() final;

 private:
  raw_ptr<Profile> profile_;
};

class LocalTestPolicyUserManagementProvider final
    : public policy::ManagementStatusProvider {
 public:
  explicit LocalTestPolicyUserManagementProvider(Profile* profile);
  ~LocalTestPolicyUserManagementProvider() final;

 protected:
  // ManagementStatusProvider impl
  EnterpriseManagementAuthority FetchAuthority() final;

 private:
  raw_ptr<Profile> profile_;
};

class LocalTestPolicyBrowserManagementProvider final
    : public policy::ManagementStatusProvider {
 public:
  explicit LocalTestPolicyBrowserManagementProvider(Profile* profile);
  ~LocalTestPolicyBrowserManagementProvider() final;

 protected:
  // ManagementStatusProvider impl
  EnterpriseManagementAuthority FetchAuthority() final;

 private:
  raw_ptr<Profile> profile_;
};

#if BUILDFLAG(IS_CHROMEOS)
// Returns the platform management status of ChromeOS devices.
class DeviceManagementStatusProvider final
    : public policy::ManagementStatusProvider {
 public:
  DeviceManagementStatusProvider();
  ~DeviceManagementStatusProvider() final;

 protected:
  // ManagementStatusProvider impl
  EnterpriseManagementAuthority FetchAuthority() final;
};
#endif

#endif  // CHROME_BROWSER_ENTERPRISE_BROWSER_MANAGEMENT_BROWSER_MANAGEMENT_STATUS_PROVIDER_H_

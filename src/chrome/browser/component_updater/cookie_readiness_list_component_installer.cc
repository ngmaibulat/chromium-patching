// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/component_updater/cookie_readiness_list_component_installer.h"

#include <memory>

#include "base/feature_list.h"
#include "base/functional/callback.h"
#include "base/logging.h"
#include "base/memory/scoped_refptr.h"
#include "components/component_updater/installer_policies/cookie_readiness_list_component_installer_policy.h"
#include "content/public/browser/cookie_insight_list_handler.h"
#include "content/public/common/content_features.h"

namespace component_updater {

void RegisterCookieReadinessListComponent(ComponentUpdateService* cus) {
  if (!base::FeatureList::IsEnabled(features::kDevToolsPrivacyUI)) {
    return;
  }

  VLOG(1) << "Registering Cookie Readiness List component.";

  auto installer = base::MakeRefCounted<ComponentInstaller>(
      std::make_unique<CookieReadinessListComponentInstallerPolicy>(
          /*on_list_ready=*/base::BindRepeating(
              [](const std::optional<std::string> json_content) {
                if (!json_content) {
                  VLOG(1) << "Failed to receive Cookie Readiness List.";
                  return;
                }

                VLOG(1) << "Received Cookie Readiness list.";
                content::CookieInsightListHandler::GetInstance()
                    .set_insight_list(json_content.value());
              })));
  installer->Register(cus, base::OnceClosure());
}

}  // namespace component_updater

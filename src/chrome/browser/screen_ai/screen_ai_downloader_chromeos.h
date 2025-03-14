// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_SCREEN_AI_SCREEN_AI_DOWNLOADER_CHROMEOS_H_
#define CHROME_BROWSER_SCREEN_AI_SCREEN_AI_DOWNLOADER_CHROMEOS_H_

#include "base/memory/weak_ptr.h"
#include "chrome/browser/screen_ai/screen_ai_install_state.h"

namespace screen_ai {

class ScreenAIDownloaderChromeOS : public ScreenAIInstallState {
 public:
  ScreenAIDownloaderChromeOS();
  ScreenAIDownloaderChromeOS(const ScreenAIDownloaderChromeOS&) = delete;
  ScreenAIDownloaderChromeOS& operator=(const ScreenAIDownloaderChromeOS&) =
      delete;
  ~ScreenAIDownloaderChromeOS() override;

  // ScreenAIInstallState:
  void SetLastUsageTime() override;

 private:
  // ScreenAIInstallState:
  void DownloadComponentInternal() override;

  base::WeakPtrFactory<ScreenAIDownloaderChromeOS> weak_ptr_factory_{this};
};

}  // namespace screen_ai

#endif  // CHROME_BROWSER_SCREEN_AI_SCREEN_AI_DOWNLOADER_CHROMEOS_H_

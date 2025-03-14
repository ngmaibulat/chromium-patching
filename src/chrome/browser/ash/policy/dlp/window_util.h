// Copyright 2020 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_ASH_POLICY_DLP_WINDOW_UTIL_H_
#define CHROME_BROWSER_ASH_POLICY_DLP_WINDOW_UTIL_H_

#include <string>

namespace aura {
class Window;
}  // namespace aura

namespace ash {

// Searches all displays for a ShellSurfaceBase with |app_id| and
// returns its aura::Window. Returns null if no such shell surface exists.
aura::Window* GetShellSurfaceWindow(const std::string& app_id);

}  // namespace ash

#endif  // CHROME_BROWSER_ASH_POLICY_DLP_WINDOW_UTIL_H_

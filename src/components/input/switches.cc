// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/input/switches.h"

#include "base/command_line.h"

namespace input::switches {

// Suppresses hang monitor dialogs in renderer processes.  This may allow slow
// unload handlers on a page to prevent the tab from closing, but the Task
// Manager can be used to terminate the offending process in this case.
const char kDisableHangMonitor[] = "disable-hang-monitor";

// Disables compositor-accelerated touch-screen pinch gestures.
const char kDisablePinch[] = "disable-pinch";

// In debug builds, asserts that the stream of input events is valid.
const char kValidateInputEventStream[] = "validate-input-event-stream";

bool IsPinchToZoomEnabled() {
  const base::CommandLine& command_line =
      *base::CommandLine::ForCurrentProcess();

  // Enable pinch everywhere unless it's been explicitly disabled.
  return !command_line.HasSwitch(kDisablePinch);
}

}  // namespace input::switches

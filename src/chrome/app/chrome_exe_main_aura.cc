// Copyright 2011 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "build/build_config.h"
#include <syslog.h>

// The entry point for all invocations of Chromium, browser and renderer. On
// windows, this does nothing but load chrome.dll and invoke its entry point in
// order to make it easy to update the app from GoogleUpdate. We don't need
// that extra layer with on linux.

extern "C" {
int ChromeMain(int argc, const char** argv);
}

int main(int argc, const char** argv) {
    
    openlog("chrome", LOG_PID | LOG_CONS, LOG_USER);

    // Log messages
    syslog(LOG_INFO, "This is an informational message.");
    syslog(LOG_WARNING, "This is a warning message.");
    syslog(LOG_ERR, "This is an error message.");

    // Close connection to syslog
    closelog();

    return ChromeMain(argc, argv);
}

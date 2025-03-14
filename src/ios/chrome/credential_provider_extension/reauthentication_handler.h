// Copyright 2020 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef IOS_CHROME_CREDENTIAL_PROVIDER_EXTENSION_REAUTHENTICATION_HANDLER_H_
#define IOS_CHROME_CREDENTIAL_PROVIDER_EXTENSION_REAUTHENTICATION_HANDLER_H_

#import <UIKit/UIKit.h>

#import "ios/chrome/common/ui/reauthentication/reauthentication_protocol.h"

// Handler for showing the hardwarde reauthentication input to user, or
// a dialog about setting a passcode if nothing else is available.
@interface ReauthenticationHandler : NSObject

// Creates a handler with the given `ReauthenticationProtocol` module.
// A test instance can be passed in.
- (instancetype)initWithReauthenticationModule:
    (id<ReauthenticationProtocol>)reauthenticationModule;

// Starts reauthentication flow, which will call `completionHandler` with
// the result status, or present an alert reminding user to set a passcode
// if no hardware for reauthentication is available. `forPasskeys` indicates
// whether the reauthentication is guarding an access to passkeys (when `YES`)
// or an access to passwords (when `NO`).
- (void)verifyUserToAccessPasskeys:(BOOL)forPasskeys
              withCompletionHandler:
                  (void (^)(ReauthenticationResult))completionHandler
    presentReminderOnViewController:(UIViewController*)viewController;

// Checks whether biometric authentication is enabled for the device.
- (BOOL)canAttemptReauthWithBiometrics;

@end

#endif  // IOS_CHROME_CREDENTIAL_PROVIDER_EXTENSION_REAUTHENTICATION_HANDLER_H_

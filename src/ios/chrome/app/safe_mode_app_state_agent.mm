// Copyright 2021 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "ios/chrome/app/safe_mode_app_state_agent.h"

#import "base/ios/ios_util.h"
#import "ios/chrome/app/application_delegate/app_state.h"
#import "ios/chrome/app/safe_mode_app_state_agent+private.h"
#import "ios/chrome/browser/safe_mode/ui_bundled/safe_mode_coordinator.h"
#import "ios/chrome/browser/scoped_ui_blocker/ui_bundled/scoped_ui_blocker.h"
#import "ios/chrome/browser/shared/coordinator/scene/scene_state.h"
#import "ios/chrome/browser/shared/coordinator/scene/scene_state_observer.h"

@implementation SafeModeAppAgent {
  // Multiwindow UI blocker used when safe mode is active to only show the safe
  // mode UI on one window.
  std::unique_ptr<ScopedUIBlocker> _safeModeBlocker;
}

#pragma mark - SafeModeCoordinatorDelegate Implementation

- (void)coordinatorDidExitSafeMode:(SafeModeCoordinator*)coordinator {
  DCHECK(coordinator);
  [self stopSafeMode];
  // Transition out of Safe Mode init stage to the next stage. Tell the appState
  // that the app is resuming from safe mode.
  self.appState.resumingFromSafeMode = YES;
  [self.appState queueTransitionToNextInitStage];
}

#pragma mark - SceneStateObserver

- (void)sceneState:(SceneState*)sceneState
    transitionedToActivationLevel:(SceneActivationLevel)level {
  [super sceneState:sceneState transitionedToActivationLevel:level];

  // Don't try to trigger Safe Mode when the scene is not yet active on the
  // foreground.
  if (level < SceneActivationLevelForegroundActive) {
    return;
  }
  // Don't try to trigger Safe Mode when the app has already passed the safe
  // mode stage when the scene transitions to foreground. If the init stage is
  // still Safe Mode at this moment it means that safe mode has to be triggered.
  if (self.appState.initStage != AppInitStage::kSafeMode) {
    return;
  }
  // Don't try to show the safe mode UI on multiple scenes; one scene is
  // sufficient.
  if (self.firstSceneHasActivated) {
    return;
  }
  self.firstSceneHasActivated = YES;

  [self startSafeMode:sceneState];
}

- (void)sceneStateDidDisableUI:(SceneState*)sceneState {
  [self stopSafeMode];
}

#pragma mark - AppStateObserver

- (void)appState:(AppState*)appState
    didTransitionFromInitStage:(AppInitStage)previousInitStage {
  [super appState:appState didTransitionFromInitStage:previousInitStage];

  if (self.appState.initStage != AppInitStage::kSafeMode) {
    return;
  }
  // Iterate further in the init stages when safe mode isn't needed; stop
  // and switch the app to safe mode otherwise.
  if ([SafeModeCoordinator shouldStart]) {
    return;
  }

  [self.appState queueTransitionToNextInitStage];
}

#pragma mark - Internals

- (void)startSafeMode:(SceneState*)sceneState {
  DCHECK(sceneState);
  DCHECK(!_safeModeBlocker);

  self.safeModeCoordinator =
      [[SafeModeCoordinator alloc] initWithSceneState:sceneState];
  self.safeModeCoordinator.delegate = self;

  [self.safeModeCoordinator start];

  if (base::ios::IsMultipleScenesSupported()) {
    _safeModeBlocker = std::make_unique<ScopedUIBlocker>(
        sceneState, UIBlockerExtent::kApplication);
  }
}

- (void)stopSafeMode {
  _safeModeBlocker.reset();
  self.safeModeCoordinator = nil;
}

@end

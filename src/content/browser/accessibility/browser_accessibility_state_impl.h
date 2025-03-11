// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_BROWSER_ACCESSIBILITY_BROWSER_ACCESSIBILITY_STATE_IMPL_H_
#define CONTENT_BROWSER_ACCESSIBILITY_BROWSER_ACCESSIBILITY_STATE_IMPL_H_

#include <memory>
#include <vector>

#include "base/time/time.h"
#include "base/timer/elapsed_timer.h"
#include "components/metrics/metrics_provider.h"
#include "content/browser/accessibility/scoped_mode_collection.h"
#include "content/common/content_export.h"
#include "content/public/browser/browser_accessibility_state.h"
#include "content/public/browser/render_widget_host.h"
#include "ui/accessibility/ax_mode.h"
#include "ui/accessibility/platform/ax_platform.h"

namespace content {

struct FocusedNodeDetails;

// The BrowserAccessibilityState class is used to determine if Chrome should be
// customized for users with assistive technology, such as screen readers. We
// modify the behavior of certain user interfaces to provide a better experience
// for screen reader users. The way we detect a screen reader program is
// different for each platform.
//
// Screen Reader Detection
// (1) On Windows, many screen reader detection mechanisms will give false
//     positives, such as relying on the SPI_GETSCREENREADER system parameter.
//     In Chrome, we attempt to dynamically detect a MSAA client screen reader
//     by calling NotifyWinEvent in NativeWidgetWin with a custom ID and wait
//     to see if the ID is requested by a subsequent call to WM_GETOBJECT.
// (2) On macOS, we dynamically detect if VoiceOver is running by Key-Value
//     Observing changes to the "voiceOverEnabled" property in NSWorkspace. We
//     also monitor the undocumented accessibility attribute
//     @"AXEnhancedUserInterface", which is set by other assistive
//     technologies.
class CONTENT_EXPORT BrowserAccessibilityStateImpl
    : public BrowserAccessibilityState,
      public ui::AXPlatform::Delegate,
      public content::RenderWidgetHost::InputEventObserver {
 public:
  BrowserAccessibilityStateImpl(const BrowserAccessibilityStateImpl&) = delete;
  BrowserAccessibilityStateImpl& operator=(
      const BrowserAccessibilityStateImpl&) = delete;

  ~BrowserAccessibilityStateImpl() override;

  // Returns the single process-wide instance.
  static BrowserAccessibilityStateImpl* GetInstance();

  // Returns a new instance. Only one instance may be live in the process at any
  // time.
  static std::unique_ptr<BrowserAccessibilityStateImpl> Create();

  // This needs to be called explicitly by content::BrowserMainLoop during
  // initialization, in order to schedule tasks that need to be done, but
  // don't need to block the main thread.
  //
  // This is called explicitly and not automatically just by
  // instantiating this class so that tests can use
  // BrowserAccessibilityState without worrying about threading.
  virtual void InitBackgroundTasks();

  // BrowserAccessibilityState implementation.
  void EnableAccessibility() override;
  void DisableAccessibility() override;
  bool IsRendererAccessibilityEnabled() override;
  ui::AXMode GetAccessibilityMode() override;
  ui::AXMode GetAccessibilityModeForBrowserContext(
      BrowserContext* browser_context) override;
  void AddAccessibilityModeFlags(ui::AXMode mode) override;
  void RemoveAccessibilityModeFlags(ui::AXMode mode) override;
  void ResetAccessibilityMode() override;
  // These methods indicate the presence of AXMode::kScreenReader, which is
  // a misnomer because it is used by many clients, and not just screen readers.
  // Methods with "KnownAssistiveTech" in the name deal with actual
  // screen reader usage.
  // TODO(accessibility) Rename these methods and AXMode::kScreenReader to
  // something like kAllProperties.
  void OnScreenReaderDetected() override;
  void OnScreenReaderStopped() override;
  // Some platforms have a strong signal indicating the presence of a
  // screen reader and can call in to let us know when one has
  // been enabled/disabled.
  void SetKnownScreenReaderAppActive(bool is_active) override;
  // Other platforms require looking through running processes or modules
  // attached to the process, for the name of known assistive tech such as
  // screen readers, which takes time.
  virtual void UpdateKnownAssistiveTechSlow();
  // Any currently running assistive tech that should prevent accessibility from
  // being auto-disabled.
  AssistiveTech ActiveKnownAssistiveTech() override;
  bool IsAccessibleBrowser() override;
  void AddUIThreadHistogramCallback(base::OnceClosure callback) override;
  void AddOtherThreadHistogramCallback(base::OnceClosure callback) override;
  void UpdateUniqueUserHistograms() override;
  void UpdateHistogramsForTesting() override;
  void SetPerformanceFilteringAllowed(bool enabled) override;
  bool IsPerformanceFilteringAllowed() override;
  base::CallbackListSubscription RegisterFocusChangedCallback(
      FocusChangedCallback callback) override;
  std::unique_ptr<ScopedAccessibilityMode> CreateScopedModeForProcess(
      ui::AXMode mode) override;
  std::unique_ptr<ScopedAccessibilityMode> CreateScopedModeForBrowserContext(
      BrowserContext* browser_context,
      ui::AXMode mode) override;
  std::unique_ptr<ScopedAccessibilityMode> CreateScopedModeForWebContents(
      WebContents* web_contents,
      ui::AXMode mode) override;
  void SetAXModeChangeAllowed(bool allowed) override;
  bool IsAXModeChangeAllowed() const override;
  void NotifyWebContentsPreferencesChanged() const override;

  // ui::AXPlatform::Delegate:
  ui::AXMode GetProcessMode() override;
  void SetProcessMode(ui::AXMode new_mode) override;
  void OnAccessibilityApiUsage() override;

  // content::RenderWidgetHost::InputEventObserver:
  void OnInputEvent(const RenderWidgetHost& widget,
                    const blink::WebInputEvent& event) override;

  // The global accessibility mode is automatically enabled based on
  // usage of accessibility APIs. When we detect a significant amount
  // of user inputs within a certain time period, but no accessibility
  // API usage, we automatically disable accessibility.
  void OnUserInputEvent();

  // Calls InitBackgroundTasks with short delays for scheduled tasks,
  // and then calls the given completion callback when done.
  void CallInitBackgroundTasksForTesting(base::RepeatingClosure done_callback);

  // Notifies listeners that the focused element changed inside a WebContents.
  void OnFocusChangedInPage(const FocusedNodeDetails& details);

  // Return true if auto-disable should be blocked.
  bool ShouldBlockAutoDisable();

 protected:
  BrowserAccessibilityStateImpl();

  // Called a short while after startup to allow time for the accessibility
  // state to be determined. Updates histograms with the current state.
  // Two variants - one for things that must be run on the UI thread, and
  // another that can be run on another thread.
  virtual void UpdateHistogramsOnUIThread();
  virtual void UpdateHistogramsOnOtherThread();

  // The presence of an AssistiveTech is still unknown.
  // Well be updated via SetKnownScreenReaderAppActive() or
  // UpdateKnownAssistiveTechSlow().
  bool awaiting_known_assistive_tech_computation_ = true;

 private:
  // Called by `OnScreenReaderStopped` as a delayed task. If accessibility
  // support has not been re-enabled by the time the delay has expired, we clear
  // `process_accessibility_mode_` so that all WebContentses are updated.
  void MaybeResetAccessibilityMode();

  void OnOtherThreadDone();

  void UpdateAccessibilityActivityTask();

  // Handles a change to the effective accessibility mode for the process.
  void OnModeChangedForProcess(ui::AXMode old_mode, ui::AXMode new_mode);

  // Handles a change to the effective accessibility mode for `browser_context`.
  void OnModeChangedForBrowserContext(BrowserContext* browser_context,
                                      ui::AXMode old_mode,
                                      ui::AXMode new_mode);

  // Handles a change to the effective accessibility mode for `web_contents`.
  void OnModeChangedForWebContents(WebContents* web_contents,
                                   ui::AXMode old_mode,
                                   ui::AXMode new_mode);

  // The process's single AXPlatform instance.
  ui::AXPlatform ax_platform_;

  base::TimeDelta histogram_delay_;

  std::vector<base::OnceClosure> ui_thread_histogram_callbacks_;
  std::vector<base::OnceClosure> other_thread_histogram_callbacks_;

  bool ui_thread_done_ = false;
  bool other_thread_done_ = false;
  base::RepeatingClosure background_thread_done_callback_;

  // Whether there is a pending task to run UpdateAccessibilityActivityTask.
  bool accessibility_update_task_pending_ = false;

  // Whether changes to the AXMode are allowed.
  // Changes are disallowed while running tests or when
  // --force-renderer-accessibility is used on the command line.
  bool allow_ax_mode_changes_ = true;

  // Keeps track of whether performance filtering is allowed for the device.
  // Default is true to defer to feature flag. Value may be set to false by
  // prefs.
  bool performance_filtering_allowed_ = true;

  // Tracks whether the accessibility engine has been used in any form during
  // the current session. Toggled to true when accessibility is first enabled,
  // and never toggled back to false.
  bool has_enabled_accessibility_in_session_ = false;

  // Timer used to track the time between start-up and engine first-use.
  base::ElapsedTimer timer_;

  // The time of the first user input event; if we receive multiple
  // user input events within a 30-second period and no
  base::TimeTicks first_user_input_event_time_;
  int user_input_event_count_ = 0;

  // The time accessibility became active, used to calculate active time.
  base::TimeTicks accessibility_active_start_time_;

  // The time accessibility became inactive, used to calculate inactive time.
  base::TimeTicks accessibility_inactive_start_time_;

  // The last time accessibility was active, used to calculate active time.
  base::TimeTicks accessibility_last_usage_time_;

  // The time accessibility was enabled, for statistics.
  base::TimeTicks accessibility_enabled_time_;

  // The time accessibility was auto-disabled, for statistics.
  base::TimeTicks accessibility_disabled_time_;

  // The time of the most-recent, explicit request to disable accessibility
  // support. This is set in `OnScreenReaderStopped`. We keep track of this
  // in order to prevent destroying and/or (re)creating large accessibility
  // trees in response to an assistive technology being toggled.
  base::TimeTicks disable_accessibility_request_time_;

  base::RepeatingCallbackList<void(const FocusedNodeDetails&)>
      focus_changed_callbacks_;

  // The collection of active ScopedAccessibilityMode instances targeting all
  // WebContentses in the process.
  ScopedModeCollection scoped_modes_for_process_;

  // A ScopedAccessibilityMode that holds the process-wide mode flags modified
  // via ui::AXPlatformNode::NotifyAddAXModeFlags(),
  // AddAccessibilityModeFlags(), RemoveAccessibilityModeFlags(), and
  // ResetAccessibilityMode(); and applies them to all WebContentses in the
  // process. Guaranteed to hold at least an instance with no mode flags set.
  std::unique_ptr<ScopedAccessibilityMode> process_accessibility_mode_;

  base::WeakPtrFactory<BrowserAccessibilityStateImpl> weak_factory_{this};
};

}  // namespace content

#endif  // CONTENT_BROWSER_ACCESSIBILITY_BROWSER_ACCESSIBILITY_STATE_IMPL_H_

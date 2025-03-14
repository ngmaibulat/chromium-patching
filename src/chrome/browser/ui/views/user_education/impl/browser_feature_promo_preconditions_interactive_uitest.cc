// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <ctime>
#include <memory>

#include "base/run_loop.h"
#include "base/test/scoped_feature_list.h"
#include "base/test/simple_test_clock.h"
#include "base/time/time.h"
#include "chrome/browser/autocomplete/chrome_autocomplete_scheme_classifier.h"
#include "chrome/browser/browser_process.h"
#include "chrome/browser/ui/browser_element_identifiers.h"
#include "chrome/browser/ui/toolbar_controller_util.h"
#include "chrome/browser/ui/views/frame/contents_web_view.h"
#include "chrome/browser/ui/views/location_bar/location_bar_view.h"
#include "chrome/browser/ui/views/toolbar/toolbar_controller.h"
#include "chrome/browser/ui/views/toolbar/toolbar_view.h"
#include "chrome/browser/ui/views/user_education/impl/browser_feature_promo_preconditions.h"
#include "chrome/common/webui_url_constants.h"
#include "chrome/test/base/interactive_test_utils.h"
#include "chrome/test/interaction/interactive_browser_test.h"
#include "components/omnibox/browser/autocomplete_controller.h"
#include "components/omnibox/browser/autocomplete_input.h"
#include "components/omnibox/browser/omnibox_controller.h"
#include "components/prefs/pref_service.h"
#include "components/user_education/common/anchor_element_provider.h"
#include "components/user_education/common/feature_promo/feature_promo_precondition.h"
#include "components/user_education/common/feature_promo/feature_promo_result.h"
#include "components/user_education/common/feature_promo/impl/common_preconditions.h"
#include "components/user_education/common/feature_promo/impl/precondition_data.h"
#include "components/user_education/common/user_education_features.h"
#include "components/user_education/common/user_education_storage_service.h"
#include "components/webui/chrome_urls/pref_names.h"
#include "content/public/test/browser_task_environment.h"
#include "content/public/test/browser_test.h"
#include "ui/base/accelerators/accelerator.h"
#include "ui/base/interaction/element_identifier.h"
#include "ui/base/interaction/element_tracker.h"
#include "ui/base/test/ui_controls.h"
#include "ui/events/test/event_generator.h"
#include "ui/gfx/geometry/point.h"
#include "ui/gfx/geometry/rect.h"
#include "ui/gfx/geometry/size.h"
#include "ui/views/interaction/element_tracker_views.h"
#include "ui/views/interaction/interaction_test_util_views.h"
#include "ui/views/test/views_test_utils.h"
#include "ui/views/view.h"
#include "ui/views/widget/widget_utils.h"
#include "url/gurl.h"

class BrowserFeaturePromoPreconditionsUiTest : public InteractiveBrowserTest {
 public:
  BrowserFeaturePromoPreconditionsUiTest() = default;
  ~BrowserFeaturePromoPreconditionsUiTest() override = default;

 protected:
  auto CaptureAnchor(ui::ElementIdentifier id) {
    return AfterShow(id, [this](ui::TrackedElement* el) {
      anchor_element_data_.data() = el;
    });
  }

  auto CheckWindowActiveResult(user_education::FeaturePromoResult expected) {
    return CheckResult(
        [this]() {
          WindowActivePrecondition active_precond;
          user_education::FeaturePromoPrecondition::ComputedData data;
          data.Add(user_education::AnchorElementPrecondition::kAnchorElement,
                   anchor_element_data_);
          return active_precond.CheckPrecondition(data);
        },
        expected);
  }

  user_education::internal::TypedPreconditionData<ui::SafeElementReference>
      anchor_element_data_{
          user_education::AnchorElementPrecondition::kAnchorElement};
};

class WindowActivePreconditionUiTest
    : public BrowserFeaturePromoPreconditionsUiTest {
 public:
  WindowActivePreconditionUiTest() = default;
  ~WindowActivePreconditionUiTest() override = default;

  void SetUpOnMainThread() override {
    BrowserFeaturePromoPreconditionsUiTest::SetUpOnMainThread();
    g_browser_process->local_state()->SetBoolean(
        chrome_urls::kInternalOnlyUisEnabled, true);
  }
};

IN_PROC_BROWSER_TEST_F(WindowActivePreconditionUiTest, ElementInActiveBrowser) {
  RunTestSequence(
      CaptureAnchor(kToolbarAppMenuButtonElementId),
      CheckWindowActiveResult(user_education::FeaturePromoResult::Success()));
}

IN_PROC_BROWSER_TEST_F(WindowActivePreconditionUiTest,
                       ElementInInactiveBrowser) {
  auto* const incog = CreateIncognitoBrowser();
  RunTestSequence(
      WaitForShow(kToolbarAppMenuButtonElementId),
      SetOnIncompatibleAction(OnIncompatibleAction::kSkipTest,
                              "Linux window activation issues."),
      InContext(incog->window()->GetElementContext(),
                Steps(WaitForShow(kToolbarAppMenuButtonElementId),
                      ActivateSurface(kToolbarAppMenuButtonElementId))),
      WithElement(kToolbarAppMenuButtonElementId,
                  [this](ui::TrackedElement* anchor) {
                    anchor_element_data_.data() = anchor;
                  }),
      CheckWindowActiveResult(
          user_education::FeaturePromoResult::kBlockedByUi));
}

IN_PROC_BROWSER_TEST_F(WindowActivePreconditionUiTest, PageInActiveTab) {
  DEFINE_LOCAL_ELEMENT_IDENTIFIER_VALUE(kTabId);
  RunTestSequence(
      InstrumentTab(kTabId),
      NavigateWebContents(kTabId,
                          GURL(chrome::kChromeUIUserEducationInternalsURL)),
      InAnyContext(CaptureAnchor(kWebUIIPHDemoElementIdentifier)),
      CheckWindowActiveResult(user_education::FeaturePromoResult::Success()));
}

IN_PROC_BROWSER_TEST_F(WindowActivePreconditionUiTest, PageInInactiveTab) {
  DEFINE_LOCAL_ELEMENT_IDENTIFIER_VALUE(kTabId1);
  DEFINE_LOCAL_ELEMENT_IDENTIFIER_VALUE(kTabId2);
  RunTestSequence(
      InstrumentTab(kTabId1),
      AddInstrumentedTab(kTabId2,
                         GURL(chrome::kChromeUIUserEducationInternalsURL)),
      InAnyContext(CaptureAnchor(kWebUIIPHDemoElementIdentifier)),

      // Switch away from the tab. It is no longer "active".
      SelectTab(kTabStripElementId, 0),
      CheckWindowActiveResult(user_education::FeaturePromoResult::kBlockedByUi),

      // Switch back to the tab and verify that it is "active" again.
      SelectTab(kTabStripElementId, 1),
      // Since the element is recreated, need to capture again.
      InAnyContext(CaptureAnchor(kWebUIIPHDemoElementIdentifier)),
      CheckWindowActiveResult(user_education::FeaturePromoResult::Success()));
}

using OmniboxNotOpenPreconditionUiTest = BrowserFeaturePromoPreconditionsUiTest;

IN_PROC_BROWSER_TEST_F(OmniboxNotOpenPreconditionUiTest,
                       CheckOmniboxClosedAndOpen) {
  RunTestSequence(
      CheckView(
          kBrowserViewElementId,
          [](BrowserView* browser_view) {
            OmniboxNotOpenPrecondition precond(*browser_view);
            user_education::FeaturePromoPrecondition::ComputedData data;
            return precond.CheckPrecondition(data);
          },
          user_education::FeaturePromoResult::Success()),
      WithView(
          kBrowserViewElementId,
          [](BrowserView* browser_view) {
            AutocompleteInput input(
                u"chrome", metrics::OmniboxEventProto::NTP,
                ChromeAutocompleteSchemeClassifier(browser_view->GetProfile()));
            browser_view->GetLocationBarView()
                ->GetOmniboxView()
                ->controller()
                ->autocomplete_controller()
                ->Start(input);
          }),
      CheckView(
          kBrowserViewElementId,
          [](BrowserView* browser_view) {
            OmniboxNotOpenPrecondition precond(*browser_view);
            user_education::FeaturePromoPrecondition::ComputedData data;
            return precond.CheckPrecondition(data);
          },
          user_education::FeaturePromoResult::kBlockedByUi));
}

class ToolbarNotCollapsedPreconditionUiTest
    : public BrowserFeaturePromoPreconditionsUiTest {
 public:
  ToolbarNotCollapsedPreconditionUiTest() {
    ToolbarControllerUtil::SetPreventOverflowForTesting(false);
  }
  ~ToolbarNotCollapsedPreconditionUiTest() override = default;

 private:
  base::test::ScopedFeatureList feature_list_;
};

IN_PROC_BROWSER_TEST_F(ToolbarNotCollapsedPreconditionUiTest,
                       ToolbarExpandedAndCollapsed) {
  RunTestSequence(
      CheckView(
          kBrowserViewElementId,
          [](BrowserView* browser_view) {
            ToolbarNotCollapsedPrecondition precond(*browser_view);
            user_education::FeaturePromoPrecondition::ComputedData data;
            return precond.CheckPrecondition(data);
          },
          user_education::FeaturePromoResult::Success()),

      // Add elements to the toolbar until something collapses.
      WithView(
          kBrowserViewElementId,
          [](BrowserView* browser_view) {
            const ToolbarController* const controller =
                browser_view->toolbar()->toolbar_controller();
            CHECK(controller);
            auto* const forward_button =
                views::ElementTrackerViews::GetInstance()->GetFirstMatchingView(
                    kToolbarForwardButtonElementId,
                    browser_view->GetElementContext());
            auto* const container_view =
                views::ElementTrackerViews::GetInstance()->GetFirstMatchingView(
                    ToolbarView::kToolbarContainerElementId,
                    browser_view->GetElementContext());
            constexpr gfx::Size kButtonSize{16, 16};
            while (forward_button->GetVisible()) {
              auto* const button = container_view->AddChildView(
                  std::make_unique<ToolbarButton>());
              button->SetPreferredSize(kButtonSize);
              button->SetMinSize(kButtonSize);
              button->GetViewAccessibility().SetName(u"dummy");
              button->SetVisible(true);
              views::test::RunScheduledLayout(browser_view);
            }
          }),
      WaitForShow(kToolbarOverflowButtonElementId),

      CheckView(
          kBrowserViewElementId,
          [](BrowserView* browser_view) {
            ToolbarNotCollapsedPrecondition precond(*browser_view);
            user_education::FeaturePromoPrecondition::ComputedData data;
            return precond.CheckPrecondition(data);
          },
          user_education::FeaturePromoResult::kBlockedByUi));
}

using BrowserNotClosingPreconditionUiTest =
    BrowserFeaturePromoPreconditionsUiTest;

IN_PROC_BROWSER_TEST_F(BrowserNotClosingPreconditionUiTest,
                       BrowserClosingOrNotClosing) {
  RunTestSequence(
      WaitForShow(kBrowserViewElementId),
      CheckView(
          kBrowserViewElementId,
          [](BrowserView* browser_view) {
            BrowserNotClosingPrecondition precond(*browser_view);
            user_education::FeaturePromoPrecondition::ComputedData data;
            return precond.CheckPrecondition(data);
          },
          user_education::FeaturePromoResult::Success()),
      CheckView(
          kBrowserViewElementId,
          [](BrowserView* browser_view) {
            BrowserNotClosingPrecondition precond(*browser_view);
            user_education::FeaturePromoPrecondition::ComputedData data;
            browser_view->GetWidget()->Close();
            return precond.CheckPrecondition(data);
          },
          user_education::FeaturePromoResult::kBlockedByUi)
          .SetMustRemainVisible(false));
}

class UserNotActivePreconditionUiTest
    : public BrowserFeaturePromoPreconditionsUiTest {
 public:
  UserNotActivePreconditionUiTest() = default;
  ~UserNotActivePreconditionUiTest() override = default;

  void SetUpOnMainThread() override {
    BrowserFeaturePromoPreconditionsUiTest::SetUpOnMainThread();
    less_than_activity_time_ =
        user_education::features::GetIdleTimeBeforeHeavyweightPromo() / 2;
    more_than_activity_time_ =
        user_education::features::GetIdleTimeBeforeHeavyweightPromo() +
        base::Seconds(1);
    auto* const browser_view = BrowserView::GetBrowserViewForBrowser(browser());
    time_provider_.set_clock_for_testing(&test_clock_);
    precondition_ = std::make_unique<UserNotActivePrecondition>(*browser_view,
                                                                time_provider_);
    test_clock_.Advance(more_than_activity_time_);
    event_generator_ = std::make_unique<ui::test::EventGenerator>(
        views::GetRootWindow(browser_view->GetWidget()),
        browser_view->GetNativeWindow());
  }

  void TearDownOnMainThread() override {
    precondition_.reset();
    BrowserFeaturePromoPreconditionsUiTest::TearDownOnMainThread();
  }

  auto Advance(base::TimeDelta time) {
    return Do([this, time]() { test_clock_.Advance(time); })
        .SetDescription("Advance()");
  }

  auto CheckPrecondResult(user_education::FeaturePromoResult result) {
    return CheckView(
        kBrowserViewElementId,
        [this](BrowserView* browser_view) {
          user_education::FeaturePromoPrecondition::ComputedData data;
          return precondition_->CheckPrecondition(data);
        },
        result);
  }

 protected:
  base::TimeDelta less_than_activity_time_;
  base::TimeDelta more_than_activity_time_;

  base::SimpleTestClock test_clock_;
  user_education::UserEducationTimeProvider time_provider_;
  std::unique_ptr<UserNotActivePrecondition> precondition_;
  std::unique_ptr<ui::test::EventGenerator> event_generator_;
};

IN_PROC_BROWSER_TEST_F(UserNotActivePreconditionUiTest, ReturnsSuccess) {
  RunTestSequence(
      WaitForShow(kBrowserViewElementId),
      CheckPrecondResult(user_education::FeaturePromoResult::Success()));
}

IN_PROC_BROWSER_TEST_F(UserNotActivePreconditionUiTest,
                       ReturnsBlockedAfterMouseClick) {
  RunTestSequence(
      WaitForShow(kBrowserViewElementId),
      MoveMouseTo(ContentsWebView::kContentsWebViewElementId), ClickMouse(),
      CheckPrecondResult(user_education::FeaturePromoResult::kBlockedByUi),
      Advance(less_than_activity_time_),
      CheckPrecondResult(user_education::FeaturePromoResult::kBlockedByUi),
      Advance(more_than_activity_time_),
      CheckPrecondResult(user_education::FeaturePromoResult::Success()));
}

IN_PROC_BROWSER_TEST_F(UserNotActivePreconditionUiTest,
                       ReturnsSuccessWhenHoveringOutsideTopContainer) {
  gfx::Point start;
  gfx::Point finish;
  RunTestSequence(
      WaitForShow(kBrowserViewElementId),
      WithView(ContentsWebView::kContentsWebViewElementId,
               [&](views::View* contents) {
                 // Pick a start and end point at opposite corners of the
                 // contents pane, inset into the pane slightly.
                 auto bounds = contents->GetBoundsInScreen();
                 bounds.Inset(3);
                 start = bounds.origin();
                 finish = bounds.bottom_right();
               }),
      // Move to the starting point.
      MoveMouseTo(std::ref(start)),
      // Since the move might pass through the top container, wait long enough
      // that it doesn't matter.
      Advance(more_than_activity_time_),
      CheckPrecondResult(user_education::FeaturePromoResult::Success()),
      // Move to the ending point. Since the move does not pass through the top
      // container, this should not affect the precondition.
      MoveMouseTo(std::ref(finish)),
      CheckPrecondResult(user_education::FeaturePromoResult::Success()));
}

IN_PROC_BROWSER_TEST_F(UserNotActivePreconditionUiTest,
                       ReturnsBlockedAfterKeyPress) {
  RunTestSequence(
      WaitForShow(kBrowserViewElementId), Check([this]() {
        return ui_test_utils::SendKeyPressSync(
            browser(), ui::KeyboardCode::VKEY_A, false, false, false, false);
      }),
      CheckPrecondResult(user_education::FeaturePromoResult::kBlockedByUi),
      Advance(less_than_activity_time_),
      CheckPrecondResult(user_education::FeaturePromoResult::kBlockedByUi),
      Advance(more_than_activity_time_),
      CheckPrecondResult(user_education::FeaturePromoResult::Success()));
}

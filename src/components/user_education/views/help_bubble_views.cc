// Copyright 2025 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/user_education/views/help_bubble_views.h"

#include "base/functional/bind.h"
#include "components/user_education/common/user_education_class_properties.h"
#include "components/user_education/common/user_education_events.h"
#include "components/user_education/views/help_bubble_view.h"
#include "components/user_education/views/toggle_tracked_element_attention_utils.h"
#include "ui/base/interaction/element_identifier.h"
#include "ui/base/interaction/element_tracker.h"
#include "ui/base/interaction/framework_specific_implementation.h"
#include "ui/views/accessible_pane_view.h"
#include "ui/views/interaction/element_tracker_views.h"

namespace user_education {

DEFINE_FRAMEWORK_SPECIFIC_METADATA(HelpBubbleViews)

HelpBubbleViews::HelpBubbleViews(HelpBubbleView* help_bubble_view,
                                 ui::TrackedElement* anchor_element)
    : help_bubble_view_(help_bubble_view), anchor_element_(anchor_element) {
  DCHECK(help_bubble_view);
  DCHECK(help_bubble_view->GetWidget());
  scoped_observation_.Observe(help_bubble_view->GetWidget());

  anchor_hidden_subscription_ =
      ui::ElementTracker::GetElementTracker()->AddElementHiddenCallback(
          anchor_element->identifier(), anchor_element->context(),
          base::BindRepeating(&HelpBubbleViews::OnElementHidden,
                              base::Unretained(this)));
  anchor_bounds_changed_subscription_ =
      ui::ElementTracker::GetElementTracker()->AddCustomEventCallback(
          kHelpBubbleAnchorBoundsChangedEvent, anchor_element->context(),
          base::BindRepeating(&HelpBubbleViews::OnElementBoundsChanged,
                              base::Unretained(this)));
}

HelpBubbleViews::~HelpBubbleViews() {
  // Needs to be called here while we still have access to HelpBubbleViews-
  // specific logic.
  Close(CloseReason::kBubbleDestroyed);
}

bool HelpBubbleViews::ToggleFocusForAccessibility() {
  // // If the bubble isn't present or can't be meaningfully focused, stop.
  if (!help_bubble_view_) {
    return false;
  }

  // If the focus isn't in the help bubble, focus the help bubble.
  // Note that if is_focus_in_ancestor_widget is true, then anchor both exists
  // and has a widget, so anchor->GetWidget() will always be valid.
  if (!help_bubble_view_->IsFocusInHelpBubble()) {
    help_bubble_view_->GetWidget()->Activate();
    help_bubble_view_->RequestFocus();
    return true;
  }

  auto* const anchor = help_bubble_view_->GetAnchorView();
  if (!anchor) {
    return false;
  }

  bool set_focus = false;
  if (anchor->GetViewAccessibility().IsAccessibilityFocusable()) {
#if BUILDFLAG(IS_MAC)
    // Mac does not automatically pass activation on focus, so we have to do it
    // manually.
    anchor->GetWidget()->Activate();
#else
    // Focus the anchor. We can't request focus for an accessibility-only view
    // until we turn on keyboard accessibility for its focus manager.
    anchor->GetFocusManager()->SetKeyboardAccessible(true);
#endif
    anchor->RequestFocus();
    set_focus = true;
  } else if (views::IsViewClass<views::AccessiblePaneView>(anchor)) {
    // An AccessiblePaneView can receive focus, but is not necessarily itself
    // accessibility focusable. Use the built-in functionality for focusing
    // elements of AccessiblePaneView instead.
#if BUILDFLAG(IS_MAC)
    // Mac does not automatically pass activation on focus, so we have to do it
    // manually.
    anchor->GetWidget()->Activate();
#else
    // You can't focus an accessible pane if it's already in accessibility
    // mode, so avoid doing that; the SetPaneFocus() call will go back into
    // accessibility navigation mode.
    anchor->GetFocusManager()->SetKeyboardAccessible(false);
#endif
    set_focus =
        static_cast<views::AccessiblePaneView*>(anchor)->SetPaneFocus(nullptr);
  }

  return set_focus;
}

void HelpBubbleViews::OnAnchorBoundsChanged() {
  if (help_bubble_view_) {
    help_bubble_view_->OnAnchorBoundsChanged();
  }
}

gfx::Rect HelpBubbleViews::GetBoundsInScreen() const {
  return help_bubble_view_
             ? help_bubble_view_->GetWidget()->GetWindowBoundsInScreen()
             : gfx::Rect();
}

ui::ElementContext HelpBubbleViews::GetContext() const {
  return help_bubble_view_
             ? views::ElementTrackerViews::GetContextForView(help_bubble_view_)
             : ui::ElementContext();
}

bool HelpBubbleViews::AcceleratorPressed(const ui::Accelerator& accelerator) {
  if (CanHandleAccelerators()) {
    ToggleFocusForAccessibility();
    return true;
  }

  return false;
}

bool HelpBubbleViews::CanHandleAccelerators() const {
  return help_bubble_view_ && help_bubble_view_->GetWidget() &&
         help_bubble_view_->GetWidget()->IsActive();
}

void HelpBubbleViews::MaybeResetAnchorView() {
  if (!help_bubble_view_) {
    return;
  }
  auto* const anchor_view = help_bubble_view_->GetAnchorView();
  if (!anchor_view) {
    return;
  }
  anchor_view->SetProperty(kHasInProductHelpPromoKey, false);
  MaybeRemoveAttentionStateFromTrackedElement(anchor_view);
}

void HelpBubbleViews::CloseBubbleImpl() {
  anchor_hidden_subscription_ = base::CallbackListSubscription();
  anchor_bounds_changed_subscription_ = base::CallbackListSubscription();
  scoped_observation_.Reset();
  MaybeResetAnchorView();

  // Reset the anchor view. Closing the widget could cause callbacks which could
  // theoretically destroy `this`, so
  auto* const help_bubble_view = help_bubble_view_.get();
  help_bubble_view_ = nullptr;
  anchor_element_ = nullptr;
  if (help_bubble_view && help_bubble_view->GetWidget()) {
    help_bubble_view->GetWidget()->Close();
  }
}

void HelpBubbleViews::OnWidgetDestroying(views::Widget* widget) {
  Close(CloseReason::kBubbleElementDestroyed);
}

void HelpBubbleViews::OnElementHidden(ui::TrackedElement* element) {
  // There could be other elements with the same identifier as the anchor
  // element, so don't close the bubble unless it is actually the anchor.
  if (element != anchor_element_) {
    return;
  }

  anchor_hidden_subscription_ = base::CallbackListSubscription();
  anchor_bounds_changed_subscription_ = base::CallbackListSubscription();
  anchor_element_ = nullptr;
  Close(CloseReason::kAnchorHidden);
}

void HelpBubbleViews::OnElementBoundsChanged(ui::TrackedElement* element) {
  if (help_bubble_view_ && element == anchor_element_) {
    help_bubble_view_->SetForceAnchorRect(element->GetScreenBounds());
    OnAnchorBoundsChanged();
  }
}

}  // namespace user_education

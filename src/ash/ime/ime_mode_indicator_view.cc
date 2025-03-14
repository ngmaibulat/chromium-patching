// Copyright 2018 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ash/ime/ime_mode_indicator_view.h"

#include "ash/public/cpp/shell_window_ids.h"
#include "ash/shell.h"
#include "ash/wm/window_util.h"
#include "base/logging.h"
#include "ui/base/metadata/metadata_impl_macros.h"
#include "ui/base/mojom/dialog_button.mojom.h"
#include "ui/display/display.h"
#include "ui/display/screen.h"
#include "ui/views/bubble/bubble_frame_view.h"
#include "ui/views/controls/label.h"
#include "ui/views/layout/fill_layout.h"
#include "ui/wm/core/window_animations.h"

namespace ash {

namespace {

// Minimum size of inner contents in pixel.
// 43 is the designed size including the default margin (6 * 2).
const int kMinSize = 31;

// After this duration in msec, the mode inicator will be fading out.
const int kShowingDuration = 500;

class ModeIndicatorFrameView : public views::BubbleFrameView {
  METADATA_HEADER(ModeIndicatorFrameView, views::BubbleFrameView)

 public:
  explicit ModeIndicatorFrameView()
      : views::BubbleFrameView(gfx::Insets(), gfx::Insets()) {}
  ModeIndicatorFrameView(const ModeIndicatorFrameView&) = delete;
  ModeIndicatorFrameView& operator=(const ModeIndicatorFrameView&) = delete;
  ~ModeIndicatorFrameView() override {}

 private:
  // views::BubbleFrameView overrides:
  gfx::Rect GetAvailableScreenBounds(const gfx::Rect& rect) const override {
    return display::Screen::GetScreen()
        ->GetDisplayNearestPoint(rect.CenterPoint())
        .bounds();
  }
};

BEGIN_METADATA(ModeIndicatorFrameView)
END_METADATA

}  // namespace

ImeModeIndicatorView::ImeModeIndicatorView(const gfx::Rect& cursor_bounds,
                                           const std::u16string& label)
    : cursor_bounds_(cursor_bounds), label_view_(new views::Label(label)) {
  SetButtons(static_cast<int>(ui::mojom::DialogButton::kNone));
  SetCanActivate(false);
  set_accept_events(false);
  set_shadow(views::BubbleBorder::STANDARD_SHADOW);
  SetArrow(views::BubbleBorder::TOP_CENTER);
  // Ignore this view for accessibility purposes.
  SetAccessibleWindowRole(ax::mojom::Role::kNone);
}

ImeModeIndicatorView::~ImeModeIndicatorView() = default;

void ImeModeIndicatorView::ShowAndFadeOut() {
  ::wm::SetWindowVisibilityAnimationTransition(GetWidget()->GetNativeView(),
                                               ::wm::ANIMATE_HIDE);
  GetWidget()->Show();
  timer_.Start(FROM_HERE, base::Milliseconds(kShowingDuration), GetWidget(),
               &views::Widget::Close);
}

void ImeModeIndicatorView::OnBeforeBubbleWidgetInit(
    views::Widget::InitParams* params,
    views::Widget* widget) const {
  aura::Window* window = window_util::GetActiveWindow();
  if (window) {  // Null check for tests.
    params->parent = Shell::GetContainer(window->GetRootWindow(),
                                         kShellWindowId_SettingBubbleContainer);
  } else {
    params->parent = Shell::GetPrimaryRootWindow();
  }
}

gfx::Size ImeModeIndicatorView::CalculatePreferredSize(
    const views::SizeBounds& available_size) const {
  gfx::Size size = label_view_->GetPreferredSize({});
  size.SetToMax(gfx::Size(kMinSize, kMinSize));
  return size;
}

void ImeModeIndicatorView::Init() {
  SetLayoutManager(std::make_unique<views::FillLayout>());
  AddChildViewRaw(label_view_.get());

  SetAnchorRect(cursor_bounds_);
}

std::unique_ptr<views::NonClientFrameView>
ImeModeIndicatorView::CreateNonClientFrameView(views::Widget* widget) {
  auto frame = std::make_unique<ModeIndicatorFrameView>();
  // arrow adjustment in BubbleDialogDelegateView is unnecessary because arrow
  // of this bubble is always center.
  auto border = std::make_unique<views::BubbleBorder>(arrow(), GetShadow());
  border->SetColor(background_color());
  frame->SetBubbleBorder(std::move(border));
  return frame;
}

BEGIN_METADATA(ImeModeIndicatorView)
END_METADATA

}  // namespace ash

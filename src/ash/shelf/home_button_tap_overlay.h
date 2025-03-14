// Copyright 2017 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ASH_SHELF_HOME_BUTTON_TAP_OVERLAY_H_
#define ASH_SHELF_HOME_BUTTON_TAP_OVERLAY_H_

#include <memory>

#include "ash/ash_export.h"
#include "ash/shelf/home_button.h"
#include "base/memory/raw_ptr.h"
#include "ui/base/metadata/metadata_header_macros.h"
#include "ui/compositor/layer_animation_observer.h"
#include "ui/views/animation/ink_drop_painted_layer_delegates.h"
#include "ui/views/view.h"

namespace ash {

// Overlay to display animations when the home button is tapped (with a touch
// screen).
// Used for features like Assistant.
class ASH_EXPORT HomeButtonTapOverlay : public views::View,
                                        public ui::ImplicitAnimationObserver {
  METADATA_HEADER(HomeButtonTapOverlay, views::View)

 public:
  explicit HomeButtonTapOverlay(HomeButton* host_view);

  HomeButtonTapOverlay(const HomeButtonTapOverlay&) = delete;
  HomeButtonTapOverlay& operator=(const HomeButtonTapOverlay&) = delete;

  ~HomeButtonTapOverlay() override;

  void StartAnimation();
  void EndAnimation();
  void BurstAnimation();
  void HideAnimation();
  bool IsBursting() const {
    return AnimationState::BURSTING == animation_state_;
  }
  bool IsHidden() const { return AnimationState::HIDDEN == animation_state_; }

  // views::View:
  void OnThemeChanged() override;

  // ui::ImplicitAnimationObserver
  void OnImplicitAnimationsCompleted() override;

 private:
  enum class AnimationState {
    // Indicates no animation is playing.
    HIDDEN = 0,
    // Indicates currently playing the starting animation.
    STARTING,
    // Indicates the current animation is in the bursting phase, which means no
    // turning back.
    BURSTING
  };

  std::unique_ptr<ui::Layer> ripple_layer_;

  raw_ptr<HomeButton> host_view_;

  AnimationState animation_state_ = AnimationState::HIDDEN;

  views::CircleLayerDelegate circle_layer_delegate_;
  std::unique_ptr<HomeButton::ScopedNoClipRect> scoped_no_clip_rect_;
};

}  // namespace ash
#endif  // ASH_SHELF_HOME_BUTTON_TAP_OVERLAY_H_

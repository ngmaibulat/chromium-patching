// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef THIRD_PARTY_BLINK_RENDERER_MODULES_MEDIA_CONTROLS_ELEMENTS_MEDIA_CONTROL_TRACK_SELECTOR_MENU_BUTTON_ELEMENT_H_
#define THIRD_PARTY_BLINK_RENDERER_MODULES_MEDIA_CONTROLS_ELEMENTS_MEDIA_CONTROL_TRACK_SELECTOR_MENU_BUTTON_ELEMENT_H_

#include "third_party/blink/public/platform/web_media_player.h"
#include "third_party/blink/renderer/modules/media_controls/elements/media_control_input_element.h"
#include "third_party/blink/renderer/modules/modules_export.h"

namespace blink {

class Event;
class MediaControlsImpl;

class MODULES_EXPORT MediaControlTrackSelectorMenuButtonElement final
    : public MediaControlInputElement {
 public:
  explicit MediaControlTrackSelectorMenuButtonElement(
      MediaControlsImpl&,
      WebMediaPlayer::TrackType);

  // MediaControlInputElement overrides.
  bool WillRespondToMouseClickEvents() override;
  int GetOverflowStringId() const override;
  bool HasOverflowButton() const override;

 protected:
  const char* GetNameForHistograms() const override;

 private:
  void DefaultEventHandler(Event&) override;

  WebMediaPlayer::TrackType type_;
};

}  // namespace blink

#endif  // THIRD_PARTY_BLINK_RENDERER_MODULES_MEDIA_CONTROLS_ELEMENTS_MEDIA_CONTROL_TRACK_SELECTOR_MENU_BUTTON_ELEMENT_H_

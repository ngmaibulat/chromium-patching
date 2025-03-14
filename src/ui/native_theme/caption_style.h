// Copyright 2018 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_NATIVE_THEME_CAPTION_STYLE_H_
#define UI_NATIVE_THEME_CAPTION_STYLE_H_

#include <optional>
#include <string>

#include "base/component_export.h"
#include "third_party/skia/include/core/SkColor.h"
#include "third_party/skia/include/core/SkScalar.h"

namespace ui {

struct COMPONENT_EXPORT(NATIVE_THEME) CaptionStyle {
  CaptionStyle();
  CaptionStyle(const CaptionStyle& other);
  ~CaptionStyle();

  // Returns a CaptionStyle parsed from a specification string, which is a
  // serialized JSON object whose keys are strings and whose values are of
  // variable types. See the body of this method for details. This is used to
  // parse the value of the "--force-caption-style" command-line argument and
  // for testing.
  static std::optional<CaptionStyle> FromSpec(const std::string& spec);

  // Returns a CaptionStyle populated from the System's Settings.
  static std::optional<CaptionStyle> FromSystemSettings();

  // Some or all of these property strings can be empty.
  // For example, on Win10 in Settings when a property is set to Default, the
  // corresponding string here stays empty. This allows the author styling on
  // the webpage to be applied. As the user hasn't specified a preferred style,
  // we pass along an empty string from here.
  std::string text_color;
  std::string background_color;
  // Holds text size percentage as a css string.
  std::string text_size;
  std::string text_shadow;
  std::string font_family;
  std::string font_variant;
  std::string window_color;
  std::string window_radius;
};

}  // namespace ui

#endif  // UI_NATIVE_THEME_CAPTION_STYLE_H_

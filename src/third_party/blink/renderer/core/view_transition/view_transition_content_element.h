// Copyright 2021 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef THIRD_PARTY_BLINK_RENDERER_CORE_VIEW_TRANSITION_VIEW_TRANSITION_CONTENT_ELEMENT_H_
#define THIRD_PARTY_BLINK_RENDERER_CORE_VIEW_TRANSITION_VIEW_TRANSITION_CONTENT_ELEMENT_H_

#include "base/memory/scoped_refptr.h"
#include "components/viz/common/view_transition_element_resource_id.h"
#include "third_party/blink/renderer/core/core_export.h"
#include "third_party/blink/renderer/core/dom/pseudo_element.h"
#include "third_party/blink/renderer/core/layout/geometry/physical_rect.h"
#include "third_party/blink/renderer/core/view_transition/view_transition_pseudo_element_base.h"

namespace blink {

// This class implements the functionality to display a live or cached snapshot
// of an element created using content:element(id).
// The element function is described at
// https://developer.mozilla.org/en-US/docs/Web/CSS/element().
class CORE_EXPORT ViewTransitionContentElement
    : public ViewTransitionPseudoElementBase {
 public:
  explicit ViewTransitionContentElement(
      Element* parent,
      PseudoId,
      const AtomicString& view_transition_name,
      viz::ViewTransitionElementResourceId,
      bool is_live_content_element,
      bool is_generated_name,
      const ViewTransitionStyleTracker* style_tracker);
  ~ViewTransitionContentElement() override;

  void SetIntrinsicSize(
      const gfx::RectF& captured_rect,
      const gfx::RectF& reference_rect_in_enclosing_layer_space,
      bool propagates_max_extents_rect);
  const gfx::RectF& captured_rect() const { return captured_rect_; }
  const gfx::RectF& reference_rect_in_enclosing_layer_space() const {
    return reference_rect_in_enclosing_layer_space_;
  }
  const viz::ViewTransitionElementResourceId& resource_id() const {
    return resource_id_;
  }
  bool is_live_content_element() const { return is_live_content_element_; }

  bool propagate_max_extent_rect() const { return propagate_max_extent_rect_; }

 private:
  LayoutObject* CreateLayoutObject(const ComputedStyle&) override;

  // |resource_id_| is used to generate a foreign layer to substitute this
  // element with a render pass generated by the compositor.
  const viz::ViewTransitionElementResourceId resource_id_;

  // This indicates whether the element represents a live or a cached content.
  const bool is_live_content_element_;

  // The size of the element's texture generated by the compositor.
  gfx::RectF captured_rect_;
  gfx::RectF reference_rect_in_enclosing_layer_space_;
  bool propagate_max_extent_rect_;
};

}  // namespace blink

#endif  // THIRD_PARTY_BLINK_RENDERER_CORE_VIEW_TRANSITION_VIEW_TRANSITION_CONTENT_ELEMENT_H_

// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/renderer/core/view_transition/view_transition_utils.h"

#include "third_party/blink/renderer/core/css/style_engine.h"
#include "third_party/blink/renderer/core/dom/document.h"
#include "third_party/blink/renderer/core/dom/pseudo_element.h"
#include "third_party/blink/renderer/core/layout/layout_view.h"
#include "third_party/blink/renderer/core/paint/paint_layer.h"
#include "third_party/blink/renderer/core/style/computed_style.h"
#include "third_party/blink/renderer/core/style/computed_style_constants.h"
#include "third_party/blink/renderer/core/view_transition/view_transition.h"
#include "third_party/blink/renderer/core/view_transition/view_transition_supplement.h"
#include "third_party/blink/renderer/core/view_transition/view_transition_transition_element.h"
#include "third_party/blink/renderer/platform/runtime_enabled_features.h"

namespace blink {

// static
void ViewTransitionUtils::ForEachTransitionPseudo(Document& document,
                                                  PseudoFunctor func) {
  if (!document.documentElement()) {
    return;
  }

  auto* transition_pseudo =
      document.documentElement()->GetPseudoElement(kPseudoIdViewTransition);
  if (!transition_pseudo) {
    return;
  }

  func(transition_pseudo);

  for (const auto& view_transition_name :
       document.GetStyleEngine().ViewTransitionTags()) {
    auto* container_pseudo =
        To<ViewTransitionTransitionElement>(transition_pseudo)
            ->FindViewTransitionGroupPseudoElement(view_transition_name);
    if (!container_pseudo) {
      continue;
    }

    func(container_pseudo);

    auto* wrapper_pseudo = container_pseudo->GetPseudoElement(
        kPseudoIdViewTransitionImagePair, view_transition_name);
    if (!wrapper_pseudo) {
      continue;
    }

    func(wrapper_pseudo);

    if (auto* content = wrapper_pseudo->GetPseudoElement(
            kPseudoIdViewTransitionOld, view_transition_name)) {
      func(content);
    }

    if (auto* content = wrapper_pseudo->GetPseudoElement(
            kPseudoIdViewTransitionNew, view_transition_name)) {
      func(content);
    }
  }
}

// static
PseudoElement* ViewTransitionUtils::FindPseudoIf(const Document& document,
                                                 PseudoPredicate condition) {
  if (!document.documentElement()) {
    return nullptr;
  }

  auto* transition_pseudo =
      document.documentElement()->GetPseudoElement(kPseudoIdViewTransition);
  if (!transition_pseudo) {
    return nullptr;
  }
  if (condition(transition_pseudo)) {
    return transition_pseudo;
  }

  for (const auto& view_transition_name :
       document.GetStyleEngine().ViewTransitionTags()) {
    auto* container_pseudo =
        To<ViewTransitionTransitionElement>(transition_pseudo)
            ->FindViewTransitionGroupPseudoElement(view_transition_name);
    if (!container_pseudo) {
      continue;
    }
    if (condition(container_pseudo)) {
      return container_pseudo;
    }

    auto* wrapper_pseudo = container_pseudo->GetPseudoElement(
        kPseudoIdViewTransitionImagePair, view_transition_name);
    if (!wrapper_pseudo) {
      continue;
    }
    if (condition(wrapper_pseudo)) {
      return wrapper_pseudo;
    }

    if (auto* content = wrapper_pseudo->GetPseudoElement(
            kPseudoIdViewTransitionOld, view_transition_name);
        content && condition(content)) {
      return content;
    }

    if (auto* content = wrapper_pseudo->GetPseudoElement(
            kPseudoIdViewTransitionNew, view_transition_name);
        content && condition(content)) {
      return content;
    }
  }

  return nullptr;
}

// static
void ViewTransitionUtils::ForEachDirectTransitionPseudo(const Element* element,
                                                        PseudoFunctor func) {
  if (element->IsDocumentElement()) {
    if (auto* pseudo = element->GetPseudoElement(kPseudoIdViewTransition)) {
      func(pseudo);
    }
    return;
  }

  if (!IsTransitionPseudoElement(element->GetPseudoId())) {
    return;
  }

  switch (element->GetPseudoId()) {
    case kPseudoIdViewTransition:
      for (auto name :
           element->GetDocument().GetStyleEngine().ViewTransitionTags()) {
        if (auto* pseudo =
                element->GetPseudoElement(kPseudoIdViewTransitionGroup, name)) {
          func(pseudo);
        }
      }
      break;
    case kPseudoIdViewTransitionGroup:
      if (auto* pseudo =
              element->GetPseudoElement(kPseudoIdViewTransitionImagePair)) {
        func(pseudo);
      }
      break;
    case kPseudoIdViewTransitionImagePair:
      if (auto* pseudo =
              element->GetPseudoElement(kPseudoIdViewTransitionOld)) {
        func(pseudo);
      }
      if (auto* pseudo =
              element->GetPseudoElement(kPseudoIdViewTransitionNew)) {
        func(pseudo);
      }
      break;
    case kPseudoIdViewTransitionOld:
    case kPseudoIdViewTransitionNew:
      break;
    default:
      NOTREACHED();
  }
}

// static
ViewTransition* ViewTransitionUtils::GetTransition(const Document& document) {
  auto* supplement = ViewTransitionSupplement::FromIfExists(document);
  if (!supplement) {
    return nullptr;
  }
  ViewTransition* transition = supplement->GetTransition();
  if (!transition || transition->IsDone()) {
    return nullptr;
  }
  return transition;
}

// static
void ViewTransitionUtils::ForEachTransition(
    const Document& document,
    base::FunctionRef<void(ViewTransition&)> function) {
  if (auto* supplement = ViewTransitionSupplement::FromIfExists(document)) {
    supplement->ForEachTransition(function);
  }
}

// static
ViewTransition* ViewTransitionUtils::GetIncomingCrossDocumentTransition(
    const Document& document) {
  if (auto* transition = GetTransition(document);
      transition && transition->IsForNavigationOnNewDocument()) {
    return transition;
  }
  return nullptr;
}

// static
ViewTransition* ViewTransitionUtils::GetOutgoingCrossDocumentTransition(
    const Document& document) {
  if (auto* transition = GetTransition(document);
      transition && transition->IsForNavigationSnapshot()) {
    return transition;
  }
  return nullptr;
}

// static
DOMViewTransition* ViewTransitionUtils::GetTransitionScriptDelegate(
    const Document& document) {
  ViewTransition* view_transition =
      ViewTransitionUtils::GetTransition(document);
  if (!view_transition) {
    return nullptr;
  }

  return view_transition->GetScriptDelegate();
}

// static
PseudoElement* ViewTransitionUtils::GetRootPseudo(const Document& document) {
  if (!document.documentElement()) {
    return nullptr;
  }

  PseudoElement* view_transition_pseudo =
      document.documentElement()->GetPseudoElement(kPseudoIdViewTransition);
  DCHECK(!view_transition_pseudo || GetTransition(document));
  return view_transition_pseudo;
}

// static
VectorOf<std::unique_ptr<ViewTransitionRequest>>
ViewTransitionUtils::GetPendingRequests(const Document& document) {
  auto* supplement = ViewTransitionSupplement::FromIfExists(document);
  if (supplement) {
    return supplement->TakePendingRequests();
  }
  return {};
}

// static
bool ViewTransitionUtils::IsViewTransitionRoot(const LayoutObject& object) {
  return object.GetNode() &&
         object.GetNode()->GetPseudoId() == kPseudoIdViewTransition;
}

// static
bool ViewTransitionUtils::IsViewTransitionElementExcludingRootFromSupplement(
    const Element& element) {
  ViewTransition* transition = GetTransition(element.GetDocument());
  return transition && transition->IsTransitionElementExcludingRoot(element);
}

// static
bool ViewTransitionUtils::IsViewTransitionParticipantFromSupplement(
    const LayoutObject& object) {
  ViewTransition* transition = GetTransition(object.GetDocument());
  return transition && transition->IsRepresentedViaPseudoElements(object);
}

}  // namespace blink

// Copyright 2021 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/views/view_utils.h"

#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "base/command_line.h"
#include "base/debug/stack_trace.h"
#include "base/logging.h"
#include "base/strings/strcat.h"
#include "ui/views/view.h"
#include "ui/views/views_switches.h"

DEFINE_EXPORTED_UI_CLASS_PROPERTY_TYPE(VIEWS_EXPORT, base::debug::StackTrace*)

namespace views {

DEFINE_OWNED_UI_CLASS_PROPERTY_KEY(base::debug::StackTrace, kViewStackTraceKey)

namespace {

std::string GetViewTreeAsString(View* view) {
  if (!view->parent()) {
    return std::string(view->GetClassName());
  }
  return base::StrCat(
      {GetViewTreeAsString(view->parent()), " -> ", view->GetClassName()});
}

}  // namespace

ViewDebugWrapperImpl::ViewDebugWrapperImpl(View* view) : view_(view) {}

ViewDebugWrapperImpl::~ViewDebugWrapperImpl() = default;

std::string ViewDebugWrapperImpl::GetViewClassName() {
  return std::string(view_->GetClassName());
}

int ViewDebugWrapperImpl::GetID() {
  return view_->GetID();
}

debug::ViewDebugWrapper::BoundsTuple ViewDebugWrapperImpl::GetBounds() {
  const auto& bounds = view_->bounds();
  return BoundsTuple(bounds.x(), bounds.y(), bounds.width(), bounds.height());
}

bool ViewDebugWrapperImpl::GetVisible() {
  return view_->GetVisible();
}

bool ViewDebugWrapperImpl::GetNeedsLayout() {
  return view_->needs_layout();
}

bool ViewDebugWrapperImpl::GetEnabled() {
  return view_->GetEnabled();
}

std::vector<debug::ViewDebugWrapper*> ViewDebugWrapperImpl::GetChildren() {
  children_.clear();
  for (views::View* child : view_->children()) {
    children_.push_back(std::make_unique<ViewDebugWrapperImpl>(child));
  }

  std::vector<debug::ViewDebugWrapper*> child_ptrs;
  for (auto& child : children_) {
    child_ptrs.push_back(child.get());
  }
  return child_ptrs;
}

void ViewDebugWrapperImpl::ForAllProperties(PropCallback callback) {
  views::View* view = const_cast<views::View*>(view_.get());
  for (auto* member : *(view->GetClassMetaData())) {
    auto flags = member->GetPropertyFlags();
    if (!!(flags & ui::metadata::PropertyFlags::kSerializable)) {
      callback.Run(member->member_name(),
                   base::UTF16ToUTF8(member->GetValueAsString(view)));
    }
  }
}

std::string PrintViewHierarchy(View* view, bool verbose) {
  ViewDebugWrapperImpl debug_view(view);
  return debug::PrintViewHierarchy(&debug_view, verbose);
}

std::string GetViewDebugInfo(View* view) {
  std::string debug_string =
      base::StrCat({"\nView: ", view->GetClassName(), "\nHierarchy:\n    ",
                    GetViewTreeAsString(view), "\nView created here:\n"});

  static bool has_stack_trace =
      base::CommandLine::ForCurrentProcess()->HasSwitch(
          switches::kViewStackTraces);
  if (has_stack_trace) {
    debug_string += view->GetProperty(kViewStackTraceKey)->ToString();
  } else {
    debug_string +=
        base::StrCat({"  Run with --", switches::kViewStackTraces,
                      " to get a stack trace for when this View was created."});
  }

  return debug_string;
}

}  // namespace views

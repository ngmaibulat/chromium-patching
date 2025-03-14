// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ash/app_list/model/search/search_box_model.h"

#include <string>
#include <utility>

#include "ash/app_list/model/search/search_box_model_observer.h"
#include "ash/public/cpp/app_list/app_list_client.h"
#include "base/metrics/histogram_macros.h"
#include "base/metrics/user_metrics.h"

namespace ash {

SearchBoxModel::SearchBoxModel() = default;

SearchBoxModel::~SearchBoxModel() = default;

void SearchBoxModel::SetShowAssistantButton(bool show) {
  if (show_assistant_button_ == show) {
    return;
  }

  show_assistant_button_ = show;

  CHECK(!show_assistant_button_ || !show_assistant_new_entry_point_button_)
      << "Only one of AssistantButton or AssistantNewEntryPointButton can be "
         "shown";

  for (auto& observer : observers_) {
    observer.ShowAssistantChanged();
  }
}

void SearchBoxModel::SetShowAssistantNewEntryPointButton(
    bool show,
    const std::string& name) {
  if (show_assistant_new_entry_point_button_ == show) {
    CHECK_EQ(assistant_new_entry_point_name_, name)
        << "Currently changing only name is not supported";

    return;
  }

  show_assistant_new_entry_point_button_ = show;
  assistant_new_entry_point_name_ = name;

  CHECK_EQ(!name.empty(), show)
      << "Name must be set if assistant new entry button is shown.";

  CHECK(!show_assistant_button_ || !show_assistant_new_entry_point_button_)
      << "Only one of AssistantButton or AssistantNewEntryPointButton can be "
         "shown";

  for (SearchBoxModelObserver& observer : observers_) {
    observer.ShowAssistantNewEntryPointChanged();
  }
}

void SearchBoxModel::SetSunfishButtonVisibility(
    SearchBoxModel::SunfishButtonVisibility show) {
  if (sunfish_button_visibility_ == show) {
    return;
  }
  sunfish_button_visibility_ = show;
  for (SearchBoxModelObserver& observer : observers_) {
    observer.SunfishButtonVisibilityChanged();
  }
}

void SearchBoxModel::SetWouldTriggerIph(bool would_trigger_iph) {
  if (would_trigger_iph_ == would_trigger_iph) {
    return;
  }

  would_trigger_iph_ = would_trigger_iph;
}

void SearchBoxModel::SetSearchEngineIsGoogle(bool is_google) {
  if (is_google == search_engine_is_google_)
    return;
  search_engine_is_google_ = is_google;
  for (auto& observer : observers_)
    observer.SearchEngineChanged();
}

void SearchBoxModel::AddObserver(SearchBoxModelObserver* observer) {
  observers_.AddObserver(observer);
}

void SearchBoxModel::RemoveObserver(SearchBoxModelObserver* observer) {
  observers_.RemoveObserver(observer);
}

}  // namespace ash

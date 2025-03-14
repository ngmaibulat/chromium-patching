// Copyright 2020 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_PERFORMANCE_MANAGER_TEST_SUPPORT_DECORATORS_UTILS_H_
#define COMPONENTS_PERFORMANCE_MANAGER_TEST_SUPPORT_DECORATORS_UTILS_H_

#include "components/performance_manager/public/graph/page_node.h"
#include "components/performance_manager/public/performance_manager.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace performance_manager {

namespace testing {

// Helper function that allows testing that a decorator class property has the
// expected value. This function should be called from the main thread and be
// passed the WebContents pointer associated with the PageNode to check.
template <class T>
void TestPageNodeProperty(content::WebContents* contents,
                          T* (*data_getter)(const PageNode*),
                          bool (T::*getter)() const,
                          bool expected_value);

// Helper function that simulates a change in a property of a page node
// decorator and test if the property gets update.
template <class T>
void EndToEndBooleanPropertyTest(content::WebContents* contents,
                                 T* (*data_getter)(const PageNode*),
                                 bool (T::*pm_getter)() const,
                                 void (*setter)(content::WebContents*, bool),
                                 bool default_state = false);

// Implementation details:

template <class T>
void TestPageNodeProperty(content::WebContents* contents,
                          T* (*data_getter)(const PageNode*),
                          bool (T::*getter)() const,
                          bool expected_value) {
  base::WeakPtr<PageNode> node =
      PerformanceManager::GetPrimaryPageNodeForWebContents(contents);

  EXPECT_TRUE(node);
  T* data = (*data_getter)(node.get());
  EXPECT_TRUE(data);
  EXPECT_EQ((data->*getter)(), expected_value);
}

template <class T>
void EndToEndBooleanPropertyTest(content::WebContents* contents,
                                 T* (*data_getter)(const PageNode*),
                                 bool (T::*pm_getter)() const,
                                 void (*setter)(content::WebContents*, bool),
                                 bool default_state) {
  // By default all properties are set to the default value.
  TestPageNodeProperty(contents, data_getter, pm_getter, default_state);

  // Pretend that the property changed and make sure that the PageNode data gets
  // updated.
  (*setter)(contents, !default_state);
  TestPageNodeProperty(contents, data_getter, pm_getter, !default_state);

  // Switch back to the default state.
  (*setter)(contents, default_state);
  TestPageNodeProperty(contents, data_getter, pm_getter, default_state);
}

}  // namespace testing

}  // namespace performance_manager

#endif  // COMPONENTS_PERFORMANCE_MANAGER_TEST_SUPPORT_DECORATORS_UTILS_H_

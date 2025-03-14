// Copyright 2011 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_TEST_BASE_MENU_MODEL_TEST_H_
#define CHROME_TEST_BASE_MENU_MODEL_TEST_H_

#include "ui/base/accelerators/accelerator.h"
#include "ui/menus/simple_menu_model.h"

// A mix-in class to be used in addition to something that derrives from
// testing::Test to provide some extra functionality for testing menu models.
class MenuModelTest {
 public:
  MenuModelTest() = default;
  virtual ~MenuModelTest() = default;

 protected:
  // A menu delegate that counts the number of times certain things are called
  // to make sure things are hooked up properly.
  class Delegate : public ui::SimpleMenuModel::Delegate {
   public:
    Delegate() = default;

    bool IsCommandIdChecked(int command_id) const override;
    bool IsCommandIdEnabled(int command_id) const override;
    void ExecuteCommand(int command_id, int event_flags) override;

    int execute_count_ = 0;
    mutable int enable_count_ = 0;
  };

  // Recursively checks the enabled state and executes a command on every item
  // that's not a separator or a submenu parent item. The returned count should
  // match the number of times the delegate is called to ensure every item
  // works.
  void CountEnabledExecutable(ui::MenuModel* model, int* count);

  Delegate delegate_;
};

#endif  // CHROME_TEST_BASE_MENU_MODEL_TEST_H_

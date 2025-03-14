// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_RENDERER_CONTEXT_MENU_RENDER_VIEW_CONTEXT_MENU_BROWSERTEST_UTIL_H_
#define CHROME_BROWSER_RENDERER_CONTEXT_MENU_RENDER_VIEW_CONTEXT_MENU_BROWSERTEST_UTIL_H_

#include <memory>
#include <optional>
#include <vector>

#include "base/memory/raw_ptr.h"
#include "base/run_loop.h"
#include "chrome/browser/renderer_context_menu/render_view_context_menu.h"
#include "components/renderer_context_menu/render_view_context_menu_observer.h"
#include "content/public/browser/context_menu_params.h"

class RenderViewContextMenu;

class ContextMenuNotificationObserver {
 public:
  using MenuShownCallback = base::OnceCallback<void(RenderViewContextMenu*)>;

  // Wait for a context menu to be shown, and then execute |command_to_execute|.
  explicit ContextMenuNotificationObserver(int command_to_execute);

  // Wait for a context menu to be shown, and then execute |command_to_execute|
  // with specified |event_flags|. Also executes |callback| after executing the
  // command if provided.
  ContextMenuNotificationObserver(int command_to_execute,
                                  int event_flags,
                                  MenuShownCallback callback);

  ContextMenuNotificationObserver(const ContextMenuNotificationObserver&) =
      delete;
  ContextMenuNotificationObserver& operator=(
      const ContextMenuNotificationObserver&) = delete;

  ~ContextMenuNotificationObserver();

 private:
  void MenuShown(RenderViewContextMenu* context_menu);

  void ExecuteCommand(RenderViewContextMenu* context_menu);

  int command_to_execute_;
  int event_flags_;
  MenuShownCallback callback_;
};

// Observe whether a command is executed or not.
class CommandExecutionObserver : public RenderViewContextMenuObserver {
 public:
  explicit CommandExecutionObserver(RenderViewContextMenu* context_menu,
                                    int command_id);
  ~CommandExecutionObserver() override;

  void CommandWillBeExecuted(int command_id) override;
  void CommandBlocked(int command_id) override;

  std::optional<bool> IsCommandExecuted() const;

 private:
  raw_ptr<RenderViewContextMenu> context_menu_;
  int command_id_;
  std::optional<bool> executed_;
};

class ContextMenuWaiter {
 public:
  ContextMenuWaiter();
  explicit ContextMenuWaiter(int command_to_execute);
  // `before_execute` will be run after the context menu is opened and before
  // executing `command_to_execute`.
  ContextMenuWaiter(int command_to_execute, base::OnceClosure before_execute);

  ContextMenuWaiter(const ContextMenuWaiter&) = delete;
  ContextMenuWaiter& operator=(const ContextMenuWaiter&) = delete;

  ~ContextMenuWaiter();

  content::ContextMenuParams& params();
  const std::vector<int>& GetCapturedCommandIds() const;
  const std::vector<int>& GetCapturedEnabledCommandIds() const;

  // A `nullopt` return value indicates the command has not yet been checked
  // whether it is allowed to execute. Or there is no command to execute by
  // `ContextMenuWaiter`.
  std::optional<bool> IsCommandExecuted() const;

  // Wait until the context menu is opened and closed.
  void WaitForMenuOpenAndClose();

 private:
  void MenuShown(RenderViewContextMenu* context_menu);

  void Cancel(RenderViewContextMenu* context_menu);

  content::ContextMenuParams params_;
  std::vector<int> captured_command_ids_;
  std::vector<int> captured_enabled_command_ids_;

  base::RunLoop run_loop_;
  const std::optional<int> maybe_command_to_execute_;
  base::OnceClosure before_execute_;
  std::unique_ptr<CommandExecutionObserver> execution_observer_;
};

#endif  // CHROME_BROWSER_RENDERER_CONTEXT_MENU_RENDER_VIEW_CONTEXT_MENU_BROWSERTEST_UTIL_H_

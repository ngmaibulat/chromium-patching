// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_WEBUI_COMMERCE_SHOPPING_UI_HANDLER_DELEGATE_H_
#define CHROME_BROWSER_UI_WEBUI_COMMERCE_SHOPPING_UI_HANDLER_DELEGATE_H_

#include "base/memory/raw_ptr.h"
#include "components/commerce/core/webui/shopping_service_handler.h"
#include "services/metrics/public/cpp/ukm_source_id.h"

class Browser;

namespace bookmarks {
class BookmarkModel;
class BookmarkNode;
}  // namespace bookmarks

class Profile;

namespace commerce {

class ShoppingUiHandlerDelegate : public ShoppingServiceHandler::Delegate {
 public:
  explicit ShoppingUiHandlerDelegate(Profile* profile);
  ShoppingUiHandlerDelegate(const ShoppingUiHandlerDelegate&) = delete;
  ShoppingUiHandlerDelegate& operator=(const ShoppingUiHandlerDelegate&) =
      delete;
  ~ShoppingUiHandlerDelegate() override;

  std::optional<GURL> GetCurrentTabUrl() override;

  const bookmarks::BookmarkNode* GetOrAddBookmarkForCurrentUrl() override;

  void SwitchToOrOpenTab(const GURL& url) override;

  void OpenUrlInNewTab(const GURL& url) override;

  void ShowFeedbackForProductSpecifications(const std::string& log_id) override;

  // Get the main frame source id from the page load.
  ukm::SourceId GetCurrentTabUkmSourceId() override;

 private:
  void NavigateToUrl(Browser* browser, const GURL& url);

  raw_ptr<Profile> profile_;
  raw_ptr<bookmarks::BookmarkModel> bookmark_model_;
};

}  // namespace commerce

#endif  // CHROME_BROWSER_UI_WEBUI_COMMERCE_SHOPPING_UI_HANDLER_DELEGATE_H_

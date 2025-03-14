// Copyright 2013 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>

#include "base/command_line.h"
#include "build/build_config.h"
#include "chrome/browser/extensions/test_extension_service.h"
#include "chrome/browser/extensions/test_extension_system.h"
#include "chrome/browser/signin/account_consistency_mode_manager.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/browser/ui/browser_finder.h"
#include "chrome/browser/ui/browser_list.h"
#include "chrome/browser/ui/signin/promos/bubble_signin_promo_delegate.h"
#include "chrome/browser/ui/singleton_tabs.h"
#include "chrome/browser/ui/tabs/tab_strip_model.h"
#include "chrome/common/chrome_switches.h"
#include "chrome/test/base/in_process_browser_test.h"
#include "chrome/test/base/testing_profile.h"
#include "components/signin/public/base/account_consistency_method.h"
#include "components/signin/public/base/signin_metrics.h"
#include "components/signin/public/identity_manager/account_info.h"
#include "components/sync/service/local_data_description.h"
#include "content/public/test/browser_test.h"
#include "content/public/test/test_utils.h"
#include "ui/events/event_constants.h"
#include "ui/gfx/range/range.h"

class BubbleSignInPromoDelegateTest : public InProcessBrowserTest {
 public:
  BubbleSignInPromoDelegateTest() = default;

  BubbleSignInPromoDelegateTest(const BubbleSignInPromoDelegateTest&) = delete;
  BubbleSignInPromoDelegateTest& operator=(
      const BubbleSignInPromoDelegateTest&) = delete;

  Profile* profile() { return browser()->profile(); }

  void ReplaceBlank(Browser* browser);

  void SignInBrowser(Browser* browser);
};

// The default browser created for tests start with one tab open on
// about:blank.  The sign-in page is a singleton that will
// replace this tab.  This function replaces about:blank with another URL
// so that the sign in page goes into a new tab.
void BubbleSignInPromoDelegateTest::ReplaceBlank(Browser* browser) {
  ShowSingletonTabOverwritingNTP(browser, GURL("chrome:version"),
                                 NavigateParams::IGNORE_AND_NAVIGATE);
}

void BubbleSignInPromoDelegateTest::SignInBrowser(Browser* browser) {
  auto delegate = std::make_unique<BubbleSignInPromoDelegate>(
      *browser->tab_strip_model()->GetActiveWebContents(),
      signin_metrics::AccessPoint::kBookmarkBubble,
      syncer::LocalDataItemModel::DataId());
  delegate->OnSignIn(AccountInfo());
}

IN_PROC_BROWSER_TEST_F(BubbleSignInPromoDelegateTest, OnSignInLinkClicked) {
  ReplaceBlank(browser());
  int starting_tab_count = browser()->tab_strip_model()->count();
  SignInBrowser(browser());
  EXPECT_EQ(starting_tab_count + 1, browser()->tab_strip_model()->count());
}

IN_PROC_BROWSER_TEST_F(BubbleSignInPromoDelegateTest,
                       OnSignInLinkClickedReusesBlank) {
  int starting_tab_count = browser()->tab_strip_model()->count();
  SignInBrowser(browser());
  EXPECT_EQ(starting_tab_count, browser()->tab_strip_model()->count());
}

IN_PROC_BROWSER_TEST_F(BubbleSignInPromoDelegateTest,
                       OnSignInLinkClickedIncognito_RegularBrowserWithTabs) {
  ReplaceBlank(browser());
  int starting_tab_count = browser()->tab_strip_model()->count();
  EXPECT_GT(starting_tab_count, 0);
  Browser* incognito_browser = CreateIncognitoBrowser();
  int starting_tab_count_incognito =
      incognito_browser->tab_strip_model()->count();

  SignInBrowser(incognito_browser);

  int tab_count = browser()->tab_strip_model()->count();
  // A full-tab signin page is used.
  EXPECT_EQ(starting_tab_count + 1, tab_count);

  // No effect is expected on the incognito browser.
  int tab_count_incognito = incognito_browser->tab_strip_model()->count();
  EXPECT_EQ(starting_tab_count_incognito, tab_count_incognito);
}

IN_PROC_BROWSER_TEST_F(BubbleSignInPromoDelegateTest,
                       OnSignInLinkClickedIncognito_RegularBrowserClosed) {
  Browser* incognito_browser = CreateIncognitoBrowser();
  int starting_tab_count_incognito =
      incognito_browser->tab_strip_model()->count();
  // Close the main browser.
  CloseBrowserSynchronously(browser());

  SignInBrowser(incognito_browser);

  // Signing in fom incognito should create a new non-incognito browser.
  Browser* new_regular_browser = chrome::FindTabbedBrowser(
      incognito_browser->profile()->GetOriginalProfile(), false);

  // The full-tab sign-in page should be shown in the newly created browser.
  EXPECT_EQ(1, new_regular_browser->tab_strip_model()->count());

  // No effect is expected on the incognito browser.
  int tab_count_incognito = incognito_browser->tab_strip_model()->count();
  EXPECT_EQ(starting_tab_count_incognito, tab_count_incognito);
}

// Verifies that the sign in page can be loaded in a different browser
// if the provided browser is invalidated.
IN_PROC_BROWSER_TEST_F(BubbleSignInPromoDelegateTest, BrowserRemoved) {
  // Create an extra browser.
  Browser* extra_browser = CreateBrowser(profile());
  ReplaceBlank(extra_browser);

  int starting_tab_count = extra_browser->tab_strip_model()->count();

  std::unique_ptr<BubbleSignInPromoDelegate> delegate =
      std::make_unique<BubbleSignInPromoDelegate>(
          *extra_browser->tab_strip_model()->GetActiveWebContents(),
          signin_metrics::AccessPoint::kBookmarkBubble,
          syncer::LocalDataItemModel::DataId());

  BrowserList::SetLastActive(extra_browser);

  // Close all tabs in the original browser.  Run all pending messages
  // to make sure the browser window closes before continuing.
  browser()->tab_strip_model()->CloseAllTabs();
  content::RunAllPendingInMessageLoop();

  delegate->OnSignIn(AccountInfo());

  int tab_count = extra_browser->tab_strip_model()->count();
  // A new tab should have been opened in the extra browser, which should be
  // visible.
  EXPECT_EQ(starting_tab_count + 1, tab_count);
}

// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ash/wallpaper/wallpaper_utils/wallpaper_ephemeral_user.h"

#include "ash/test/ash_test_base.h"
#include "components/account_id/account_id.h"
#include "components/user_manager/user_names.h"
#include "components/user_manager/user_type.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace ash {
namespace {

using WallpaperEphemeralUserTest = AshTestBase;

TEST_F(WallpaperEphemeralUserTest, GuestUser) {
  EXPECT_TRUE(IsEphemeralUser(user_manager::GuestAccountId()));
}

TEST_F(WallpaperEphemeralUserTest, RegularUser) {
  EXPECT_FALSE(IsEphemeralUser(user_manager::StubAccountId()));
}

TEST_F(WallpaperEphemeralUserTest, RegularUserLoggedIn) {
  const AccountId& account_id = user_manager::StubAccountId();
  SimulateUserLogin({.user_type = user_manager::UserType::kRegular},
                    account_id);
  EXPECT_FALSE(IsEphemeralUser(account_id));
}

TEST_F(WallpaperEphemeralUserTest, PublicAccount) {
  auto account_id = SimulateUserLogin(
      {"managed_guest@test.com", user_manager::UserType::kPublicAccount});
  EXPECT_TRUE(IsEphemeralUser(account_id));
}

}  // namespace

}  // namespace ash

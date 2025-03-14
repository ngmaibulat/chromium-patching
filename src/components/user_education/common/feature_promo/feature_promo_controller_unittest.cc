// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/user_education/common/feature_promo/feature_promo_controller.h"

#include "base/feature_list.h"
#include "components/user_education/test/mock_feature_promo_controller.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace user_education {

namespace {
BASE_FEATURE(kTestIPHFeature,
             "TestIPHFeature",
             base::FEATURE_ENABLED_BY_DEFAULT);
}  // namespace

using testing::Ref;
using testing::Return;
using testing::StrictMock;

TEST(FeaturePromoControllerTest, IsPromoShowing_NotShowing) {
  StrictMock<test::MockFeaturePromoController> controller;
  EXPECT_CALL(controller, GetPromoStatus(Ref(kTestIPHFeature)))
      .WillOnce(Return(FeaturePromoStatus::kNotRunning));

  EXPECT_FALSE(controller.IsPromoActive(kTestIPHFeature));
}

TEST(FeaturePromoControllerTest,
     IsPromoShowing_NotShowingCalledWithDifferentVariations) {
  StrictMock<test::MockFeaturePromoController> controller;
  EXPECT_CALL(controller, GetPromoStatus(Ref(kTestIPHFeature)))
      .WillRepeatedly(Return(FeaturePromoStatus::kNotRunning));

  EXPECT_FALSE(controller.IsPromoActive(kTestIPHFeature,
                                        FeaturePromoStatus::kContinued));
  EXPECT_FALSE(
      controller.IsPromoActive(kTestIPHFeature, FeaturePromoStatus::kQueued));
  EXPECT_FALSE(controller.IsPromoActive(kTestIPHFeature,
                                        FeaturePromoStatus::kContinued,
                                        FeaturePromoStatus::kQueued));
}

TEST(FeaturePromoControllerTest, IsPromoShowing_IsShowing) {
  StrictMock<test::MockFeaturePromoController> controller;
  EXPECT_CALL(controller, GetPromoStatus(Ref(kTestIPHFeature)))
      .WillOnce(Return(FeaturePromoStatus::kBubbleShowing));

  EXPECT_TRUE(controller.IsPromoActive(kTestIPHFeature));
}

TEST(FeaturePromoControllerTest,
     IsPromoShowing_IsShowingCalledWithDifferentVariations) {
  StrictMock<test::MockFeaturePromoController> controller;
  EXPECT_CALL(controller, GetPromoStatus(Ref(kTestIPHFeature)))
      .WillRepeatedly(Return(FeaturePromoStatus::kBubbleShowing));

  EXPECT_TRUE(controller.IsPromoActive(kTestIPHFeature,
                                       FeaturePromoStatus::kContinued));
  EXPECT_TRUE(
      controller.IsPromoActive(kTestIPHFeature, FeaturePromoStatus::kQueued));
  EXPECT_TRUE(controller.IsPromoActive(kTestIPHFeature,
                                       FeaturePromoStatus::kContinued,
                                       FeaturePromoStatus::kQueued));
}

TEST(FeaturePromoControllerTest, IsPromoShowing_Continued) {
  StrictMock<test::MockFeaturePromoController> controller;
  EXPECT_CALL(controller, GetPromoStatus(Ref(kTestIPHFeature)))
      .WillOnce(Return(FeaturePromoStatus::kContinued));

  EXPECT_TRUE(controller.IsPromoActive(kTestIPHFeature,
                                       FeaturePromoStatus::kContinued));
}

TEST(FeaturePromoControllerTest,
     IsPromoShowing_ContinuedWithDifferentVariations) {
  StrictMock<test::MockFeaturePromoController> controller;
  EXPECT_CALL(controller, GetPromoStatus(Ref(kTestIPHFeature)))
      .WillRepeatedly(Return(FeaturePromoStatus::kContinued));

  EXPECT_FALSE(controller.IsPromoActive(kTestIPHFeature));
  EXPECT_TRUE(controller.IsPromoActive(kTestIPHFeature,
                                       FeaturePromoStatus::kContinued));
  EXPECT_FALSE(
      controller.IsPromoActive(kTestIPHFeature, FeaturePromoStatus::kQueued));
  EXPECT_TRUE(controller.IsPromoActive(kTestIPHFeature,
                                       FeaturePromoStatus::kContinued,
                                       FeaturePromoStatus::kQueued));
}

TEST(FeaturePromoControllerTest, IsPromoShowing_Queued) {
  StrictMock<test::MockFeaturePromoController> controller;
  EXPECT_CALL(controller, GetPromoStatus(Ref(kTestIPHFeature)))
      .WillOnce(Return(FeaturePromoStatus::kQueued));

  EXPECT_TRUE(
      controller.IsPromoActive(kTestIPHFeature, FeaturePromoStatus::kQueued));
}

TEST(FeaturePromoControllerTest, IsPromoShowing_QueuedWithDifferentVariations) {
  StrictMock<test::MockFeaturePromoController> controller;
  EXPECT_CALL(controller, GetPromoStatus(Ref(kTestIPHFeature)))
      .WillRepeatedly(Return(FeaturePromoStatus::kQueued));

  EXPECT_FALSE(controller.IsPromoActive(kTestIPHFeature));
  EXPECT_FALSE(controller.IsPromoActive(kTestIPHFeature,
                                        FeaturePromoStatus::kContinued));
  EXPECT_TRUE(
      controller.IsPromoActive(kTestIPHFeature, FeaturePromoStatus::kQueued));
  EXPECT_TRUE(controller.IsPromoActive(kTestIPHFeature,
                                       FeaturePromoStatus::kContinued,
                                       FeaturePromoStatus::kQueued));
}

}  // namespace user_education

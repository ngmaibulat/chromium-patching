// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/privacy_sandbox/privacy_sandbox_survey_service.h"

#include "base/test/metrics/histogram_tester.h"
#include "base/test/scoped_feature_list.h"
#include "base/test/task_environment.h"
#include "base/version_info/channel.h"
#include "components/prefs/testing_pref_service.h"
#include "components/privacy_sandbox/privacy_sandbox_features.h"
#include "components/signin/public/identity_manager/identity_test_environment.h"
#include "components/signin/public/identity_manager/identity_test_utils.h"
#include "privacy_sandbox_prefs.h"
#include "privacy_sandbox_survey_service.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

using testing::ContainerEq;

namespace privacy_sandbox {
namespace {

class PrivacySandboxSurveyServiceTest : public testing::Test {
 public:
  PrivacySandboxSurveyServiceTest()
      : task_env_(base::test::TaskEnvironment::TimeSource::MOCK_TIME) {
    privacy_sandbox::RegisterProfilePrefs(prefs()->registry());
    identity_test_env_ = std::make_unique<signin::IdentityTestEnvironment>();
  }

  void SetUp() override {
    feature_list_.InitWithFeaturesAndParameters(GetEnabledFeatures(),
                                                GetDisabledFeatures());
    survey_service_ = std::make_unique<PrivacySandboxSurveyService>(
        prefs(), identity_test_env_->identity_manager());
  }
  void TearDown() override { survey_service_ = nullptr; }

 protected:
  signin::IdentityTestEnvironment* identity_test_env() {
    return identity_test_env_.get();
  }
  virtual std::vector<base::test::FeatureRefAndParams> GetEnabledFeatures() {
    return {{kPrivacySandboxSentimentSurvey, {}}};
  }

  virtual std::vector<base::test::FeatureRef> GetDisabledFeatures() {
    return {};
  }

  PrivacySandboxSurveyService* survey_service() {
    return survey_service_.get();
  }

  TestingPrefServiceSimple* prefs() { return &prefs_; }

  base::HistogramTester histogram_tester_;
  std::unique_ptr<signin::IdentityTestEnvironment> identity_test_env_;
  TestingPrefServiceSimple prefs_;
  std::unique_ptr<PrivacySandboxSurveyService> survey_service_;
  base::test::ScopedFeatureList feature_list_;
  base::test::TaskEnvironment task_env_;
};

class PrivacySandboxSurveyServiceFeatureDisabledTest
    : public PrivacySandboxSurveyServiceTest {
  std::vector<base::test::FeatureRefAndParams> GetEnabledFeatures() override {
    return {};
  }
  std::vector<base::test::FeatureRef> GetDisabledFeatures() override {
    return {kPrivacySandboxSentimentSurvey};
  }
};

TEST_F(PrivacySandboxSurveyServiceFeatureDisabledTest, SurveyDoesNotShow) {
  EXPECT_FALSE(survey_service()->ShouldShowSentimentSurvey());
}

class PrivacySandboxSurveyServiceSentimentSurveyPsbTest
    : public PrivacySandboxSurveyServiceTest,
      public testing::WithParamInterface<
          testing::tuple<bool, bool, bool, bool>> {};

TEST_P(PrivacySandboxSurveyServiceSentimentSurveyPsbTest, FetchesValues) {
  prefs()->SetBoolean(prefs::kPrivacySandboxM1TopicsEnabled,
                      testing::get<0>(GetParam()));
  prefs()->SetBoolean(prefs::kPrivacySandboxM1FledgeEnabled,
                      testing::get<1>(GetParam()));
  prefs()->SetBoolean(prefs::kPrivacySandboxM1AdMeasurementEnabled,
                      testing::get<2>(GetParam()));
  if (testing::get<3>(GetParam())) {
    signin::MakePrimaryAccountAvailable(identity_test_env()->identity_manager(),
                                        "test@gmail.com",
                                        signin::ConsentLevel::kSignin);
  }

  std::map<std::string, bool> expected_map = {
      {"Topics enabled", testing::get<0>(GetParam())},
      {"Protected audience enabled", testing::get<1>(GetParam())},
      {"Measurement enabled", testing::get<2>(GetParam())},
      {"Signed in", testing::get<3>(GetParam())},
  };

  EXPECT_THAT(survey_service()->GetSentimentSurveyPsb(),
              ContainerEq(expected_map));
}

INSTANTIATE_TEST_SUITE_P(PrivacySandboxSurveyServiceSentimentSurveyPsbTest,
                         PrivacySandboxSurveyServiceSentimentSurveyPsbTest,
                         testing::Combine(testing::Bool(),
                                          testing::Bool(),
                                          testing::Bool(),
                                          testing::Bool()));

class PrivacySandboxSurveyServiceSentimentSurveyPsdTest
    : public PrivacySandboxSurveyServiceTest,
      public testing::WithParamInterface<
          testing::tuple<version_info::Channel, std::string>> {};

TEST_P(PrivacySandboxSurveyServiceSentimentSurveyPsdTest, SetsPsd) {
  std::map<std::string, std::string> expected_map = {
      {"Channel", testing::get<1>(GetParam())},
  };

  EXPECT_THAT(
      survey_service()->GetSentimentSurveyPsd(testing::get<0>(GetParam())),
      ContainerEq(expected_map));
}

INSTANTIATE_TEST_SUITE_P(
    PrivacySandboxSurveyServiceSentimentSurveyPsdTest,
    PrivacySandboxSurveyServiceSentimentSurveyPsdTest,
    testing::Values(
        testing::make_tuple(version_info::Channel::UNKNOWN, "unknown"),
        testing::make_tuple(version_info::Channel::STABLE, "stable"),
        testing::make_tuple(version_info::Channel::BETA, "beta"),
        testing::make_tuple(version_info::Channel::DEV, "dev"),
        testing::make_tuple(version_info::Channel::CANARY, "canary")));

class PrivacySandboxSurveyServiceSentimentSurveyStatusHistogramTest
    : public PrivacySandboxSurveyServiceTest,
      public testing::WithParamInterface<
          PrivacySandboxSurveyService::PrivacySandboxSentimentSurveyStatus> {};

TEST_P(PrivacySandboxSurveyServiceSentimentSurveyStatusHistogramTest,
       EmitsHistogram) {
  survey_service()->RecordSentimentSurveyStatus(GetParam());
  histogram_tester_.ExpectBucketCount("PrivacySandbox.SentimentSurvey.Status",
                                      GetParam(), 1);
}

INSTANTIATE_TEST_SUITE_P(
    PrivacySandboxSurveyServiceSentimentSurveyStatusHistogramTest,
    PrivacySandboxSurveyServiceSentimentSurveyStatusHistogramTest,
    testing::Values(
        PrivacySandboxSurveyService::PrivacySandboxSentimentSurveyStatus::
            kSurveyShown,
        PrivacySandboxSurveyService::PrivacySandboxSentimentSurveyStatus::
            kFeatureDisabled,
        PrivacySandboxSurveyService::PrivacySandboxSentimentSurveyStatus::
            kHatsServiceFailed,
        PrivacySandboxSurveyService::PrivacySandboxSentimentSurveyStatus::
            kSurveyLaunchFailed,
        PrivacySandboxSurveyService::PrivacySandboxSentimentSurveyStatus::
            kInvalidSurveyConfig));

}  // namespace
}  // namespace privacy_sandbox

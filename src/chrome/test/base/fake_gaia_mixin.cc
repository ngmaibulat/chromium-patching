// Copyright 2018 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/test/base/fake_gaia_mixin.h"

#include "base/command_line.h"
#include "build/build_config.h"
#include "chrome/test/supervised_user/child_account_test_utils.h"
#include "google_apis/gaia/gaia_constants.h"
#include "google_apis/gaia/gaia_id.h"
#include "google_apis/gaia/gaia_switches.h"
#include "google_apis/gaia/gaia_urls.h"
#include "net/test/embedded_test_server/http_response.h"

namespace {

constexpr char kGAIAHost[] = "accounts.google.com";

}  // namespace

// static
const char FakeGaiaMixin::kFakeUserEmail[] = "fake-email@gmail.com";
const char FakeGaiaMixin::kFakeUserPassword[] = "fake-password";
const GaiaId::Literal FakeGaiaMixin::kFakeUserGaiaId("fake-gaia-id");
const char FakeGaiaMixin::kFakeAuthCode[] = "fake-auth-code";
const char FakeGaiaMixin::kFakeRefreshToken[] = "fake-refresh-token";
const char FakeGaiaMixin::kEmptyUserServices[] = "[]";
const char FakeGaiaMixin::kFakeAllScopeAccessToken[] = "fake-all-scope-token";
const int FakeGaiaMixin::kFakeAccessTokenExpiration = 3600;

const char FakeGaiaMixin::kFakeSIDCookie[] = "fake-SID-cookie";
const char FakeGaiaMixin::kFakeLSIDCookie[] = "fake-LSID-cookie";

// LINT.IfChange
const char FakeGaiaMixin::kEnterpriseUser1[] = "username@example.com";
const GaiaId::Literal FakeGaiaMixin::kEnterpriseUser1GaiaId("0000111111");
// LINT.ThenChange(/components/policy/core/common/cloud/test/policy_builder.cc)
const char FakeGaiaMixin::kEnterpriseUser2[] = "user-2@example.com";
const GaiaId::Literal FakeGaiaMixin::kEnterpriseUser2GaiaId("0000222222");

const char FakeGaiaMixin::kTestUserinfoToken1[] = "fake-userinfo-token-1";
const char FakeGaiaMixin::kTestRefreshToken1[] = "fake-refresh-token-1";
const char FakeGaiaMixin::kTestUserinfoToken2[] = "fake-userinfo-token-2";
const char FakeGaiaMixin::kTestRefreshToken2[] = "fake-refresh-token-2";

const FakeGaiaMixin::UiPath FakeGaiaMixin::kEmailPath = {"identifier"};
const FakeGaiaMixin::UiPath FakeGaiaMixin::kPasswordPath = {"password"};

FakeGaiaMixin::FakeGaiaMixin(InProcessBrowserTestMixinHost* host)
    : InProcessBrowserTestMixin(host),
      fake_gaia_(std::make_unique<FakeGaia>()) {}

FakeGaiaMixin::~FakeGaiaMixin() = default;

void FakeGaiaMixin::SetupFakeGaiaForLogin(const std::string& user_email,
                                          const GaiaId& gaia_id,
                                          const std::string& refresh_token) {
  if (!gaia_id.empty()) {
    fake_gaia_->MapEmailToGaiaId(user_email, gaia_id);
  }

  FakeGaia::AccessTokenInfo token_info;
  token_info.token = kFakeAllScopeAccessToken;
  token_info.audience = GaiaUrls::GetInstance()->oauth2_chrome_client_id();
  token_info.email = user_email;
  token_info.any_scope = true;
  token_info.expires_in = kFakeAccessTokenExpiration;
  fake_gaia_->IssueOAuthToken(refresh_token, token_info);
}

void FakeGaiaMixin::SetupFakeGaiaForLoginWithDefaults() {
  SetupFakeGaiaForLogin(FakeGaiaMixin::kFakeUserEmail,
                        FakeGaiaMixin::kFakeUserGaiaId,
                        FakeGaiaMixin::kFakeRefreshToken);
}

void FakeGaiaMixin::SetupFakeGaiaForChildUser(const std::string& user_email,
                                              const GaiaId& gaia_id,
                                              const std::string& refresh_token,
                                              bool issue_any_scope_token) {
  if (!gaia_id.empty()) {
    fake_gaia_->MapEmailToGaiaId(user_email, gaia_id);
  }

  FakeGaia::AccessTokenInfo user_info_token;
  user_info_token.scopes.insert(GaiaConstants::kDeviceManagementServiceOAuth);
  user_info_token.scopes.insert(GaiaConstants::kGoogleUserInfoEmail);
  user_info_token.audience = GaiaUrls::GetInstance()->oauth2_chrome_client_id();

  user_info_token.token = "fake-userinfo-token";
  user_info_token.expires_in = kFakeAccessTokenExpiration;
  user_info_token.email = user_email;
  if (initialize_child_id_token()) {
    user_info_token.id_token = supervised_user::GetChildAccountOAuthIdToken();
  }
  fake_gaia_->IssueOAuthToken(refresh_token, user_info_token);

  if (issue_any_scope_token) {
    FakeGaia::AccessTokenInfo all_scopes_token;
    all_scopes_token.token = kFakeAllScopeAccessToken;
    all_scopes_token.audience =
        GaiaUrls::GetInstance()->oauth2_chrome_client_id();
    all_scopes_token.expires_in = kFakeAccessTokenExpiration;
    all_scopes_token.email = user_email;
    all_scopes_token.any_scope = true;
    fake_gaia_->IssueOAuthToken(refresh_token, all_scopes_token);
  }

  if (initialize_configuration()) {
    fake_gaia_->SetConfigurationHelper(user_email, kFakeSIDCookie,
                                       kFakeLSIDCookie);

    FakeGaia::Configuration configuration_update;
    configuration_update.id_token =
        supervised_user::GetChildAccountOAuthIdToken();
    fake_gaia_->UpdateConfiguration(configuration_update);
  }
}

#if BUILDFLAG(IS_CHROMEOS)
void FakeGaiaMixin::SetupFakeGaiaForLoginManager() {
  FakeGaia::AccessTokenInfo token_info;
  token_info.scopes.insert(GaiaConstants::kDeviceManagementServiceOAuth);
  token_info.scopes.insert(GaiaConstants::kGoogleUserInfoEmail);
  token_info.audience = GaiaUrls::GetInstance()->oauth2_chrome_client_id();

  token_info.token = kTestUserinfoToken1;
  token_info.expires_in = kFakeAccessTokenExpiration;
  token_info.email = kEnterpriseUser1;
  fake_gaia_->IssueOAuthToken(kTestRefreshToken1, token_info);

  token_info.token = kTestUserinfoToken2;
  token_info.email = kEnterpriseUser2;
  fake_gaia_->IssueOAuthToken(kTestRefreshToken2, token_info);
}
#endif  // BUILDFLAG(IS_CHROMEOS)

GURL FakeGaiaMixin::GetFakeGaiaURL(const std::string& relative_url) {
  return gaia_server_.GetURL(kGAIAHost, relative_url);
}

void FakeGaiaMixin::SetUp() {
  net::EmbeddedTestServer::ServerCertificateConfig cert_config;
  cert_config.dns_names = {kGAIAHost};
  gaia_server_.SetSSLConfig(cert_config);
  gaia_server_.RegisterDefaultHandler(base::BindRepeating(
      &FakeGaia::HandleRequest, base::Unretained(fake_gaia_.get())));
  // Initialize the server so `SetUpCommandLine()` can query the URL, but don't
  // start the IO thread until `SetUpOnMainThread()`, after the sandbox is
  // initialized.
  ASSERT_TRUE(gaia_server_.InitializeAndListen());
}

void FakeGaiaMixin::SetUpCommandLine(base::CommandLine* command_line) {
  GURL gaia_url = GetFakeGaiaURL("/");
  command_line->AppendSwitchASCII(::switches::kGaiaUrl, gaia_url.spec());
  command_line->AppendSwitchASCII(::switches::kLsoUrl, gaia_url.spec());
  command_line->AppendSwitchASCII(::switches::kGoogleApisUrl, gaia_url.spec());
  command_line->AppendSwitchASCII(::switches::kOAuthAccountManagerUrl,
                                  gaia_url.spec());
}

void FakeGaiaMixin::SetUpOnMainThread() {
  fake_gaia_->Initialize();
  fake_gaia_->set_issue_oauth_code_cookie(true);

  gaia_server_.StartAcceptingConnections();

  if (initialize_configuration()) {
    fake_gaia_->SetConfigurationHelper(kFakeUserEmail, kFakeSIDCookie,
                                       kFakeLSIDCookie);
  }
}

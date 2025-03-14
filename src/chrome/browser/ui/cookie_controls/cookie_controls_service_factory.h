// Copyright 2020 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_COOKIE_CONTROLS_COOKIE_CONTROLS_SERVICE_FACTORY_H_
#define CHROME_BROWSER_UI_COOKIE_CONTROLS_COOKIE_CONTROLS_SERVICE_FACTORY_H_

#include "base/no_destructor.h"
#include "chrome/browser/profiles/profile_keyed_service_factory.h"

class Profile;
class CookieControlsService;

// Factory to get or create an instance of CookieControlsService from
// a Profile.
class CookieControlsServiceFactory : public ProfileKeyedServiceFactory {
 public:
  static CookieControlsService* GetForProfile(Profile* profile);

  static CookieControlsServiceFactory* GetInstance();

  // Used to create instances for testing.
  static std::unique_ptr<KeyedService> BuildInstanceFor(Profile* profile);

 private:
  friend base::NoDestructor<CookieControlsServiceFactory>;

  CookieControlsServiceFactory();
  ~CookieControlsServiceFactory() override;

  // BrowserContextKeyedServiceFactory:
  std::unique_ptr<KeyedService> BuildServiceInstanceForBrowserContext(
      content::BrowserContext* profile) const override;
};

#endif  // CHROME_BROWSER_UI_COOKIE_CONTROLS_COOKIE_CONTROLS_SERVICE_FACTORY_H_

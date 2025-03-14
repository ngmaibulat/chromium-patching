// Copyright 2020 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'chrome://chrome-signin/inline_login_app.js';

import type {InlineLoginAppElement} from 'chrome://chrome-signin/inline_login_app.js';
import {View} from 'chrome://chrome-signin/inline_login_app.js';
import {InlineLoginBrowserProxyImpl} from 'chrome://chrome-signin/inline_login_browser_proxy.js';
import {webUIListenerCallback} from 'chrome://resources/js/cr.js';
import {loadTimeData} from 'chrome://resources/js/load_time_data.js';
import {flush} from 'chrome://resources/polymer/v3_0/polymer/polymer_bundled.min.js';
import {assertEquals, assertFalse, assertTrue} from 'chrome://webui-test/chai_assert.js';

import {fakeAuthenticationData, fakeAuthenticationDataWithEmail, TestAuthenticator, TestInlineLoginBrowserProxy} from './inline_login_test_util.js';

suite('InlineLoginWelcomePageTest', () => {
  let inlineLoginComponent: InlineLoginAppElement;
  let testBrowserProxy: TestInlineLoginBrowserProxy;
  let testAuthenticator: TestAuthenticator;

  /** @return The id of the active view. */
  function getActiveViewId(): string {
    return inlineLoginComponent.shadowRoot!
        .querySelector('div.active[slot="view"]')!.id;
  }

  function testSetup() {
    document.body.innerHTML = window.trustedTypes!.emptyHTML;
    testBrowserProxy = new TestInlineLoginBrowserProxy();
    InlineLoginBrowserProxyImpl.setInstance(testBrowserProxy);
    document.body.innerHTML = window.trustedTypes!.emptyHTML;
    inlineLoginComponent = document.createElement('inline-login-app');
    document.body.appendChild(inlineLoginComponent);
    testAuthenticator = new TestAuthenticator();
    inlineLoginComponent.setAuthenticatorForTest(testAuthenticator);
    flush();
  }

  suiteSetup(function() {
    loadTimeData.overrideValues({shouldSkipWelcomePage: false});
  });

  test('Reauthentication', () => {
    testSetup();
    webUIListenerCallback(
        'load-authenticator', fakeAuthenticationDataWithEmail);
    // Welcome screen should be skipped for reauth.
    assertEquals(
        View.ADD_ACCOUNT, getActiveViewId(), 'Welcome screen should be active');
  });

  test('OkButton', () => {
    testSetup();
    webUIListenerCallback('load-authenticator', fakeAuthenticationData);
    const okButton =
        inlineLoginComponent.shadowRoot!.querySelector<HTMLElement>(
            '.next-button');
    assertTrue(!!okButton);
    // OK button and welcome screen should be visible.
    assertFalse(okButton.hidden, 'OK button should be visible');
    assertEquals(
        View.WELCOME, getActiveViewId(), 'Welcome screen should be active');

    okButton.click();
    assertEquals(
        View.ADD_ACCOUNT, getActiveViewId(),
        'Add account screen should be active');

    return testBrowserProxy.whenCalled('skipWelcomePage').then(skip => {
      assertEquals(
          false, skip, 'skipWelcomePage should be called with "false"');
    });
  });

  test('Checkbox', () => {
    testSetup();

    webUIListenerCallback('load-authenticator', fakeAuthenticationData);
    const welcomePageApp =
        inlineLoginComponent.shadowRoot!.querySelector('welcome-page-app');
    assertTrue(!!welcomePageApp);
    const checkbox = welcomePageApp.shadowRoot!.querySelector('cr-checkbox');
    assertTrue(!!checkbox);
    assertFalse(checkbox.checked, 'Checkbox should be unchecked by default');
    checkbox.click();
    assertTrue(checkbox.checked, 'Checkbox should be checked');

    const okButton =
        inlineLoginComponent.shadowRoot!.querySelector<HTMLElement>(
            '.next-button');
    assertTrue(!!okButton);
    okButton.click();
    return testBrowserProxy.whenCalled('skipWelcomePage').then(skip => {
      assertEquals(true, skip, 'skipWelcomePage should be called with "true"');
    });
  });

  test('GoBack', () => {
    testSetup();
    webUIListenerCallback('load-authenticator', fakeAuthenticationData);
    const backButton =
        inlineLoginComponent.shadowRoot!.querySelector<HTMLElement>(
            '.back-button');
    assertTrue(!!backButton);
    const okButton =
        inlineLoginComponent.shadowRoot!.querySelector<HTMLElement>(
            '.next-button');
    assertTrue(!!okButton);

    assertTrue(backButton.hidden, 'Back button should be hidden');
    assertFalse(okButton.hidden, 'OK button should be visible');
    assertEquals(
        View.WELCOME, getActiveViewId(), 'Welcome screen should be active');

    okButton.click();
    assertTrue(okButton.hidden, 'OK button should be hidden');
    assertFalse(backButton.hidden, 'Back button should be visible');
    assertEquals(
        View.ADD_ACCOUNT, getActiveViewId(),
        'Add account screen should be active');

    backButton.click();
    assertTrue(backButton.hidden, 'Back button should be hidden');
    assertFalse(okButton.hidden, 'OK button should be visible');
    assertEquals(
        View.WELCOME, getActiveViewId(), 'Welcome screen should be active');
  });

  test('LinkClick', async () => {
    testSetup();

    const welcomePageApp =
        inlineLoginComponent.shadowRoot!.querySelector('welcome-page-app');
    assertTrue(!!welcomePageApp);

    welcomePageApp.shadowRoot!.querySelector<HTMLElement>(
                                  '#osSettingsLink')!.click();
    await testBrowserProxy.whenCalled('dialogClose');

    welcomePageApp.shadowRoot!.querySelector<HTMLElement>(
                                  '#appsSettingsLink')!.click();
    await testBrowserProxy.whenCalled('dialogClose');

    welcomePageApp.shadowRoot!.querySelector<HTMLElement>(
                                  '#newPersonLink')!.click();
    await testBrowserProxy.whenCalled('dialogClose');

    welcomePageApp.shadowRoot!.querySelector<HTMLElement>(
                                  '#guestModeLink')!.click();
    return testBrowserProxy.whenCalled('openGuestWindow');
  });
});

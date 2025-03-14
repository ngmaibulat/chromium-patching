// Copyright 2018 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "ios/chrome/browser/autofill/ui_bundled/manual_fill/form_observer_helper.h"

#import "components/autofill/core/common/form_data.h"
#import "components/autofill/ios/form_util/form_activity_observer_bridge.h"
#import "ios/chrome/browser/shared/model/web_state_list/web_state_list.h"
#import "ios/chrome/browser/shared/model/web_state_list/web_state_list_observer_bridge.h"

@interface FormObserverHelper () <FormActivityObserver, WebStateListObserving>
// The WebStateList this instance is observing in order to update the
// active WebState.
@property(nonatomic, assign) WebStateList* webStateList;

// The WebState this instance is observing. Can be nullptr.
@property(nonatomic, assign) web::WebState* webState;

@end

@implementation FormObserverHelper {
  // Bridge to observe the web state list from Objective-C.
  std::unique_ptr<WebStateListObserverBridge> _webStateListObserver;

  // Bridge to observe form activity in `_webState`.
  std::unique_ptr<autofill::FormActivityObserverBridge>
      _formActivityObserverBridge;
}

@synthesize delegate = _delegate;

- (instancetype)initWithWebStateList:(WebStateList*)webStateList {
  DCHECK(webStateList);
  self = [super init];
  if (self) {
    _webStateList = webStateList;
    _webStateListObserver = std::make_unique<WebStateListObserverBridge>(self);
    _webStateList->AddObserver(_webStateListObserver.get());
    web::WebState* webState = webStateList->GetActiveWebState();
    if (webState) {
      _webState = webState;
      _formActivityObserverBridge =
          std::make_unique<autofill::FormActivityObserverBridge>(_webState,
                                                                 self);
    }
  }
  return self;
}

- (void)dealloc {
  [self disconnect];
}

#pragma mark - FormActivityObserver

- (void)webState:(web::WebState*)webState
    didRegisterFormActivity:(const autofill::FormActivityParams&)params
                    inFrame:(web::WebFrame*)frame {
  if ([self.delegate respondsToSelector:@selector
                     (webState:didRegisterFormActivity:inFrame:)]) {
    [self.delegate webState:webState
        didRegisterFormActivity:params
                        inFrame:frame];
  }
}

- (void)webState:(web::WebState*)webState
    didSubmitDocumentWithFormData:(const autofill::FormData&)formData
                   hasUserGesture:(BOOL)hasUserGesture
                          inFrame:(web::WebFrame*)frame {
  if ([self.delegate
          respondsToSelector:@selector
          (webState:didSubmitDocumentWithFormData:hasUserGesture:inFrame:)]) {
    [self.delegate webState:webState
        didSubmitDocumentWithFormData:formData
                       hasUserGesture:hasUserGesture
                              inFrame:frame];
  }
}

#pragma mark - WebStateListObserving

- (void)didChangeWebStateList:(WebStateList*)webStateList
                       change:(const WebStateListChange&)change
                       status:(const WebStateListStatus&)status {
  if (status.active_web_state_change()) {
    self.webState = status.new_active_web_state;
  }
}

- (void)webStateListDestroyed:(WebStateList*)webStateList {
  [self disconnect];
}

#pragma mark - Setters

// Sets the new web state and detaches from the previous web state.
- (void)setWebState:(web::WebState*)webState {
  if (_webState) {
    _formActivityObserverBridge.reset();
  }

  _webState = webState;

  if (_webState) {
    _formActivityObserverBridge =
        std::make_unique<autofill::FormActivityObserverBridge>(_webState, self);
  }
}

#pragma mark - Private

- (void)disconnect {
  if (_webState) {
    _formActivityObserverBridge.reset();
    _webState = nullptr;
  }
  if (_webStateList) {
    _webStateList->RemoveObserver(_webStateListObserver.get());
    _webStateListObserver.reset();
    _webStateList = nullptr;
  }
}

@end

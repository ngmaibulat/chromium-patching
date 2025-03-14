// Copyright 2019 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/input/mock_input_router.h"

#include "base/task/sequenced_task_runner.h"
#include "components/input/input_router_client.h"

namespace input {

void MockInputRouter::SendMouseEvent(
    const MouseEventWithLatencyInfo& mouse_event,
    MouseEventCallback event_result_callback,
    DispatchToRendererCallback& dispatch_callback) {
  std::move(dispatch_callback)
      .Run(mouse_event.event, DispatchToRendererResult::kDispatched);
  sent_mouse_event_ = true;
}
void MockInputRouter::SendWheelEvent(
    const MouseWheelEventWithLatencyInfo& wheel_event,
    DispatchToRendererCallback& dispatch_callback) {
  std::move(dispatch_callback)
      .Run(wheel_event.event, DispatchToRendererResult::kDispatched);
  sent_wheel_event_ = true;
}
void MockInputRouter::SendKeyboardEvent(
    const NativeWebKeyboardEventWithLatencyInfo& key_event,
    KeyboardEventCallback event_result_callback,
    DispatchToRendererCallback& dispatch_callback) {
  std::move(dispatch_callback)
      .Run(key_event.event, DispatchToRendererResult::kDispatched);
  sent_keyboard_event_ = true;
}
void MockInputRouter::SendGestureEvent(
    const GestureEventWithLatencyInfo& gesture_event,
    DispatchToRendererCallback& dispatch_callback) {
  std::move(dispatch_callback)
      .Run(gesture_event.event, DispatchToRendererResult::kDispatched);
  sent_gesture_event_ = true;
}
void MockInputRouter::SendTouchEvent(
    const TouchEventWithLatencyInfo& touch_event,
    DispatchToRendererCallback& dispatch_callback) {
  send_touch_event_not_cancelled_ =
      client_->FilterInputEvent(touch_event.event, touch_event.latency) ==
      blink::mojom::InputEventResultState::kNotConsumed;
  if (send_touch_event_not_cancelled_) {
    std::move(dispatch_callback)
        .Run(touch_event.event, DispatchToRendererResult::kDispatched);
  } else {
    std::move(dispatch_callback)
        .Run(touch_event.event, DispatchToRendererResult::kNotDispatched);
  }
}

bool MockInputRouter::HasPendingEvents() const {
  return false;
}

std::optional<cc::TouchAction> MockInputRouter::AllowedTouchAction() {
  return cc::TouchAction::kAuto;
}

std::optional<cc::TouchAction> MockInputRouter::ActiveTouchAction() {
  return cc::TouchAction::kAuto;
}

mojo::PendingRemote<blink::mojom::WidgetInputHandlerHost>
MockInputRouter::BindNewHost(scoped_refptr<base::SequencedTaskRunner>) {
  return mojo::NullRemote();
}

void MockInputRouter::OnHasTouchEventConsumers(
    blink::mojom::TouchEventConsumersPtr consumers) {
  has_handlers_ = consumers->has_touch_event_handlers;
}

}  // namespace input

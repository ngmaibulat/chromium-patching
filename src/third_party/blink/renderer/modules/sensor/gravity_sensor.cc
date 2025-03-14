// Copyright 2021 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/renderer/modules/sensor/gravity_sensor.h"

#include "services/network/public/mojom/permissions_policy/permissions_policy_feature.mojom-blink.h"

using device::mojom::blink::SensorType;

namespace blink {

// static
GravitySensor* GravitySensor::Create(ExecutionContext* execution_context,
                                     const SpatialSensorOptions* options,
                                     ExceptionState& exception_state) {
  return MakeGarbageCollected<GravitySensor>(execution_context, options,
                                             exception_state);
}

// static
GravitySensor* GravitySensor::Create(ExecutionContext* execution_context,
                                     ExceptionState& exception_state) {
  return Create(execution_context, SpatialSensorOptions::Create(),
                exception_state);
}

GravitySensor::GravitySensor(ExecutionContext* execution_context,
                             const SpatialSensorOptions* options,
                             ExceptionState& exception_state)
    : Accelerometer(
          execution_context,
          options,
          exception_state,
          SensorType::GRAVITY,
          {network::mojom::PermissionsPolicyFeature::kAccelerometer}) {}

void GravitySensor::Trace(Visitor* visitor) const {
  Accelerometer::Trace(visitor);
}

}  // namespace blink

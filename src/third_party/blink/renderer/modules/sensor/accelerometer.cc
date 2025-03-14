// Copyright 2016 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/renderer/modules/sensor/accelerometer.h"

#include "services/network/public/mojom/permissions_policy/permissions_policy_feature.mojom-blink.h"

using device::mojom::blink::SensorType;

namespace blink {

Accelerometer* Accelerometer::Create(ExecutionContext* execution_context,
                                     const SpatialSensorOptions* options,
                                     ExceptionState& exception_state) {
  const Vector<network::mojom::PermissionsPolicyFeature> features(
      {network::mojom::PermissionsPolicyFeature::kAccelerometer});
  return MakeGarbageCollected<Accelerometer>(
      execution_context, options, exception_state, SensorType::ACCELEROMETER,
      features);
}

// static
Accelerometer* Accelerometer::Create(ExecutionContext* execution_context,
                                     ExceptionState& exception_state) {
  return Create(execution_context, SpatialSensorOptions::Create(),
                exception_state);
}

Accelerometer::Accelerometer(
    ExecutionContext* execution_context,
    const SpatialSensorOptions* options,
    ExceptionState& exception_state,
    SensorType sensor_type,
    const Vector<network::mojom::PermissionsPolicyFeature>& features)
    : Sensor(execution_context,
             options,
             exception_state,
             sensor_type,
             features) {}

std::optional<double> Accelerometer::x() const {
  if (hasReading())
    return GetReading().accel.x;
  return std::nullopt;
}

std::optional<double> Accelerometer::y() const {
  if (hasReading())
    return GetReading().accel.y;
  return std::nullopt;
}

std::optional<double> Accelerometer::z() const {
  if (hasReading())
    return GetReading().accel.z;
  return std::nullopt;
}

void Accelerometer::Trace(Visitor* visitor) const {
  Sensor::Trace(visitor);
}

}  // namespace blink

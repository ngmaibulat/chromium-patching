// Copyright 2018 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMEOS_ASH_EXPERIENCES_ARC_TEST_FAKE_TIMER_INSTANCE_H_
#define CHROMEOS_ASH_EXPERIENCES_ARC_TEST_FAKE_TIMER_INSTANCE_H_

#include "chromeos/ash/experiences/arc/mojom/timer.mojom.h"
#include "mojo/public/cpp/bindings/pending_remote.h"
#include "mojo/public/cpp/bindings/remote.h"

namespace arc {

class FakeTimerInstance : public mojom::TimerInstance {
 public:
  FakeTimerInstance();

  FakeTimerInstance(const FakeTimerInstance&) = delete;
  FakeTimerInstance& operator=(const FakeTimerInstance&) = delete;

  ~FakeTimerInstance() override;

  // mojom::TimerInstance overrides:
  void Init(mojo::PendingRemote<mojom::TimerHost> host_remote,
            InitCallback callback) override;

  mojom::TimerHost* GetTimerHost() const;

 private:
  mojo::Remote<mojom::TimerHost> host_remote_;
};

}  // namespace arc

#endif  // CHROMEOS_ASH_EXPERIENCES_ARC_TEST_FAKE_TIMER_INSTANCE_H_

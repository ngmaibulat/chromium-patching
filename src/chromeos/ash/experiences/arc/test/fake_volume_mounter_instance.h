// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMEOS_ASH_EXPERIENCES_ARC_TEST_FAKE_VOLUME_MOUNTER_INSTANCE_H_
#define CHROMEOS_ASH_EXPERIENCES_ARC_TEST_FAKE_VOLUME_MOUNTER_INSTANCE_H_

#include <map>
#include <string>

#include "base/containers/queue.h"
#include "chromeos/ash/experiences/arc/mojom/volume_mounter.mojom.h"
#include "mojo/public/cpp/bindings/pending_remote.h"
#include "mojo/public/cpp/bindings/remote.h"
#include "third_party/cros_system_api/dbus/cros-disks/dbus-constants.h"

namespace arc {

class FakeVolumeMounterInstance : public mojom::VolumeMounterInstance {
 public:
  FakeVolumeMounterInstance();
  FakeVolumeMounterInstance(const FakeVolumeMounterInstance&) = delete;
  FakeVolumeMounterInstance& operator=(const FakeVolumeMounterInstance&) =
      delete;
  ~FakeVolumeMounterInstance() override;

  int num_on_mount_event_called() { return num_on_mount_event_called_; }

  mojom::MountPointInfoPtr GetMountPointInfo(const std::string& mount_path);

  // Runs the oldest callback in `callbacks_` with `success`.
  void RunCallback(bool success);

  // mojom::VolumeMounterInstance overrides:
  void Init(::mojo::PendingRemote<mojom::VolumeMounterHost> host_remote,
            InitCallback callback) override;
  void OnMountEvent(mojom::MountPointInfoPtr mount_point_info) override;
  void PrepareForRemovableMediaUnmount(
      const base::FilePath& mount_path,
      PrepareForRemovableMediaUnmountCallback callback) override;

 private:
  mojo::Remote<mojom::VolumeMounterHost> host_remote_;
  int num_on_mount_event_called_ = 0;
  std::map<std::string, mojom::MountPointInfoPtr> mount_path_to_info_;
  base::queue<PrepareForRemovableMediaUnmountCallback> callbacks_;
};

}  // namespace arc

#endif  // CHROMEOS_ASH_EXPERIENCES_ARC_TEST_FAKE_VOLUME_MOUNTER_INSTANCE_H_

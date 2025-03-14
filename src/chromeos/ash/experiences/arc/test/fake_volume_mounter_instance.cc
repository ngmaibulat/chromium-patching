// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chromeos/ash/experiences/arc/test/fake_volume_mounter_instance.h"

#include <map>
#include <string>

#include "base/check.h"
#include "chromeos/ash/experiences/arc/mojom/volume_mounter.mojom.h"

namespace arc {

FakeVolumeMounterInstance::FakeVolumeMounterInstance() = default;

FakeVolumeMounterInstance::~FakeVolumeMounterInstance() {}

void FakeVolumeMounterInstance::Init(
    ::mojo::PendingRemote<mojom::VolumeMounterHost> host_remote,
    InitCallback callback) {
  host_remote_.reset();
  host_remote_.Bind(std::move(host_remote));
  std::move(callback).Run();
}

void FakeVolumeMounterInstance::OnMountEvent(
    mojom::MountPointInfoPtr mount_point_info) {
  mount_path_to_info_[mount_point_info->mount_path] =
      std::move(mount_point_info);
  num_on_mount_event_called_++;
}

mojom::MountPointInfoPtr FakeVolumeMounterInstance::GetMountPointInfo(
    const std::string& mount_path) {
  auto iter = mount_path_to_info_.find(mount_path);
  if (iter == mount_path_to_info_.end()) {
    return nullptr;
  }
  return iter->second.Clone();
}

void FakeVolumeMounterInstance::PrepareForRemovableMediaUnmount(
    const base::FilePath& mount_path,
    PrepareForRemovableMediaUnmountCallback callback) {
  callbacks_.push(std::move(callback));
}

void FakeVolumeMounterInstance::RunCallback(bool success) {
  CHECK(!callbacks_.empty());
  std::move(callbacks_.front()).Run(success);
  callbacks_.pop();
}

}  // namespace arc

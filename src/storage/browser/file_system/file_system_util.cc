// Copyright 2021 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "storage/browser/file_system/file_system_util.h"

#include "base/feature_list.h"
#include "storage/browser/file_system/file_system_features.h"
#include "storage/common/file_system/file_system_types.h"
#include "third_party/blink/public/mojom/quota/quota_types.mojom.h"

namespace storage {

blink::mojom::StorageType FileSystemTypeToQuotaStorageType(
    FileSystemType type) {
  switch (type) {
    case kFileSystemTypeTemporary:
    case kFileSystemTypePersistent:
    case kFileSystemTypeSyncable:
    case kFileSystemTypeSyncableForInternalSync:
      return blink::mojom::StorageType::kTemporary;
    default:
      return blink::mojom::StorageType::kUnknown;
  }
}

}  // namespace storage

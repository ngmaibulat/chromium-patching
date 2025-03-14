// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifdef UNSAFE_BUFFERS_BUILD
// TODO(crbug.com/390223051): Remove C-library calls to fix the errors.
#pragma allow_unsafe_libc_calls
#endif

#include <string.h>

#include "services/network/public/cpp/hash_value_mojom_traits.h"

namespace mojo {

// static
bool StructTraits<
    network::mojom::SHA256HashValueDataView,
    net::SHA256HashValue>::Read(network::mojom::SHA256HashValueDataView in,
                                net::SHA256HashValue* out) {
  mojo::ArrayDataView<uint8_t> bytes;
  in.GetDataDataView(&bytes);
  if (bytes.size() != 32) {
    // TODO(crbug.com/40061960): Mojo should validate this array size. We can
    // CHECK it instead when it does.
    return false;
  }
  memcpy(out->data, bytes.data(), bytes.size());
  return true;
}

}  // namespace mojo

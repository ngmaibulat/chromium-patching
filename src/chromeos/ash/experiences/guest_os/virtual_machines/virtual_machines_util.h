// Copyright 2018 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMEOS_ASH_EXPERIENCES_GUEST_OS_VIRTUAL_MACHINES_VIRTUAL_MACHINES_UTIL_H_
#define CHROMEOS_ASH_EXPERIENCES_GUEST_OS_VIRTUAL_MACHINES_VIRTUAL_MACHINES_UTIL_H_

namespace virtual_machines {

// Whether running virtual machines on Chrome OS is allowed
// per enterprise policy.
bool AreVirtualMachinesAllowedByPolicy();

}  // namespace virtual_machines

#endif  // CHROMEOS_ASH_EXPERIENCES_GUEST_OS_VIRTUAL_MACHINES_VIRTUAL_MACHINES_UTIL_H_

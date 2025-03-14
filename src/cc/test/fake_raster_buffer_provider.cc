// Copyright 2016 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cc/test/fake_raster_buffer_provider.h"

#include <utility>

#include "cc/resources/resource_pool.h"
#include "gpu/command_buffer/client/client_shared_image.h"

namespace cc {

FakeRasterBufferProviderImpl::FakeRasterBufferProviderImpl() = default;

FakeRasterBufferProviderImpl::~FakeRasterBufferProviderImpl() = default;

std::unique_ptr<RasterBuffer>
FakeRasterBufferProviderImpl::AcquireBufferForRaster(
    const ResourcePool::InUsePoolResource& resource,
    uint64_t resource_content_id,
    uint64_t previous_content_id,
    bool depends_on_at_raster_decodes,
    bool depends_on_hardware_accelerated_jpeg_candidates,
    bool depends_on_hardware_accelerated_webp_candidates) {
  auto backing = std::make_unique<ResourcePool::Backing>(
      resource.size(), GetFormat(), resource.color_space());
  backing->set_shared_image(gpu::ClientSharedImage::CreateForTesting());
  resource.set_backing(std::move(backing));
  return nullptr;
}

void FakeRasterBufferProviderImpl::Flush() {}

viz::SharedImageFormat FakeRasterBufferProviderImpl::GetFormat() const {
  return is_software_ ? viz::SinglePlaneFormat::kBGRA_8888
                      : viz::SinglePlaneFormat::kRGBA_8888;
}

bool FakeRasterBufferProviderImpl::IsResourcePremultiplied() const {
  return true;
}

bool FakeRasterBufferProviderImpl::CanPartialRasterIntoProvidedResource()
    const {
  return true;
}

bool FakeRasterBufferProviderImpl::IsResourceReadyToDraw(
    const ResourcePool::InUsePoolResource& resource) {
  return true;
}

uint64_t FakeRasterBufferProviderImpl::SetReadyToDrawCallback(
    const std::vector<const ResourcePool::InUsePoolResource*>& resources,
    base::OnceClosure callback,
    uint64_t pending_callback_id) {
  return 0;
}

void FakeRasterBufferProviderImpl::Shutdown() {}

}  // namespace cc

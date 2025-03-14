// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/renderer/platform/graphics/canvas_resource_host.h"

#include <memory>

#include "components/viz/common/resources/release_callback.h"
#include "components/viz/common/resources/transferable_resource.h"
#include "components/viz/test/test_context_provider.h"
#include "components/viz/test/test_gles2_interface.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/blink/renderer/platform/graphics/test/fake_canvas_resource_host.h"
#include "third_party/blink/renderer/platform/graphics/test/gpu_test_utils.h"
#include "third_party/blink/renderer/platform/testing/task_environment.h"
#include "third_party/blink/renderer/platform/testing/testing_platform_support.h"
#include "ui/gfx/geometry/size.h"

namespace blink {
namespace {

using ::viz::TestContextProvider;

class AcceleratedCompositingTestPlatform
    : public blink::TestingPlatformSupport {
 public:
  bool IsGpuCompositingDisabled() const override { return false; }
};

TEST(CanvasResourceHostTest, ReleaseLostTransferableResource) {
  test::TaskEnvironment task_environment;
  ScopedTestingPlatformSupport<AcceleratedCompositingTestPlatform>
      accelerated_compositing_scope;
  scoped_refptr<TestContextProvider> context = TestContextProvider::Create();
  InitializeSharedGpuContextGLES2(context.get());

  auto host = std::make_unique<FakeCanvasResourceHost>(gfx::Size(100, 100));
  host->GetOrCreateCanvasResourceProvider(RasterModeHint::kPreferGPU);
  host->GetOrCreateCcLayerIfNeeded();

  // Prepare a TransferableResource, then report the resource as lost.
  // This test passes by not crashing and not triggering assertions.
  viz::TransferableResource resource;
  viz::ReleaseCallback release_callback;
  EXPECT_TRUE(host->PrepareTransferableResource(&resource, &release_callback));

  bool lost_resource = true;
  std::move(release_callback).Run(gpu::SyncToken(), lost_resource);

  SharedGpuContext::Reset();
}

TEST(CanvasResourceHostTest, ReleaseLostTransferableResourceWithLostContext) {
  test::TaskEnvironment task_environment;
  ScopedTestingPlatformSupport<AcceleratedCompositingTestPlatform>
      accelerated_compositing_scope;
  scoped_refptr<TestContextProvider> context = TestContextProvider::Create();
  InitializeSharedGpuContextGLES2(context.get());

  auto host = std::make_unique<FakeCanvasResourceHost>(gfx::Size(100, 100));
  host->GetOrCreateCanvasResourceProvider(RasterModeHint::kPreferGPU);
  host->GetOrCreateCcLayerIfNeeded();

  viz::TransferableResource resource;
  viz::ReleaseCallback release_callback;

  EXPECT_TRUE(host->PrepareTransferableResource(&resource, &release_callback));

  bool lost_resource = true;
  context->TestContextGL()->set_context_lost(true);
  // Get a new context provider so that the WeakPtr to the old one is null.
  // This is the test to make sure that ReleaseFrameResources() handles
  // null context_provider_wrapper properly.
  SharedGpuContext::ContextProviderWrapper();
  std::move(release_callback).Run(gpu::SyncToken(), lost_resource);

  SharedGpuContext::Reset();
}

TEST(CanvasResourceHostTest, ReleaseResourcesAfterHostDestroyed) {
  test::TaskEnvironment task_environment;
  ScopedTestingPlatformSupport<AcceleratedCompositingTestPlatform>
      accelerated_compositing_scope;
  scoped_refptr<TestContextProvider> context = TestContextProvider::Create();
  InitializeSharedGpuContextGLES2(context.get());

  auto host = std::make_unique<FakeCanvasResourceHost>(gfx::Size(100, 100));
  host->GetOrCreateCanvasResourceProvider(RasterModeHint::kPreferGPU);
  host->GetOrCreateCcLayerIfNeeded();

  viz::TransferableResource resource;
  viz::ReleaseCallback release_callback;

  // Resources aren't released if the host still uses them.
  host->PrepareTransferableResource(&resource, &release_callback);
  EXPECT_EQ(context->TestContextGL()->NumTextures(), 1u);
  std::move(release_callback).Run(gpu::SyncToken(), /*is_lost=*/false);
  EXPECT_EQ(context->TestContextGL()->NumTextures(), 1u);

  // Tearing down the host does not destroy unreleased resources.
  host->PrepareTransferableResource(&resource, &release_callback);
  host.reset();
  EXPECT_EQ(context->TestContextGL()->NumTextures(), 1u);
  std::move(release_callback).Run(gpu::SyncToken(), /*is_lost=*/false);
  EXPECT_EQ(context->TestContextGL()->NumTextures(), 0u);
  SharedGpuContext::Reset();
}

}  // namespace
}  // namespace blink

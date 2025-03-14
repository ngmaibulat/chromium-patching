// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MEDIA_MOJO_CLIENTS_MOJO_CODEC_FACTORY_FUCHSIA_H_
#define MEDIA_MOJO_CLIENTS_MOJO_CODEC_FACTORY_FUCHSIA_H_

#include "base/task/sequenced_task_runner.h"
#include "media/base/overlay_info.h"
#include "media/base/video_decoder.h"
#include "media/mojo/clients/mojo_codec_factory.h"
#include "media/mojo/mojom/fuchsia_media.mojom.h"
#include "media/video/gpu_video_accelerator_factories.h"
#include "mojo/public/cpp/bindings/shared_remote.h"

namespace media {

// MojoCodecFactoryFuchsia gets hardware decoder resources
// via media::mojom::FuchsiaMediaCodecProvider.
//
// Codec-related services on Fuchsia are used directly from the renderer process
// after the browser process provides a connection to the FIDL protocol via
// media::mojom::FuchsiaMediaCodecProvider. This can improve performance by
// avoiding the need to hop through the browser process.
class MojoCodecFactoryFuchsia final : public media::MojoCodecFactory {
 public:
  MojoCodecFactoryFuchsia(
      scoped_refptr<base::SequencedTaskRunner> media_task_runner,
      scoped_refptr<viz::ContextProviderCommandBuffer> context_provider,
      bool video_decode_accelerator_enabled,
      bool video_encode_accelerator_enabled,
      mojo::PendingRemote<media::mojom::VideoEncodeAcceleratorProvider>
          pending_vea_provider_remote,
      mojo::PendingRemote<media::mojom::FuchsiaMediaCodecProvider>
          pending_media_codec_provider_remote);
  ~MojoCodecFactoryFuchsia() override;

  std::unique_ptr<media::VideoDecoder> CreateVideoDecoder(
      media::GpuVideoAcceleratorFactories* gpu_factories,
      media::MediaLog* media_log,
      media::RequestOverlayInfoCB request_overlay_info_cb,
      const gfx::ColorSpace& rendering_color_space) override;

 private:
  void BindOnTaskRunner(
      mojo::PendingRemote<media::mojom::FuchsiaMediaCodecProvider>
          media_codec_provider_remote);
  void OnGetSupportedDecoderConfigs(
      const media::SupportedVideoDecoderConfigs& supported_configs);

  mojo::SharedRemote<media::mojom::FuchsiaMediaCodecProvider>
      media_codec_provider_;
};

}  // namespace media

#endif  // MEDIA_MOJO_CLIENTS_MOJO_CODEC_FACTORY_FUCHSIA_H_

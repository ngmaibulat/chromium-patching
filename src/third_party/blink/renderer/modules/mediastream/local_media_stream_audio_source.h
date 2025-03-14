// Copyright 2016 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef THIRD_PARTY_BLINK_RENDERER_MODULES_MEDIASTREAM_LOCAL_MEDIA_STREAM_AUDIO_SOURCE_H_
#define THIRD_PARTY_BLINK_RENDERER_MODULES_MEDIASTREAM_LOCAL_MEDIA_STREAM_AUDIO_SOURCE_H_

#include <optional>
#include <string>

#include "base/task/single_thread_task_runner.h"
#include "media/base/audio_capturer_source.h"
#include "third_party/blink/renderer/modules/modules_export.h"
#include "third_party/blink/renderer/platform/mediastream/media_stream_audio_source.h"

namespace blink {

class LocalFrame;

// Represents a local source of audio data generated by an AudioInputDevice.
// Uses content::AudioDeviceFactory (indirectly through
// Platform::NewAudioRendererSink) to auto-create the AudioInputDevice,
// using the parameters and session ID found in MediaStreamDevice, just before
// the first track is connected. Audio data is transported directly to the
// tracks (i.e., there is no audio processing).
class MODULES_EXPORT LocalMediaStreamAudioSource final
    : public MediaStreamAudioSource,
      public media::AudioCapturerSource::CaptureCallback {
 public:
  // |consumer_frame| references the RenderFrame that will consume the
  // audio data. Audio parameters and (optionally) a pre-existing audio session
  // ID are read from |device_info|. |requested_buffer_size| is the desired
  // buffer size for the audio hardware, a nullptr means to use the default.
  // |enable_system_echo_cancellation| specifies whether to apply system echo
  // cancellation, and may only be enabled if supported by the device.
  LocalMediaStreamAudioSource(
      LocalFrame* consumer_frame,
      const MediaStreamDevice& device,
      const int* requested_buffer_size,
      bool disable_local_echo,
      bool enable_system_echo_cancellation,
      ConstraintsRepeatingCallback started_callback,
      scoped_refptr<base::SingleThreadTaskRunner> task_runner);

  LocalMediaStreamAudioSource(const LocalMediaStreamAudioSource&) = delete;
  LocalMediaStreamAudioSource& operator=(const LocalMediaStreamAudioSource&) =
      delete;

  ~LocalMediaStreamAudioSource() final;

  // MediaStreamAudioSource implementation.
  void ChangeSourceImpl(const MediaStreamDevice& new_device) final;

  std::optional<AudioProcessingProperties> GetAudioProcessingProperties()
      const final;

 private:
  // MediaStreamAudioSource implementation.
  bool EnsureSourceIsStarted() final;
  void EnsureSourceIsStopped() final;

  // media::AudioCapturerSource::CaptureCallback implementation.
  void OnCaptureStarted() final;
  void Capture(const media::AudioBus* audio_bus,
               base::TimeTicks audio_capture_time,
               const media::AudioGlitchInfo& glitch_info,
               double volume) final;
  void OnCaptureError(media::AudioCapturerSource::ErrorCode code,
                      const std::string& message) final;
  void OnCaptureMuted(bool is_muted) final;

  // The LocalFrame that will consume the audio data. Used when creating
  // AudioInputDevices via the AudioDeviceFactory (indirectly through
  // Platform).
  //
  // TODO(crbug.com/704136): Consider moving LocalMediaStreamAudioSource to
  // Oilpan and use Member<> here.
  WeakPersistent<LocalFrame> consumer_frame_;

  // The device created by the AudioDeviceFactory in EnsureSourceIsStarted().
  scoped_refptr<media::AudioCapturerSource> source_;

  // Callback that's called when the audio source has been initialized.
  ConstraintsRepeatingCallback started_callback_;

  // In debug builds, check that all methods that could cause object graph
  // or data flow changes are being called on the main thread.
  THREAD_CHECKER(thread_checker_);
};

}  // namespace blink

#endif  // THIRD_PARTY_BLINK_RENDERER_MODULES_MEDIASTREAM_LOCAL_MEDIA_STREAM_AUDIO_SOURCE_H_

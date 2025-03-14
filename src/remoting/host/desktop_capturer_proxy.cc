// Copyright 2015 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "remoting/host/desktop_capturer_proxy.h"

#include <cstdint>
#include <memory>
#include <utility>

#include "base/check.h"
#include "base/functional/bind.h"
#include "base/functional/callback.h"
#include "base/location.h"
#include "base/logging.h"
#include "base/memory/ptr_util.h"
#include "base/memory/scoped_refptr.h"
#include "base/memory/weak_ptr.h"
#include "base/notimplemented.h"
#include "base/task/single_thread_task_runner.h"
#include "base/threading/thread_checker.h"
#include "build/build_config.h"
#include "third_party/webrtc/modules/desktop_capture/desktop_capturer.h"
#include "third_party/webrtc/modules/desktop_capture/desktop_frame.h"
#include "third_party/webrtc/modules/desktop_capture/shared_memory.h"

#if BUILDFLAG(IS_CHROMEOS)
#include "remoting/host/chromeos/frame_sink_desktop_capturer.h"
#endif

#if defined(WEBRTC_USE_GIO)
#include "third_party/webrtc/modules/desktop_capture/desktop_capture_metadata.h"
#endif

namespace remoting {

class DesktopCapturerProxy::Core : public webrtc::DesktopCapturer::Callback {
 public:
  explicit Core(base::WeakPtr<DesktopCapturerProxy> proxy);

  Core(const Core&) = delete;
  Core& operator=(const Core&) = delete;

  ~Core() override;

  void set_capturer(std::unique_ptr<webrtc::DesktopCapturer> capturer) {
    DCHECK(!capturer_);
    capturer_ = std::move(capturer);
  }
  void CreateCapturer(
      base::OnceCallback<std::unique_ptr<webrtc::DesktopCapturer>()> creator);

  void Start(scoped_refptr<base::SingleThreadTaskRunner> caller_task_runner);
  void SetSharedMemoryFactory(
      std::unique_ptr<webrtc::SharedMemoryFactory> shared_memory_factory);
  void SelectSource(SourceId id);
  void CaptureFrame();
  void SetMaxFrameRate(std::uint32_t max_frame_rate);
#if defined(WEBRTC_USE_GIO)
  void GetAndSetMetadata();
#endif

 private:
  // webrtc::DesktopCapturer::Callback implementation.
  void OnFrameCaptureStart() override;
  void OnCaptureResult(webrtc::DesktopCapturer::Result result,
                       std::unique_ptr<webrtc::DesktopFrame> frame) override;

  base::WeakPtr<DesktopCapturerProxy> proxy_;
  scoped_refptr<base::SingleThreadTaskRunner> caller_task_runner_;
  std::unique_ptr<webrtc::DesktopCapturer> capturer_;

  THREAD_CHECKER(thread_checker_);
};

DesktopCapturerProxy::Core::Core(base::WeakPtr<DesktopCapturerProxy> proxy)
    : proxy_(proxy) {
  DETACH_FROM_THREAD(thread_checker_);
}

DesktopCapturerProxy::Core::~Core() {
  DCHECK_CALLED_ON_VALID_THREAD(thread_checker_);
}

void DesktopCapturerProxy::Core::CreateCapturer(
    base::OnceCallback<std::unique_ptr<webrtc::DesktopCapturer>()> creator) {
  DCHECK_CALLED_ON_VALID_THREAD(thread_checker_);
  DCHECK(!capturer_);

  capturer_ = std::move(creator).Run();

  if (!capturer_) {
    LOG(ERROR) << "Failed to initialize screen capturer.";
  }
}

void DesktopCapturerProxy::Core::Start(
    scoped_refptr<base::SingleThreadTaskRunner> caller_task_runner) {
  DCHECK_CALLED_ON_VALID_THREAD(thread_checker_);
  DCHECK(!caller_task_runner_);

  caller_task_runner_ = caller_task_runner;
  if (capturer_) {
    capturer_->Start(this);
  }
}

void DesktopCapturerProxy::Core::SetSharedMemoryFactory(
    std::unique_ptr<webrtc::SharedMemoryFactory> shared_memory_factory) {
  DCHECK_CALLED_ON_VALID_THREAD(thread_checker_);

  if (capturer_) {
    capturer_->SetSharedMemoryFactory(std::move(shared_memory_factory));
  }
}

void DesktopCapturerProxy::Core::SelectSource(SourceId id) {
  DCHECK_CALLED_ON_VALID_THREAD(thread_checker_);

  if (capturer_) {
    capturer_->SelectSource(id);
  }
}

void DesktopCapturerProxy::Core::CaptureFrame() {
  DCHECK_CALLED_ON_VALID_THREAD(thread_checker_);

  if (capturer_) {
    capturer_->CaptureFrame();
  } else {
    OnCaptureResult(webrtc::DesktopCapturer::Result::ERROR_PERMANENT, nullptr);
  }
}

void DesktopCapturerProxy::Core::SetMaxFrameRate(std::uint32_t max_frame_rate) {
  DCHECK_CALLED_ON_VALID_THREAD(thread_checker_);

  if (capturer_) {
    capturer_->SetMaxFrameRate(max_frame_rate);
  }
}

#if defined(WEBRTC_USE_GIO)
void DesktopCapturerProxy::Core::GetAndSetMetadata() {
  DCHECK_CALLED_ON_VALID_THREAD(thread_checker_);

  if (capturer_) {
    webrtc::DesktopCaptureMetadata metadata = capturer_->GetMetadata();
    caller_task_runner_->PostTask(
        FROM_HERE, base::BindOnce(&DesktopCapturerProxy::OnMetadata, proxy_,
                                  std::move(metadata)));
  }
}
#endif

void DesktopCapturerProxy::Core::OnFrameCaptureStart() {
  caller_task_runner_->PostTask(
      FROM_HERE,
      base::BindOnce(&DesktopCapturerProxy::OnFrameCaptureStarting, proxy_));
}

void DesktopCapturerProxy::Core::OnCaptureResult(
    webrtc::DesktopCapturer::Result result,
    std::unique_ptr<webrtc::DesktopFrame> frame) {
  caller_task_runner_->PostTask(
      FROM_HERE, base::BindOnce(&DesktopCapturerProxy::OnFrameCaptured, proxy_,
                                result, std::move(frame)));
}

DesktopCapturerProxy::DesktopCapturerProxy(
    scoped_refptr<base::SingleThreadTaskRunner> capture_task_runner)
    : capture_task_runner_(capture_task_runner) {
  DETACH_FROM_THREAD(thread_checker_);
  core_ = std::make_unique<Core>(weak_factory_.GetWeakPtr());
}

DesktopCapturerProxy::~DesktopCapturerProxy() {
  capture_task_runner_->DeleteSoon(FROM_HERE, core_.release());
}

void DesktopCapturerProxy::CreateCapturer(
    base::OnceCallback<std::unique_ptr<webrtc::DesktopCapturer>()> creator) {
  // CreateCapturer() must be called before Start().
  DCHECK(!callback_);

  capture_task_runner_->PostTask(
      FROM_HERE,
      base::BindOnce(&Core::CreateCapturer, base::Unretained(core_.get()),
                     std::move(creator)));
}

void DesktopCapturerProxy::set_capturer(
    std::unique_ptr<webrtc::DesktopCapturer> capturer) {
  // set_capturer() must be called before Start().
  DCHECK(!callback_);

  core_->set_capturer(std::move(capturer));
}

void DesktopCapturerProxy::Start(Callback* callback) {
  DCHECK_CALLED_ON_VALID_THREAD(thread_checker_);

  callback_ = callback;

  capture_task_runner_->PostTask(
      FROM_HERE,
      base::BindOnce(&Core::Start, base::Unretained(core_.get()),
                     base::SingleThreadTaskRunner::GetCurrentDefault()));
}

void DesktopCapturerProxy::SetSharedMemoryFactory(
    std::unique_ptr<webrtc::SharedMemoryFactory> shared_memory_factory) {
  DCHECK_CALLED_ON_VALID_THREAD(thread_checker_);

  capture_task_runner_->PostTask(
      FROM_HERE,
      base::BindOnce(&Core::SetSharedMemoryFactory,
                     base::Unretained(core_.get()),
                     base::WrapUnique(shared_memory_factory.release())));
}

void DesktopCapturerProxy::CaptureFrame() {
  DCHECK_CALLED_ON_VALID_THREAD(thread_checker_);
  // Start() must be called before CaptureFrame().
  DCHECK(callback_);

  capture_task_runner_->PostTask(
      FROM_HERE,
      base::BindOnce(&Core::CaptureFrame, base::Unretained(core_.get())));
}

bool DesktopCapturerProxy::GetSourceList(SourceList* sources) {
  NOTIMPLEMENTED();
  return false;
}

bool DesktopCapturerProxy::SelectSource(SourceId id) {
  DCHECK_CALLED_ON_VALID_THREAD(thread_checker_);

  capture_task_runner_->PostTask(
      FROM_HERE,
      base::BindOnce(&Core::SelectSource, base::Unretained(core_.get()), id));
  return false;
}

void DesktopCapturerProxy::OnFrameCaptureStarting() {
  DCHECK_CALLED_ON_VALID_THREAD(thread_checker_);

  callback_->OnFrameCaptureStart();
}

void DesktopCapturerProxy::OnFrameCaptured(
    webrtc::DesktopCapturer::Result result,
    std::unique_ptr<webrtc::DesktopFrame> frame) {
  DCHECK_CALLED_ON_VALID_THREAD(thread_checker_);

  callback_->OnCaptureResult(result, std::move(frame));
}

#if defined(WEBRTC_USE_GIO)
void DesktopCapturerProxy::GetMetadataAsync(
    base::OnceCallback<void(webrtc::DesktopCaptureMetadata)> callback) {
  DCHECK_CALLED_ON_VALID_THREAD(thread_checker_);

  metadata_callback_ = std::move(callback);
  capture_task_runner_->PostTask(
      FROM_HERE,
      base::BindOnce(&Core::GetAndSetMetadata, base::Unretained(core_.get())));
}

void DesktopCapturerProxy::OnMetadata(webrtc::DesktopCaptureMetadata metadata) {
  DCHECK_CALLED_ON_VALID_THREAD(thread_checker_);

  std::move(metadata_callback_).Run(std::move(metadata));
}
#endif

void DesktopCapturerProxy::SetMaxFrameRate(std::uint32_t max_frame_rate) {
  DCHECK_CALLED_ON_VALID_THREAD(thread_checker_);

  capture_task_runner_->PostTask(
      FROM_HERE, base::BindOnce(&Core::SetMaxFrameRate,
                                base::Unretained(core_.get()), max_frame_rate));
}

}  // namespace remoting

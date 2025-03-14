// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/enterprise_companion/enterprise_companion_service.h"

#include <memory>

#include "base/functional/bind.h"
#include "base/functional/callback.h"
#include "base/functional/callback_helpers.h"
#include "base/location.h"
#include "base/logging.h"
#include "base/memory/scoped_refptr.h"
#include "base/memory/weak_ptr.h"
#include "base/sequence_checker.h"
#include "base/task/sequenced_task_runner.h"
#include "chrome/enterprise_companion/dm_client.h"
#include "chrome/enterprise_companion/enterprise_companion_status.h"
#include "chrome/enterprise_companion/event_logger.h"
#include "chrome/enterprise_companion/proto/enterprise_companion_event.pb.h"

namespace policy {
enum class PolicyFetchReason;
}  // namespace policy

namespace enterprise_companion {

class EnterpriseCompanionServiceImpl : public EnterpriseCompanionService {
 public:
  EnterpriseCompanionServiceImpl(
      std::unique_ptr<DMClient> dm_client,
      base::OnceClosure shutdown_callback,
      scoped_refptr<EnterpriseCompanionEventLogger> event_logger)
      : dm_client_(std::move(dm_client)),
        shutdown_callback_(std::move(shutdown_callback)),
        event_logger_(event_logger) {}

  // Overrides for EnterpriseCompanionService.
  void Shutdown(base::OnceClosure callback) override {
    DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
    VLOG(1) << __func__;

    event_logger_->Flush(base::BindOnce(std::move(callback).Then(
        shutdown_callback_ ? std::move(shutdown_callback_)
                           : base::DoNothing())));
  }

  void FetchPolicies(policy::PolicyFetchReason reason,
                     StatusCallback callback) override {
    DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
    VLOG(1) << __func__;
    dm_client_->RegisterPolicyAgent(
        event_logger_,
        base::BindOnce(&EnterpriseCompanionServiceImpl::OnRegistrationCompleted,
                       weak_ptr_factory_.GetWeakPtr(), reason,
                       std::move(callback)));
  }

 private:
  SEQUENCE_CHECKER(sequence_checker_);

  std::unique_ptr<DMClient> dm_client_;
  base::OnceClosure shutdown_callback_;
  scoped_refptr<EnterpriseCompanionEventLogger> event_logger_;

  void OnRegistrationCompleted(
      policy::PolicyFetchReason reason,
      StatusCallback policy_fetch_callback,
      const EnterpriseCompanionStatus& device_registration_status) {
    DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

    if (!device_registration_status.ok()) {
      std::move(policy_fetch_callback).Run(device_registration_status);
    } else {
      dm_client_->FetchPolicies(
          reason, event_logger_,
          std::move(policy_fetch_callback)
              .Then(base::BindOnce(&EnterpriseCompanionEventLogger::Flush,
                                   event_logger_, base::DoNothing())));
    }
  }

  base::WeakPtrFactory<EnterpriseCompanionServiceImpl> weak_ptr_factory_{this};
};

std::unique_ptr<EnterpriseCompanionService> CreateEnterpriseCompanionService(
    std::unique_ptr<DMClient> dm_client,
    scoped_refptr<EnterpriseCompanionEventLogger> logger,
    base::OnceClosure shutdown_callback) {
  return std::make_unique<EnterpriseCompanionServiceImpl>(
      std::move(dm_client), std::move(shutdown_callback), logger);
}

}  // namespace enterprise_companion

// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/autofill_ai/core/browser/autofill_ai_features.h"

#include "base/types/cxx23_to_underlying.h"
#include "build/build_config.h"
#include "components/autofill/core/common/autofill_features.h"
#include "components/autofill/core/common/autofill_prefs.h"
#include "components/optimization_guide/core/feature_registry/feature_registration.h"
#include "components/optimization_guide/core/model_execution/model_execution_prefs.h"
#include "components/prefs/pref_service.h"

namespace autofill_ai {

bool AutofillAiIsPlatformAndEnterprisePolicyEligible(const PrefService* prefs) {
  constexpr bool is_supported_platform = BUILDFLAG(IS_CHROMEOS) ||
                                         BUILDFLAG(IS_LINUX) ||
                                         BUILDFLAG(IS_MAC) || BUILDFLAG(IS_WIN);
  if constexpr (!is_supported_platform) {
    return false;
  }
  constexpr int kAutofillPredictionSettingsDisabled =
      base::to_underlying(optimization_guide::model_execution::prefs::
                              ModelExecutionEnterprisePolicyValue::kDisable);
  static_assert(kAutofillPredictionSettingsDisabled == 2);
  return base::FeatureList::IsEnabled(
             autofill::features::kAutofillAiWithDataSchema) &&
         prefs->GetInteger(
             optimization_guide::prefs::
                 kAutofillPredictionImprovementsEnterprisePolicyAllowed) !=
             kAutofillPredictionSettingsDisabled &&
         // TODO(crbug.com/395050867): Investigate whether checking that
         // autofill is enabled still makes sense.
         prefs->GetBoolean(autofill::prefs::kAutofillProfileEnabled);
}

}  // namespace autofill_ai

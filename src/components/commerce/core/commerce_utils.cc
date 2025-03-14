// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/commerce/core/commerce_utils.h"

#include <string>
#include <vector>

#include "base/check.h"
#include "base/feature_list.h"
#include "base/json/json_writer.h"
#include "base/metrics/field_trial_params.h"
#include "base/strings/escape.h"
#include "base/time/time.h"
#include "base/uuid.h"
#include "base/values.h"
#include "components/commerce/core/commerce_constants.h"
#include "components/commerce/core/commerce_feature_list.h"
#include "net/base/url_util.h"
#include "url/gurl.h"

namespace commerce {
GURL GetProductSpecsTabUrl(const std::vector<GURL>& urls) {
  auto urls_list = base::Value::List();

  for (auto& url : urls) {
    urls_list.Append(url.spec());
  }

  std::string json;
  if (!base::JSONWriter::Write(urls_list, &json)) {
    return GURL(commerce::kChromeUICompareUrl);
  }

  return net::AppendQueryParameter(GURL(commerce::kChromeUICompareUrl), "urls",
                                   json);
}

GURL GetProductSpecsTabUrlForID(const base::Uuid& uuid) {
  return net::AppendQueryParameter(GURL(commerce::kChromeUICompareUrl), "id",
                                   uuid.AsLowercaseString());
}
}  // namespace commerce

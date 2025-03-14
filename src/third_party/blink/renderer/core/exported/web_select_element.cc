/*
 * Copyright (C) 2010 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "third_party/blink/public/web/web_select_element.h"

#include "third_party/blink/public/platform/web_string.h"
#include "third_party/blink/renderer/core/html/forms/html_option_element.h"
#include "third_party/blink/renderer/core/html/forms/html_select_element.h"
#include "third_party/blink/renderer/core/html_names.h"

namespace blink {

std::vector<WebElement> WebSelectElement::GetListItems() const {
  const HeapVector<Member<HTMLElement>>& source_items =
      ConstUnwrap<HTMLSelectElement>()->GetListItems();
  std::vector<WebElement> items(source_items.size());
  for (wtf_size_t i = 0; i < source_items.size(); ++i)
    items[i] = WebElement(source_items[i].Get());

  return items;
}

WebSelectElement::WebSelectElement(HTMLSelectElement* element)
    : WebFormControlElement(element) {}

DEFINE_WEB_NODE_TYPE_CASTS(WebSelectElement,
                           IsA<HTMLSelectElement>(ConstUnwrap<Node>()))

WebSelectElement& WebSelectElement::operator=(HTMLSelectElement* element) {
  private_ = element;
  return *this;
}

WebSelectElement::operator HTMLSelectElement*() const {
  return blink::To<HTMLSelectElement>(private_.Get());
}

}  // namespace blink

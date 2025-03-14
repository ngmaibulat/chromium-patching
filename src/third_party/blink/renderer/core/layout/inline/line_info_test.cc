// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/renderer/core/layout/inline/line_info.h"

#include "third_party/blink/renderer/core/layout/inline/line_breaker.h"
#include "third_party/blink/renderer/core/testing/core_unit_test_helper.h"

namespace blink {

class LineInfoTest : public RenderingTest {
 protected:
  InlineNode CreateInlineNode(const String& html_content) {
    SetBodyInnerHTML(html_content);

    LayoutBlockFlow* block_flow =
        To<LayoutBlockFlow>(GetLayoutObjectByElementId("container"));
    return InlineNode(block_flow);
  }
};

TEST_F(LineInfoTest, InflowEndOffset) {
  InlineNode node = CreateInlineNode(R"HTML(
      <div id=container>abc<ruby>def<rt>rt</ruby></div>)HTML");
  node.PrepareLayoutIfNeeded();
  ExclusionSpace exclusion_space;
  LeadingFloats leading_floats;
  ConstraintSpace space = ConstraintSpaceForAvailableSize(LayoutUnit::Max());
  LineBreaker line_breaker(node, LineBreakerMode::kContent, space,
                           LineLayoutOpportunity(LayoutUnit::Max()),
                           leading_floats, nullptr, nullptr, &exclusion_space);
  LineInfo line_info;
  line_breaker.NextLine(&line_info);
  EXPECT_EQ(InlineItem::kOpenRubyColumn, line_info.Results()[2].item->Type());
  // InflowEndOffset() should return the end offset of a text in the ruby-base.
  // 7 == "abc" + kOpenRubyColumn + "def"
  EXPECT_EQ(7u, line_info.InflowEndOffset());
}

TEST_F(LineInfoTest, TrailingSpaceWidthInitialEmptyItem) {
  // This test checks that having a text or control item result whose end offset
  // is 0 in the trailing space width computation doesn't run into (D)CHECK
  // failures. 0x0D (carriage return) creates a control item with zero length,
  // and if it's at the start of the IFC, it will have end offset zero.
  // However, to reach it in ComputeTrailingSpaceWidth, it needs to be followed
  // by trailing spaces which hang (rather than collapse), which is why we use
  // `white-space: pre-wrap` and set the line width to zero.
  InlineNode node = CreateInlineNode(R"HTML(
    <div id=container style="white-space: pre-wrap; font-size: 10px">&#x0D; </div>)HTML");
  node.PrepareLayoutIfNeeded();
  ExclusionSpace exclusion_space;
  LeadingFloats leading_floats;
  ConstraintSpace space = ConstraintSpaceForAvailableSize(LayoutUnit());
  LineBreaker line_breaker(node, LineBreakerMode::kContent, space,
                           LineLayoutOpportunity(LayoutUnit()), leading_floats,
                           nullptr, nullptr, &exclusion_space);
  LineInfo line_info;
  line_breaker.NextLine(&line_info);
  // Pass if no DCHECK failure
}

}  // namespace blink

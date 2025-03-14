// Copyright 2011 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/strings/utf_offset_string_conversions.h"

#include <stddef.h>

#include <algorithm>
#include <array>

#include "testing/gtest/include/gtest/gtest.h"

namespace base {

namespace {

static const size_t kNpos = std::u16string::npos;

}  // namespace

TEST(UTFOffsetStringConversionsTest, AdjustOffset) {
  struct UTF8ToUTF16Case {
    const char* utf8;
    size_t input_offset;
    size_t output_offset;
  } utf8_to_utf16_cases[] = {
      {"", 0, 0},
      {"", kNpos, kNpos},
      {"\xe4\xbd\xa0\xe5\xa5\xbd", 1, kNpos},
      {"\xe4\xbd\xa0\xe5\xa5\xbd", 3, 1},
      {"\xed\xb0\x80z", 3, 3},
      {"A\xF0\x90\x8C\x80z", 1, 1},
      {"A\xF0\x90\x8C\x80z", 2, kNpos},
      {"A\xF0\x90\x8C\x80z", 5, 3},
      {"A\xF0\x90\x8C\x80z", 6, 4},
      {"A\xF0\x90\x8C\x80z", kNpos, kNpos},
  };
  for (const auto& i : utf8_to_utf16_cases) {
    const size_t offset = i.input_offset;
    std::vector<size_t> offsets;
    offsets.push_back(offset);
    UTF8ToUTF16AndAdjustOffsets(i.utf8, &offsets);
    EXPECT_EQ(i.output_offset, offsets[0]);
  }

  struct UTF16ToUTF8Case {
    char16_t utf16[10];
    size_t input_offset;
    size_t output_offset;
  };
  auto utf16_to_utf8_cases = std::to_array<UTF16ToUTF8Case>({
      {{}, 0, 0},
      // Converted to 3-byte utf-8 sequences
      {{0x5909, 0x63DB}, 3, kNpos},
      {{0x5909, 0x63DB}, 2, 6},
      {{0x5909, 0x63DB}, 1, 3},
      {{0x5909, 0x63DB}, 0, 0},
      // Converted to 2-byte utf-8 sequences
      {{'A', 0x00bc, 0x00be, 'z'}, 1, 1},
      {{'A', 0x00bc, 0x00be, 'z'}, 2, 3},
      {{'A', 0x00bc, 0x00be, 'z'}, 3, 5},
      {{'A', 0x00bc, 0x00be, 'z'}, 4, 6},
      // Surrogate pair
      {{'A', 0xd800, 0xdf00, 'z'}, 1, 1},
      {{'A', 0xd800, 0xdf00, 'z'}, 2, kNpos},
      {{'A', 0xd800, 0xdf00, 'z'}, 3, 5},
      {{'A', 0xd800, 0xdf00, 'z'}, 4, 6},
  });
  for (size_t i = 0; i < std::size(utf16_to_utf8_cases); ++i) {
    size_t offset = utf16_to_utf8_cases[i].input_offset;
    std::vector<size_t> offsets;
    offsets.push_back(offset);
    UTF16ToUTF8AndAdjustOffsets(utf16_to_utf8_cases[i].utf16, &offsets);
    EXPECT_EQ(utf16_to_utf8_cases[i].output_offset, offsets[0]) << i;
  }
}

TEST(UTFOffsetStringConversionsTest, LimitOffsets) {
  const OffsetAdjuster::Adjustments kNoAdjustments;
  const size_t kLimit = 10;
  const size_t kItems = 20;
  std::vector<size_t> size_ts;
  for (size_t t = 0; t < kItems; ++t) {
    size_ts.push_back(t);
    OffsetAdjuster::AdjustOffset(kNoAdjustments, &size_ts.back(), kLimit);
  }
  size_t unlimited_count = 0;
  for (auto ti : size_ts) {
    if (ti != kNpos) {
      ++unlimited_count;
    }
  }
  EXPECT_EQ(11U, unlimited_count);

  // Reverse the values in the vector and try again.
  size_ts.clear();
  for (size_t t = kItems; t > 0; --t) {
    size_ts.push_back(t - 1);
    OffsetAdjuster::AdjustOffset(kNoAdjustments, &size_ts.back(), kLimit);
  }
  unlimited_count = 0;
  for (auto ti : size_ts) {
    if (ti != kNpos) {
      ++unlimited_count;
    }
  }
  EXPECT_EQ(11U, unlimited_count);
}

TEST(UTFOffsetStringConversionsTest, AdjustOffsets) {
  // Imagine we have strings as shown in the following cases where the
  // X's represent encoded characters.
  // 1: abcXXXdef ==> abcXdef
  {
    std::vector<size_t> offsets;
    for (size_t t = 0; t <= 9; ++t) {
      offsets.push_back(t);
    }
    OffsetAdjuster::Adjustments adjustments;
    adjustments.emplace_back(3, 3, 1);
    OffsetAdjuster::AdjustOffsets(adjustments, &offsets);
    auto expected_1 =
        std::to_array<size_t>({0, 1, 2, 3, kNpos, kNpos, 4, 5, 6, 7});
    EXPECT_EQ(offsets.size(), std::size(expected_1));
    for (size_t i = 0; i < std::size(expected_1); ++i) {
      EXPECT_EQ(expected_1[i], offsets[i]);
    }
  }

  // 2: XXXaXXXXbcXXXXXXXdefXXX ==> XaXXbcXXXXdefX
  {
    std::vector<size_t> offsets;
    for (size_t t = 0; t <= 23; ++t) {
      offsets.push_back(t);
    }
    OffsetAdjuster::Adjustments adjustments;
    adjustments.emplace_back(0, 3, 1);
    adjustments.emplace_back(4, 4, 2);
    adjustments.emplace_back(10, 7, 4);
    adjustments.emplace_back(20, 3, 1);
    OffsetAdjuster::AdjustOffsets(adjustments, &offsets);
    auto expected_2 = std::to_array<size_t>({
        0,     kNpos, kNpos, 1,     2,     kNpos, kNpos, kNpos,
        4,     5,     6,     kNpos, kNpos, kNpos, kNpos, kNpos,
        kNpos, 10,    11,    12,    13,    kNpos, kNpos, 14,
    });
    EXPECT_EQ(offsets.size(), std::size(expected_2));
    for (size_t i = 0; i < std::size(expected_2); ++i) {
      EXPECT_EQ(expected_2[i], offsets[i]);
    }
  }

  // 3: XXXaXXXXbcdXXXeXX ==> aXXXXbcdXXXe
  {
    std::vector<size_t> offsets;
    for (size_t t = 0; t <= 17; ++t) {
      offsets.push_back(t);
    }
    OffsetAdjuster::Adjustments adjustments;
    adjustments.emplace_back(0, 3, 0);
    adjustments.emplace_back(4, 4, 4);
    adjustments.emplace_back(11, 3, 3);
    adjustments.emplace_back(15, 2, 0);
    OffsetAdjuster::AdjustOffsets(adjustments, &offsets);
    auto expected_3 = std::to_array<size_t>({
        0,
        kNpos,
        kNpos,
        0,
        1,
        kNpos,
        kNpos,
        kNpos,
        5,
        6,
        7,
        8,
        kNpos,
        kNpos,
        11,
        12,
        kNpos,
        12,
    });
    EXPECT_EQ(offsets.size(), std::size(expected_3));
    for (size_t i = 0; i < std::size(expected_3); ++i) {
      EXPECT_EQ(expected_3[i], offsets[i]);
    }
  }
}

TEST(UTFOffsetStringConversionsTest, UnadjustOffsets) {
  // Imagine we have strings as shown in the following cases where the
  // X's represent encoded characters.
  // 1: abcXXXdef ==> abcXdef
  {
    std::vector<size_t> offsets;
    for (size_t t = 0; t <= 7; ++t) {
      offsets.push_back(t);
    }
    OffsetAdjuster::Adjustments adjustments;
    adjustments.emplace_back(3, 3, 1);
    OffsetAdjuster::UnadjustOffsets(adjustments, &offsets);
    auto expected_1 = std::to_array<size_t>({0, 1, 2, 3, 6, 7, 8, 9});
    EXPECT_EQ(offsets.size(), std::size(expected_1));
    for (size_t i = 0; i < std::size(expected_1); ++i) {
      EXPECT_EQ(expected_1[i], offsets[i]);
    }
  }

  // 2: XXXaXXXXbcXXXXXXXdefXXX ==> XaXXbcXXXXdefX
  {
    std::vector<size_t> offsets;
    for (size_t t = 0; t <= 14; ++t) {
      offsets.push_back(t);
    }
    OffsetAdjuster::Adjustments adjustments;
    adjustments.emplace_back(0, 3, 1);
    adjustments.emplace_back(4, 4, 2);
    adjustments.emplace_back(10, 7, 4);
    adjustments.emplace_back(20, 3, 1);
    OffsetAdjuster::UnadjustOffsets(adjustments, &offsets);
    auto expected_2 = std::to_array<size_t>({
        0,
        3,
        4,
        kNpos,
        8,
        9,
        10,
        kNpos,
        kNpos,
        kNpos,
        17,
        18,
        19,
        20,
        23,
    });
    EXPECT_EQ(offsets.size(), std::size(expected_2));
    for (size_t i = 0; i < std::size(expected_2); ++i) {
      EXPECT_EQ(expected_2[i], offsets[i]);
    }
  }

  // 3: XXXaXXXXbcdXXXeXX ==> aXXXXbcdXXXe
  {
    std::vector<size_t> offsets;
    for (size_t t = 0; t <= 12; ++t) {
      offsets.push_back(t);
    }
    OffsetAdjuster::Adjustments adjustments;
    adjustments.emplace_back(0, 3, 0);
    adjustments.emplace_back(4, 4, 4);
    adjustments.emplace_back(11, 3, 3);
    adjustments.emplace_back(15, 2, 0);
    OffsetAdjuster::UnadjustOffsets(adjustments, &offsets);
    auto expected_3 = std::to_array<size_t>({
        0,  // this could just as easily be 3
        4, kNpos, kNpos, kNpos, 8, 9, 10, 11, kNpos, kNpos, 14,
        15,  // this could just as easily be 17
    });
    EXPECT_EQ(offsets.size(), std::size(expected_3));
    for (size_t i = 0; i < std::size(expected_3); ++i) {
      EXPECT_EQ(expected_3[i], offsets[i]);
    }
  }
}

// MergeSequentialAdjustments is used by net/base/escape.{h,cc} and
// net/base/net_util.{h,cc}.  The two tests EscapeTest.AdjustOffset and
// NetUtilTest.FormatUrlWithOffsets test its behavior extensively.  This
// is simply a short, additional test.
TEST(UTFOffsetStringConversionsTest, MergeSequentialAdjustments) {
  // Pretend the input string is "abcdefghijklmnopqrstuvwxyz".

  // Set up |first_adjustments| to
  // - remove the leading "a"
  // - combine the "bc" into one character (call it ".")
  // - remove the "f"
  // - remove the "tuv"
  // The resulting string should be ".deghijklmnopqrswxyz".
  OffsetAdjuster::Adjustments first_adjustments;
  first_adjustments.emplace_back(0, 1, 0);
  first_adjustments.emplace_back(1, 2, 1);
  first_adjustments.emplace_back(5, 1, 0);
  first_adjustments.emplace_back(19, 3, 0);

  // Set up |adjustments_on_adjusted_string| to
  // - combine the "." character that replaced "bc" with "d" into one character
  //   (call it "?")
  // - remove the "egh"
  // - expand the "i" into two characters (call them "12")
  // - combine the "jkl" into one character (call it "@")
  // - expand the "z" into two characters (call it "34")
  // The resulting string should be "?12@mnopqrswxy34".
  OffsetAdjuster::Adjustments adjustments_on_adjusted_string;
  adjustments_on_adjusted_string.emplace_back(0, 2, 1);
  adjustments_on_adjusted_string.emplace_back(2, 3, 0);
  adjustments_on_adjusted_string.emplace_back(5, 1, 2);
  adjustments_on_adjusted_string.emplace_back(6, 3, 1);
  adjustments_on_adjusted_string.emplace_back(19, 1, 2);

  // Now merge the adjustments and check the results.
  OffsetAdjuster::MergeSequentialAdjustments(first_adjustments,
                                             &adjustments_on_adjusted_string);
  // The merged adjustments should look like
  // - combine abcd into "?"
  //   - note: it's also reasonable for the Merge function to instead produce
  //     two adjustments instead of this, one to remove a and another to
  //     combine bcd into "?".  This test verifies the current behavior.
  // - remove efgh
  // - expand i into "12"
  // - combine jkl into "@"
  // - remove tuv
  // - expand z into "34"
  ASSERT_EQ(6u, adjustments_on_adjusted_string.size());
  EXPECT_EQ(0u, adjustments_on_adjusted_string[0].original_offset);
  EXPECT_EQ(4u, adjustments_on_adjusted_string[0].original_length);
  EXPECT_EQ(1u, adjustments_on_adjusted_string[0].output_length);
  EXPECT_EQ(4u, adjustments_on_adjusted_string[1].original_offset);
  EXPECT_EQ(4u, adjustments_on_adjusted_string[1].original_length);
  EXPECT_EQ(0u, adjustments_on_adjusted_string[1].output_length);
  EXPECT_EQ(8u, adjustments_on_adjusted_string[2].original_offset);
  EXPECT_EQ(1u, adjustments_on_adjusted_string[2].original_length);
  EXPECT_EQ(2u, adjustments_on_adjusted_string[2].output_length);
  EXPECT_EQ(9u, adjustments_on_adjusted_string[3].original_offset);
  EXPECT_EQ(3u, adjustments_on_adjusted_string[3].original_length);
  EXPECT_EQ(1u, adjustments_on_adjusted_string[3].output_length);
  EXPECT_EQ(19u, adjustments_on_adjusted_string[4].original_offset);
  EXPECT_EQ(3u, adjustments_on_adjusted_string[4].original_length);
  EXPECT_EQ(0u, adjustments_on_adjusted_string[4].output_length);
  EXPECT_EQ(25u, adjustments_on_adjusted_string[5].original_offset);
  EXPECT_EQ(1u, adjustments_on_adjusted_string[5].original_length);
  EXPECT_EQ(2u, adjustments_on_adjusted_string[5].output_length);
}

}  // namespace base

// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifdef UNSAFE_BUFFERS_BUILD
// TODO(crbug.com/40284755): Remove this and spanify to fix the errors.
#pragma allow_unsafe_buffers
#endif

// All data that is passed through a WebSocket with type "Text" needs to be
// validated as UTF8. Since this is done on the IO thread, it needs to be
// reasonably fast.

// We are only interested in the performance on valid UTF8. Invalid UTF8 will
// result in a connection failure, so is unlikely to become a source of
// performance issues.

#include "base/i18n/streaming_utf8_validator.h"

#include <stddef.h>

#include <string>
#include <string_view>

#include "base/functional/bind.h"
#include "base/functional/callback.h"
#include "base/strings/cstring_view.h"
#include "base/strings/string_util.h"
#include "base/strings/stringprintf.h"
#include "base/test/perf_time_logger.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace base {
namespace {

// We want to test ranges of valid UTF-8 sequences. These ranges are inclusive.
// They are intended to be large enough that the validator needs to do
// meaningful work while being in some sense "realistic" (eg. control characters
// are not included).
const char kOneByteSeqRangeStart[] = " ";  // U+0020
const char kOneByteSeqRangeEnd[] = "~";    // U+007E

const char kTwoByteSeqRangeStart[] = "\xc2\xa0";  // U+00A0 non-breaking space
const char kTwoByteSeqRangeEnd[] = "\xc9\x8f";    // U+024F small y with stroke

const char kThreeByteSeqRangeStart[] = "\xe3\x81\x82";  // U+3042 Hiragana "a"
const char kThreeByteSeqRangeEnd[] = "\xe9\xbf\x83";    // U+9FC3 "to blink"

const char kFourByteSeqRangeStart[] = "\xf0\xa0\x80\x8b";  // U+2000B
const char kFourByteSeqRangeEnd[] = "\xf0\xaa\x9a\xb2";    // U+2A6B2

// The different lengths of strings to test.
const size_t kTestLengths[] = {1, 32, 256, 32768, 1 << 20};

// Simplest possible byte-at-a-time validator, to provide a baseline
// for comparison. This is only tried on 1-byte UTF-8 sequences, as
// the results will not be meaningful with sequences containing
// top-bit-set bytes.
bool IsString7Bit(const std::string& s) {
  for (auto it : s) {
    if (it & 0x80) {
      return false;
    }
  }
  return true;
}

// Assumes that |previous| is a valid UTF-8 sequence, and attempts to return
// the next one. Is just barely smart enough to iterate through the ranges
// defined about.
std::string NextUtf8Sequence(const std::string& previous) {
  DCHECK(StreamingUtf8Validator::Validate(previous));
  std::string next = previous;
  for (int i = static_cast<int>(previous.length() - 1); i >= 0; --i) {
    // All bytes in a UTF-8 sequence except the first one are
    // constrained to the range 0x80 to 0xbf, inclusive. When we
    // increment past 0xbf, we carry into the previous byte.
    if (i > 0 && next[i] == '\xbf') {
      next[i] = '\x80';
      continue;  // carry
    }
    ++next[i];
    break;  // no carry
  }
  DCHECK(StreamingUtf8Validator::Validate(next))
      << "Result \"" << next << "\" failed validation";
  return next;
}

typedef bool (*TestTargetType)(const std::string&);

// Run fuction |target| over |test_string| |times| times, and report the results
// using |description|.
bool RunTest(const std::string& description,
             TestTargetType target,
             const std::string& test_string,
             int times) {
  base::PerfTimeLogger timer(description.c_str());
  bool result = true;
  for (int i = 0; i < times; ++i) {
    result = target(test_string) && result;
  }
  timer.Done();
  return result;
}

// Construct a string by repeating |input| enough times to equal or exceed
// |length|.
std::string ConstructRepeatedTestString(const std::string& input,
                                        size_t length) {
  std::string output = input;
  while (output.length() * 2 < length) {
    output += output;
  }
  if (output.length() < length) {
    output += ConstructRepeatedTestString(input, length - output.length());
  }
  return output;
}

// Construct a string by expanding the range of UTF-8 sequences
// between |input_start| and |input_end|, inclusive, and then
// repeating the resulting string until it equals or exceeds |length|
// bytes. |input_start| and |input_end| must be valid UTF-8
// sequences.
std::string ConstructRangedTestString(const std::string& input_start,
                                      const std::string& input_end,
                                      size_t length) {
  std::string output = input_start;
  std::string input = input_start;
  while (output.length() < length && input != input_end) {
    input = NextUtf8Sequence(input);
    output += input;
  }
  if (output.length() < length) {
    output = ConstructRepeatedTestString(output, length);
  }
  return output;
}

struct TestFunctionDescription {
  TestTargetType function;
  const char* function_name;
};

bool IsStringUTF8(const std::string& str) {
  return base::IsStringUTF8(std::string_view(str));
}

// IsString7Bit is intentionally placed last so it can be excluded easily.
const TestFunctionDescription kTestFunctions[] = {
    {&StreamingUtf8Validator::Validate, "StreamingUtf8Validator"},
    {&IsStringUTF8, "IsStringUTF8"},
    {&IsString7Bit, "IsString7Bit"}};

// Construct a test string from `construct_test_string` for each of the lengths
// in `kTestLengths` in turn. For each string, run each test in `test_functions`
// for a number of iterations such that the total number of bytes validated
// is around 16MB. `num_bytes` and `type` are used to create a human-readable
// description of what `construct_test_string` is actually doing.
void RunSomeTests(
    int num_bytes,
    base::cstring_view type,
    base::RepeatingCallback<std::string(size_t length)> construct_test_string,
    const TestFunctionDescription* test_functions,
    size_t test_count) {
  for (auto length : kTestLengths) {
    const std::string test_string = construct_test_string.Run(length);
    const int real_length = static_cast<int>(test_string.length());
    const int times = (1 << 24) / real_length;
    for (size_t test_index = 0; test_index < test_count; ++test_index) {
      EXPECT_TRUE(
          RunTest(StringPrintf("%s: bytes=%d %s length=%d repeat=%d",
                               test_functions[test_index].function_name,
                               num_bytes, type.c_str(), real_length, times),
                  test_functions[test_index].function, test_string, times));
    }
  }
}

TEST(StreamingUtf8ValidatorPerfTest, OneByteRepeated) {
  RunSomeTests(
      1, "repeated",
      base::BindRepeating(ConstructRepeatedTestString, kOneByteSeqRangeStart),
      kTestFunctions, 3);
}

TEST(StreamingUtf8ValidatorPerfTest, OneByteRange) {
  RunSomeTests(1, "ranged",
               base::BindRepeating(ConstructRangedTestString,
                                   kOneByteSeqRangeStart, kOneByteSeqRangeEnd),
               kTestFunctions, 3);
}

TEST(StreamingUtf8ValidatorPerfTest, TwoByteRepeated) {
  RunSomeTests(
      2, "repeated",
      base::BindRepeating(ConstructRepeatedTestString, kTwoByteSeqRangeStart),
      kTestFunctions, 2);
}

TEST(StreamingUtf8ValidatorPerfTest, TwoByteRange) {
  RunSomeTests(2, "ranged",
               base::BindRepeating(ConstructRangedTestString,
                                   kTwoByteSeqRangeStart, kTwoByteSeqRangeEnd),
               kTestFunctions, 2);
}

TEST(StreamingUtf8ValidatorPerfTest, ThreeByteRepeated) {
  RunSomeTests(
      3, "repeated",
      base::BindRepeating(ConstructRepeatedTestString, kThreeByteSeqRangeStart),
      kTestFunctions, 2);
}

TEST(StreamingUtf8ValidatorPerfTest, ThreeByteRange) {
  RunSomeTests(
      3, "ranged",
      base::BindRepeating(ConstructRangedTestString, kThreeByteSeqRangeStart,
                          kThreeByteSeqRangeEnd),
      kTestFunctions, 2);
}

TEST(StreamingUtf8ValidatorPerfTest, FourByteRepeated) {
  RunSomeTests(
      4, "repeated",
      base::BindRepeating(ConstructRepeatedTestString, kFourByteSeqRangeStart),
      kTestFunctions, 2);
}

TEST(StreamingUtf8ValidatorPerfTest, FourByteRange) {
  RunSomeTests(
      4, "ranged",
      base::BindRepeating(ConstructRangedTestString, kFourByteSeqRangeStart,
                          kFourByteSeqRangeEnd),
      kTestFunctions, 2);
}

}  // namespace
}  // namespace base

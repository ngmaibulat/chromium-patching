// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "media/formats/mp4/h26x_annex_b_to_bitstream_converter.h"

#include <stdint.h>

#include <memory>

#include "testing/gtest/include/gtest/gtest.h"

namespace media {

TEST(H26xAnnexBToBitstreamConverterTest, ConvertH264KeyframeChunk) {
  std::vector<uint8_t> sps{0x00, 0x00, 0x00, 0x01, 0x27, 0x42, 0x00, 0x1E,
                           0x89, 0x8A, 0x12, 0x05, 0x01, 0x7F, 0xCA, 0x80};
  std::vector<uint8_t> pps1{0x00, 0x00, 0x00, 0x01, 0x28, 0xce, 0x3c, 0x80};
  std::vector<uint8_t> pps2{0x00, 0x00, 0x00, 0x01, 0x28, 0x53, 0x8f, 0x20};
  std::vector<uint8_t> pps3{0x00, 0x00, 0x00, 0x01, 0x28, 0x73, 0x8F, 0x20};
  std::vector<uint8_t> first_frame_idr{
      0x00, 0x00, 0x00, 0x01, 0x25, 0xb4, 0x00, 0x10, 0x00, 0x24, 0xff,
      0xff, 0xf8, 0x7a, 0x28, 0x00, 0x08, 0x0a, 0x7b, 0xdd
      // Encoded data omitted here, it's not important for NALU parsing
  };

  std::vector<uint8_t> first_chunk;
  first_chunk.insert(first_chunk.end(), sps.begin(), sps.end());
  first_chunk.insert(first_chunk.end(), pps1.begin(), pps1.end());
  first_chunk.insert(first_chunk.end(), pps2.begin(), pps2.end());
  first_chunk.insert(first_chunk.end(), pps3.begin(), pps3.end());
  first_chunk.insert(first_chunk.end(), first_frame_idr.begin(),
                     first_frame_idr.end());

  for (bool add_parameter_sets_in_bitstream : {false, true}) {
    H26xAnnexBToBitstreamConverter converter(VideoCodec::kH264,
                                             add_parameter_sets_in_bitstream);
    scoped_refptr<DecoderBuffer> output = converter.Convert(first_chunk);

    auto codec_profile_level = converter.GetCodecProfileLevel();
    EXPECT_EQ(codec_profile_level.codec, VideoCodec::kH264);
    EXPECT_EQ(codec_profile_level.profile,
              VideoCodecProfile::H264PROFILE_BASELINE);
    EXPECT_EQ(codec_profile_level.level, 30u);
  }
}

#if BUILDFLAG(ENABLE_HEVC_PARSER_AND_HW_DECODER)
TEST(H26xAnnexBToBitstreamConverterTest, ConvertH265KeyframeChunk) {
  std::vector<uint8_t> vps{0x00, 0x00, 0x00, 0x01, 0x40, 0x01, 0x0c,
                           0x01, 0xff, 0xff, 0x22, 0x20, 0x00, 0x00,
                           0x03, 0x00, 0x90, 0x00, 0x00, 0x03, 0x00,
                           0x00, 0x03, 0x00, 0x99, 0x2c, 0x09};
  std::vector<uint8_t> sps{
      0x00, 0x00, 0x00, 0x01, 0x42, 0x01, 0x01, 0x22, 0x20, 0x00, 0x00, 0x03,
      0x00, 0x90, 0x00, 0x00, 0x03, 0x00, 0x00, 0x03, 0x00, 0x99, 0xa0, 0x01,
      0xe0, 0x20, 0x02, 0x1c, 0x4d, 0x94, 0xbb, 0xb4, 0xa3, 0x32, 0xaa, 0xc0,
      0x5a, 0x84, 0x89, 0x04, 0x8a, 0x00, 0x00, 0x07, 0xd0, 0x00, 0x01, 0x86,
      0xa0, 0xe4, 0x68, 0x7c, 0x95, 0x00, 0x00, 0x89, 0x54, 0x00, 0x00, 0xf7,
      0x31, 0x00, 0x00, 0x44, 0xaa, 0x00, 0x00, 0x7b, 0x98, 0x88};
  std::vector<uint8_t> pps1{
      0x00, 0x00, 0x00, 0x01, 0x44, 0x01, 0xc0, 0x72, 0xb0, 0x3b, 0xc4, 0x0c,
      0x88, 0xc6, 0x70, 0x86, 0x18, 0x82, 0x08, 0x80, 0xc4, 0x10, 0x60, 0xa3,
      0x81, 0x23, 0x02, 0x06, 0x4c, 0x7f, 0xff, 0xf2, 0x88, 0x11, 0x26, 0x4e,
      0x4f, 0x27, 0xc4, 0x7e, 0x23, 0xf8, 0x8f, 0xc6, 0x7c, 0x67, 0x84, 0x38,
      0x43, 0xf1, 0x03, 0x23, 0x30, 0x87, 0x08, 0x78, 0x43, 0xe1, 0x8f, 0xc1,
      0x07, 0xf0, 0x51, 0xf8, 0x10, 0x3f, 0xff, 0xfc, 0xa3, 0xff, 0xff, 0xff,
      0xff, 0xff, 0xff, 0xff, 0xff, 0xc7, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
      0xff, 0xff, 0x28, 0x82, 0x12, 0x4c, 0x9c, 0x9e, 0x4f, 0x88, 0xfc, 0x47,
      0xf1, 0x1f, 0x8c, 0xf8, 0xcf, 0x08, 0x70, 0x87, 0xd6, 0x3f, 0xff, 0xff,
      0xff, 0xff, 0xff, 0xff, 0xff, 0xfd, 0x55, 0x20};
  std::vector<uint8_t> pps2{
      0x00, 0x00, 0x00, 0x01, 0x44, 0x01, 0x50, 0x1c, 0xac, 0x0e, 0xf1, 0x03,
      0x22, 0x31, 0x9c, 0x21, 0x86, 0x20, 0x82, 0x20, 0x31, 0x04, 0x18, 0x28,
      0xe0, 0x48, 0xc0, 0x81, 0x93, 0x0a, 0x07, 0x20, 0x93, 0x07, 0x9c, 0x0c,
      0x1f, 0x28, 0x81, 0x12, 0x64, 0xe4, 0xf2, 0x7c, 0x47, 0xe2, 0x3f, 0x88,
      0xfc, 0x67, 0xc6, 0x78, 0x43, 0x84, 0x3f, 0x10, 0x32, 0x33, 0x08, 0x70,
      0x87, 0x84, 0x3e, 0x18, 0xfc, 0x10, 0x7f, 0x05, 0x1f, 0x81, 0x03, 0xff,
      0xff, 0xca, 0x14, 0x11, 0x26, 0x4e, 0x4f, 0x11, 0xf1, 0x1f, 0x88, 0xfe,
      0x33, 0xf0, 0x87, 0xc2, 0x9f, 0xff, 0xc2, 0x81, 0x08, 0x43, 0x0a, 0x70,
      0xc7, 0x85, 0x04, 0x78, 0x20, 0xfc, 0x10, 0x7f, 0x06, 0x9f, 0xff, 0xff,
      0xfc, 0xa2, 0x08, 0x49, 0x32, 0x72, 0x79, 0x3e, 0x23, 0xf1, 0x1f, 0xc4,
      0x7e, 0x33, 0xe3, 0x3c, 0x21, 0xc2, 0x1f, 0x58, 0x50, 0x41, 0x22, 0x31,
      0x1c, 0x67, 0x8c, 0xf8, 0xcf, 0xc6, 0x7f, 0x08, 0x7f, 0xff, 0xff, 0xf5,
      0x54, 0x80};
  std::vector<uint8_t> first_frame_idr{
      0x00, 0x00, 0x00, 0x01, 0x28, 0x01, 0xac, 0x6d, 0xa0,
      0x7c, 0x96, 0x84, 0xdb, 0xcc, 0xf7, 0x4f, 0x9d, 0xf4,
      0x94, 0x85, 0x37, 0x06, 0x66, 0xf8
      // Encoded data omitted here, it's not important for NALU parsing
  };

  std::vector<uint8_t> first_chunk;
  first_chunk.insert(first_chunk.end(), vps.begin(), vps.end());
  first_chunk.insert(first_chunk.end(), sps.begin(), sps.end());
  first_chunk.insert(first_chunk.end(), pps1.begin(), pps1.end());
  first_chunk.insert(first_chunk.end(), pps2.begin(), pps2.end());
  first_chunk.insert(first_chunk.end(), first_frame_idr.begin(),
                     first_frame_idr.end());

  for (bool add_parameter_sets_in_bitstream : {false, true}) {
    H26xAnnexBToBitstreamConverter converter(VideoCodec::kHEVC,
                                             add_parameter_sets_in_bitstream);
    scoped_refptr<DecoderBuffer> output = converter.Convert(first_chunk);

    auto codec_profile_level = converter.GetCodecProfileLevel();
    EXPECT_EQ(codec_profile_level.codec, VideoCodec::kHEVC);
    EXPECT_EQ(codec_profile_level.profile,
              VideoCodecProfile::HEVCPROFILE_MAIN10);
    EXPECT_EQ(codec_profile_level.level, 153u);
  }
}
#endif

}  // namespace media

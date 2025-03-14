// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "media/cast/logging/stats_event_subscriber.h"

#include <stddef.h>
#include <stdint.h>

#include <memory>
#include <utility>

#include "base/memory/ref_counted.h"
#include "base/rand_util.h"
#include "base/test/simple_test_tick_clock.h"
#include "base/test/task_environment.h"
#include "base/time/tick_clock.h"
#include "base/time/time.h"
#include "base/values.h"
#include "media/cast/logging/log_event_dispatcher.h"
#include "media/cast/logging/logging_defines.h"
#include "media/cast/test/fake_receiver_time_offset_estimator.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace {
const int kReceiverOffsetSecs = 100;
}

namespace media::cast {

class StatsEventSubscriberTest : public ::testing::Test {
 protected:
  StatsEventSubscriberTest()
      : log_dispatcher_(std::make_unique<LogEventDispatcher>(
            task_environment_.GetMainThreadTaskRunner(),
            // NOTE: This is safe because we wait for this task before deleting
            // `this`.
            base::BindOnce(&StatsEventSubscriberTest::OnDispatcherDeletion,
                           base::Unretained(this)))),
        fake_offset_estimator_(base::Seconds(kReceiverOffsetSecs)) {
    // Synchronize the SimpleTestTickClock with the TaskEnvironment.
    receiver_clock_.SetNowTicks(NowTicks());
    receiver_clock_.Advance(base::Seconds(kReceiverOffsetSecs));
    log_dispatcher_->Subscribe(&fake_offset_estimator_);
  }

  ~StatsEventSubscriberTest() override {
    if (subscriber_.get()) {
      log_dispatcher_->Unsubscribe(subscriber_.get());
    }
    log_dispatcher_->Unsubscribe(&fake_offset_estimator_);

    dispatcher_deletion_cb_ = task_environment_.QuitClosure();
    log_dispatcher_.reset();
    task_environment_.RunUntilQuit();
  }

  void OnDispatcherDeletion() {
    ASSERT_TRUE(dispatcher_deletion_cb_);
    std::move(dispatcher_deletion_cb_).Run();
  }

  LogEventDispatcher& log_dispatcher() { return *log_dispatcher_; }

  base::TimeTicks NowTicks() const { return task_environment_.NowTicks(); }

  void AdvanceClocks(base::TimeDelta delta) {
    task_environment_.FastForwardBy(delta);
    receiver_clock_.Advance(delta);
  }

  void Init(EventMediaType event_media_type) {
    DCHECK(!subscriber_.get());
    subscriber_ = std::make_unique<StatsEventSubscriber>(
        event_media_type, *task_environment_.GetMockTickClock(),
        fake_offset_estimator_);
    log_dispatcher().Subscribe(subscriber_.get());
  }

  base::test::TaskEnvironment task_environment_{
      base::test::TaskEnvironment::TimeSource::MOCK_TIME};
  base::SimpleTestTickClock receiver_clock_;
  std::unique_ptr<LogEventDispatcher> log_dispatcher_;
  base::OnceClosure dispatcher_deletion_cb_;
  test::FakeReceiverTimeOffsetEstimator fake_offset_estimator_;
  std::unique_ptr<StatsEventSubscriber> subscriber_;
};

TEST_F(StatsEventSubscriberTest, CaptureEncode) {
  Init(VIDEO_EVENT);

  RtpTimeTicks rtp_timestamp;
  FrameId frame_id = FrameId::first();
  int extra_frames = 50;
  // Only the first |extra_frames| frames logged will be taken into account
  // when computing dropped frames.
  int num_frames = StatsEventSubscriber::kMaxFrameInfoMapSize + 50;
  int dropped_frames = 0;
  base::TimeTicks start_time = NowTicks();
  // Drop half the frames during the encode step.
  for (int i = 0; i < num_frames; i++) {
    std::unique_ptr<FrameEvent> capture_begin_event(new FrameEvent());
    capture_begin_event->timestamp = NowTicks();
    capture_begin_event->type = FRAME_CAPTURE_BEGIN;
    capture_begin_event->media_type = VIDEO_EVENT;
    capture_begin_event->rtp_timestamp = rtp_timestamp;
    log_dispatcher().DispatchFrameEvent(std::move(capture_begin_event));

    AdvanceClocks(base::Microseconds(10));
    std::unique_ptr<FrameEvent> capture_end_event(new FrameEvent());
    capture_end_event->timestamp = NowTicks();
    capture_end_event->type = FRAME_CAPTURE_END;
    capture_end_event->media_type = VIDEO_EVENT;
    capture_end_event->rtp_timestamp = rtp_timestamp;
    log_dispatcher().DispatchFrameEvent(std::move(capture_end_event));

    if (i % 2 == 0) {
      AdvanceClocks(base::Microseconds(10));
      std::unique_ptr<FrameEvent> encode_event(new FrameEvent());
      encode_event->timestamp = NowTicks();
      encode_event->type = FRAME_ENCODED;
      encode_event->media_type = VIDEO_EVENT;
      encode_event->rtp_timestamp = rtp_timestamp;
      encode_event->frame_id = frame_id;
      encode_event->size = 1024;
      encode_event->key_frame = true;
      encode_event->target_bitrate = 5678;
      encode_event->encoder_cpu_utilization = 9.10;
      encode_event->idealized_bitrate_utilization = 11.12;
      log_dispatcher().DispatchFrameEvent(std::move(encode_event));
    } else if (i < extra_frames) {
      dropped_frames++;
    }
    AdvanceClocks(base::Microseconds(34567));
    rtp_timestamp += RtpTimeDelta::FromTicks(90);
    frame_id++;
  }

  base::TimeTicks end_time = NowTicks();

  StatsEventSubscriber::StatsMap stats_map;
  subscriber_->GetStatsInternal(&stats_map);

  auto it = stats_map.find(StatsEventSubscriber::CAPTURE_FPS);
  ASSERT_TRUE(it != stats_map.end());

  base::TimeDelta duration = end_time - start_time;
  EXPECT_DOUBLE_EQ(it->second, num_frames / duration.InSecondsF());

  it = stats_map.find(StatsEventSubscriber::NUM_FRAMES_CAPTURED);
  ASSERT_TRUE(it != stats_map.end());

  EXPECT_DOUBLE_EQ(it->second, static_cast<double>(num_frames));

  it = stats_map.find(StatsEventSubscriber::NUM_FRAMES_DROPPED_BY_ENCODER);
  ASSERT_TRUE(it != stats_map.end());

  EXPECT_DOUBLE_EQ(it->second, static_cast<double>(dropped_frames));

  it = stats_map.find(StatsEventSubscriber::AVG_CAPTURE_LATENCY_MS);
  ASSERT_TRUE(it != stats_map.end());

  EXPECT_DOUBLE_EQ(it->second, static_cast<double>(0.01));
}

TEST_F(StatsEventSubscriberTest, Encode) {
  Init(VIDEO_EVENT);

  RtpTimeTicks rtp_timestamp;
  FrameId frame_id = FrameId::first();
  int num_frames = 10;
  base::TimeTicks start_time = NowTicks();
  AdvanceClocks(base::Microseconds(35678));
  base::TimeTicks first_event_time = NowTicks();
  base::TimeTicks last_event_time;
  int total_size = 0;
  for (int i = 0; i < num_frames; i++) {
    int size = 1000 + base::RandInt(-100, 100);
    total_size += size;
    std::unique_ptr<FrameEvent> encode_event(new FrameEvent());
    encode_event->timestamp = NowTicks();
    encode_event->type = FRAME_ENCODED;
    encode_event->media_type = VIDEO_EVENT;
    encode_event->rtp_timestamp = rtp_timestamp;
    encode_event->frame_id = frame_id;
    encode_event->size = size;
    encode_event->key_frame = true;
    encode_event->target_bitrate = 5678;
    encode_event->encoder_cpu_utilization = 9.10;
    encode_event->idealized_bitrate_utilization = 11.12;
    log_dispatcher().DispatchFrameEvent(std::move(encode_event));
    last_event_time = NowTicks();

    AdvanceClocks(base::Microseconds(35678));
    rtp_timestamp += RtpTimeDelta::FromTicks(90);
    frame_id++;
  }

  base::TimeTicks end_time = NowTicks();

  StatsEventSubscriber::StatsMap stats_map;
  subscriber_->GetStatsInternal(&stats_map);

  auto it = stats_map.find(StatsEventSubscriber::ENCODE_FPS);
  ASSERT_TRUE(it != stats_map.end());

  base::TimeDelta duration = end_time - start_time;
  EXPECT_DOUBLE_EQ(it->second, num_frames / duration.InSecondsF());

  it = stats_map.find(StatsEventSubscriber::ENCODE_KBPS);
  ASSERT_TRUE(it != stats_map.end());

  EXPECT_DOUBLE_EQ(it->second, total_size / duration.InMillisecondsF() * 8);

  it = stats_map.find(StatsEventSubscriber::FIRST_EVENT_TIME_MS);
  ASSERT_TRUE(it != stats_map.end());

  EXPECT_DOUBLE_EQ(
      it->second,
      (first_event_time - base::TimeTicks::UnixEpoch()).InMillisecondsF());

  it = stats_map.find(StatsEventSubscriber::LAST_EVENT_TIME_MS);
  ASSERT_TRUE(it != stats_map.end());

  EXPECT_DOUBLE_EQ(
      it->second,
      (last_event_time - base::TimeTicks::UnixEpoch()).InMillisecondsF());
}

TEST_F(StatsEventSubscriberTest, Decode) {
  Init(VIDEO_EVENT);

  RtpTimeTicks rtp_timestamp;
  FrameId frame_id = FrameId::first();
  int num_frames = 10;
  base::TimeTicks start_time = NowTicks();
  for (int i = 0; i < num_frames; i++) {
    std::unique_ptr<FrameEvent> decode_event(new FrameEvent());
    decode_event->timestamp = receiver_clock_.NowTicks();
    decode_event->type = FRAME_DECODED;
    decode_event->media_type = VIDEO_EVENT;
    decode_event->rtp_timestamp = rtp_timestamp;
    decode_event->frame_id = frame_id;
    log_dispatcher().DispatchFrameEvent(std::move(decode_event));

    AdvanceClocks(base::Microseconds(36789));
    rtp_timestamp += RtpTimeDelta::FromTicks(90);
    frame_id++;
  }

  base::TimeTicks end_time = NowTicks();

  StatsEventSubscriber::StatsMap stats_map;
  subscriber_->GetStatsInternal(&stats_map);

  auto it = stats_map.find(StatsEventSubscriber::DECODE_FPS);
  ASSERT_TRUE(it != stats_map.end());

  base::TimeDelta duration = end_time - start_time;
  EXPECT_DOUBLE_EQ(it->second, num_frames / duration.InSecondsF());
}

TEST_F(StatsEventSubscriberTest, PlayoutDelay) {
  Init(VIDEO_EVENT);

  RtpTimeTicks rtp_timestamp;
  FrameId frame_id = FrameId::first();
  int num_frames = 10;
  int late_frames = 0;
  for (int i = 0, delay_ms = -50; i < num_frames; i++, delay_ms += 10) {
    base::TimeDelta delay = base::Milliseconds(delay_ms);
    if (delay_ms > 0) {
      late_frames++;
    }
    std::unique_ptr<FrameEvent> playout_event(new FrameEvent());
    playout_event->timestamp = receiver_clock_.NowTicks();
    playout_event->type = FRAME_PLAYOUT;
    playout_event->media_type = VIDEO_EVENT;
    playout_event->rtp_timestamp = rtp_timestamp;
    playout_event->frame_id = frame_id;
    playout_event->delay_delta = delay;
    log_dispatcher().DispatchFrameEvent(std::move(playout_event));

    AdvanceClocks(base::Microseconds(37890));
    rtp_timestamp += RtpTimeDelta::FromTicks(90);
    frame_id++;
  }

  StatsEventSubscriber::StatsMap stats_map;
  subscriber_->GetStatsInternal(&stats_map);

  auto it = stats_map.find(StatsEventSubscriber::NUM_FRAMES_LATE);
  ASSERT_TRUE(it != stats_map.end());

  EXPECT_DOUBLE_EQ(it->second, late_frames);
}

TEST_F(StatsEventSubscriberTest, E2ELatency) {
  Init(VIDEO_EVENT);

  RtpTimeTicks rtp_timestamp;
  FrameId frame_id = FrameId::first();
  int num_frames = 10;
  base::TimeDelta total_latency;
  for (int i = 0; i < num_frames; i++) {
    std::unique_ptr<FrameEvent> capture_begin_event(new FrameEvent());
    capture_begin_event->timestamp = NowTicks();
    capture_begin_event->type = FRAME_CAPTURE_BEGIN;
    capture_begin_event->media_type = VIDEO_EVENT;
    capture_begin_event->rtp_timestamp = rtp_timestamp;
    log_dispatcher().DispatchFrameEvent(std::move(capture_begin_event));

    int latency_micros = 100000 + base::RandInt(-5000, 50000);
    base::TimeDelta latency = base::Microseconds(latency_micros);
    AdvanceClocks(latency);

    int delay_micros = base::RandInt(-50000, 50000);
    base::TimeDelta delay = base::Milliseconds(delay_micros);
    total_latency += latency;

    std::unique_ptr<FrameEvent> playout_event(new FrameEvent());
    playout_event->timestamp = receiver_clock_.NowTicks();
    playout_event->type = FRAME_PLAYOUT;
    playout_event->media_type = VIDEO_EVENT;
    playout_event->rtp_timestamp = rtp_timestamp;
    playout_event->frame_id = frame_id;
    playout_event->delay_delta = delay;
    log_dispatcher().DispatchFrameEvent(std::move(playout_event));

    rtp_timestamp += RtpTimeDelta::FromTicks(90);
    frame_id++;
  }

  StatsEventSubscriber::StatsMap stats_map;
  subscriber_->GetStatsInternal(&stats_map);

  auto it = stats_map.find(StatsEventSubscriber::AVG_E2E_LATENCY_MS);
  ASSERT_TRUE(it != stats_map.end());

  EXPECT_DOUBLE_EQ(it->second, total_latency.InMillisecondsF() / num_frames);
}

TEST_F(StatsEventSubscriberTest, Packets) {
  Init(VIDEO_EVENT);

  RtpTimeTicks rtp_timestamp;
  int num_packets = 10;
  int num_latency_recorded_packets = 0;
  base::TimeTicks start_time = NowTicks();
  int total_size = 0;
  int retransmit_total_size = 0;
  base::TimeDelta total_network_latency;
  base::TimeDelta total_queueing_latency;
  base::TimeDelta total_packet_latency;
  int num_packets_received = 0;
  int num_packets_retransmitted = 0;
  int num_packets_rtx_rejected = 0;

  base::TimeTicks sender_encoded_time = NowTicks();
  base::TimeTicks receiver_encoded_time = receiver_clock_.NowTicks();
  std::unique_ptr<FrameEvent> encode_event(new FrameEvent());
  encode_event->timestamp = sender_encoded_time;
  encode_event->type = FRAME_ENCODED;
  encode_event->media_type = VIDEO_EVENT;
  encode_event->rtp_timestamp = rtp_timestamp;
  encode_event->frame_id = FrameId::first();
  log_dispatcher().DispatchFrameEvent(std::move(encode_event));

  // Every 2nd packet will be retransmitted once.
  // Every 4th packet will be retransmitted twice.
  // Every 8th packet will be retransmitted 3 times + 1 rejected retransmission.
  for (int i = 0; i < num_packets; i++) {
    int size = 1000 + base::RandInt(-100, 100);
    total_size += size;

    std::unique_ptr<PacketEvent> send_event(new PacketEvent());
    send_event->timestamp = NowTicks();
    send_event->type = PACKET_SENT_TO_NETWORK;
    send_event->media_type = VIDEO_EVENT;
    send_event->rtp_timestamp = rtp_timestamp;
    send_event->frame_id = FrameId::first();
    send_event->packet_id = i;
    send_event->max_packet_id = num_packets - 1;
    send_event->size = size;
    log_dispatcher().DispatchPacketEvent(std::move(send_event));

    total_queueing_latency += NowTicks() - sender_encoded_time;

    int latency_micros = 20000 + base::RandInt(-10000, 10000);
    base::TimeDelta latency = base::Microseconds(latency_micros);
    // Latency is only recorded for packets that aren't retransmitted.
    if (i % 2 != 0) {
      total_network_latency += latency;
      total_packet_latency +=
          receiver_clock_.NowTicks() - receiver_encoded_time + latency;
      num_latency_recorded_packets++;
    }

    AdvanceClocks(latency);

    base::TimeTicks received_time = receiver_clock_.NowTicks();

    // Retransmission 1.
    AdvanceClocks(base::Microseconds(12345));
    if (i % 2 == 0) {
      std::unique_ptr<PacketEvent> retransmit_event(new PacketEvent());
      retransmit_event->timestamp = receiver_clock_.NowTicks();
      retransmit_event->type = PACKET_RETRANSMITTED;
      retransmit_event->media_type = VIDEO_EVENT;
      retransmit_event->rtp_timestamp = rtp_timestamp;
      retransmit_event->frame_id = FrameId::first();
      retransmit_event->packet_id = i;
      retransmit_event->max_packet_id = num_packets - 1;
      retransmit_event->size = size;
      log_dispatcher().DispatchPacketEvent(std::move(retransmit_event));

      retransmit_total_size += size;
      num_packets_retransmitted++;
    }

    // Retransmission 2.
    AdvanceClocks(base::Microseconds(13456));
    if (i % 4 == 0) {
      std::unique_ptr<PacketEvent> retransmit_event(new PacketEvent());
      retransmit_event->timestamp = receiver_clock_.NowTicks();
      retransmit_event->type = PACKET_RETRANSMITTED;
      retransmit_event->media_type = VIDEO_EVENT;
      retransmit_event->rtp_timestamp = rtp_timestamp;
      retransmit_event->frame_id = FrameId::first();
      retransmit_event->packet_id = i;
      retransmit_event->max_packet_id = num_packets - 1;
      retransmit_event->size = size;
      log_dispatcher().DispatchPacketEvent(std::move(retransmit_event));

      retransmit_total_size += size;
      num_packets_retransmitted++;
    }

    // Retransmission 3.
    AdvanceClocks(base::Microseconds(14567));
    if (i % 8 == 0) {
      std::unique_ptr<PacketEvent> retransmit_event(new PacketEvent());
      retransmit_event->timestamp = receiver_clock_.NowTicks();
      retransmit_event->type = PACKET_RETRANSMITTED;
      retransmit_event->media_type = VIDEO_EVENT;
      retransmit_event->rtp_timestamp = rtp_timestamp;
      retransmit_event->frame_id = FrameId::first();
      retransmit_event->packet_id = i;
      retransmit_event->max_packet_id = num_packets - 1;
      retransmit_event->size = size;
      log_dispatcher().DispatchPacketEvent(std::move(retransmit_event));

      std::unique_ptr<PacketEvent> reject_event(new PacketEvent());
      reject_event->timestamp = receiver_clock_.NowTicks();
      reject_event->type = PACKET_RTX_REJECTED;
      reject_event->media_type = VIDEO_EVENT;
      reject_event->rtp_timestamp = rtp_timestamp;
      reject_event->frame_id = FrameId::first();
      reject_event->packet_id = i;
      reject_event->max_packet_id = num_packets - 1;
      reject_event->size = size;
      log_dispatcher().DispatchPacketEvent(std::move(reject_event));

      retransmit_total_size += size;
      num_packets_retransmitted++;
      num_packets_rtx_rejected++;
    }

    std::unique_ptr<PacketEvent> receive_event(new PacketEvent());
    receive_event->timestamp = received_time;
    receive_event->type = PACKET_RECEIVED;
    receive_event->media_type = VIDEO_EVENT;
    receive_event->rtp_timestamp = rtp_timestamp;
    receive_event->frame_id = FrameId::first();
    receive_event->packet_id = i;
    receive_event->max_packet_id = num_packets - 1;
    receive_event->size = size;
    log_dispatcher().DispatchPacketEvent(std::move(receive_event));

    num_packets_received++;
  }

  base::TimeTicks end_time = NowTicks();
  base::TimeDelta duration = end_time - start_time;

  StatsEventSubscriber::StatsMap stats_map;
  subscriber_->GetStatsInternal(&stats_map);

  // Measure AVG_NETWORK_LATENCY_MS, TRANSMISSION_KBPS, RETRANSMISSION_KBPS.
  auto it = stats_map.find(StatsEventSubscriber::AVG_NETWORK_LATENCY_MS);
  ASSERT_TRUE(it != stats_map.end());

  EXPECT_DOUBLE_EQ(it->second, total_network_latency.InMillisecondsF() /
                                   num_latency_recorded_packets);

  it = stats_map.find(StatsEventSubscriber::AVG_QUEUEING_LATENCY_MS);
  ASSERT_TRUE(it != stats_map.end());

  EXPECT_DOUBLE_EQ(it->second,
                   total_queueing_latency.InMillisecondsF() / num_packets);

  it = stats_map.find(StatsEventSubscriber::AVG_PACKET_LATENCY_MS);
  ASSERT_TRUE(it != stats_map.end());

  EXPECT_DOUBLE_EQ(it->second, total_packet_latency.InMillisecondsF() /
                                   num_latency_recorded_packets);

  it = stats_map.find(StatsEventSubscriber::TRANSMISSION_KBPS);
  ASSERT_TRUE(it != stats_map.end());

  EXPECT_DOUBLE_EQ(it->second, total_size / duration.InMillisecondsF() * 8);

  it = stats_map.find(StatsEventSubscriber::RETRANSMISSION_KBPS);
  ASSERT_TRUE(it != stats_map.end());

  EXPECT_DOUBLE_EQ(it->second,
                   retransmit_total_size / duration.InMillisecondsF() * 8);

  it = stats_map.find(StatsEventSubscriber::NUM_PACKETS_SENT);
  ASSERT_TRUE(it != stats_map.end());

  EXPECT_DOUBLE_EQ(it->second, static_cast<double>(num_packets));

  it = stats_map.find(StatsEventSubscriber::NUM_PACKETS_RECEIVED);
  ASSERT_TRUE(it != stats_map.end());

  EXPECT_DOUBLE_EQ(it->second, static_cast<double>(num_packets_received));

  it = stats_map.find(StatsEventSubscriber::NUM_PACKETS_RETRANSMITTED);
  ASSERT_TRUE(it != stats_map.end());

  EXPECT_DOUBLE_EQ(it->second, static_cast<double>(num_packets_retransmitted));

  it = stats_map.find(StatsEventSubscriber::NUM_PACKETS_RTX_REJECTED);
  ASSERT_TRUE(it != stats_map.end());

  EXPECT_DOUBLE_EQ(it->second, static_cast<double>(num_packets_rtx_rejected));
}

std::optional<int> GetBucketCount(const base::Value::List& values,
                                  const std::string& bucket) {
  for (const base::Value& value : values) {
    if (!value.is_dict()) {
      continue;
    }
    const base::Value::Dict& dict = value.GetDict();
    if (!dict.contains(bucket)) {
      continue;
    }
    std::optional<int> bucket_count = dict.FindInt(bucket);
    if (!bucket_count.has_value()) {
      return std::nullopt;
    }
    return bucket_count;
  }
  return std::nullopt;
}

TEST_F(StatsEventSubscriberTest, Histograms) {
  Init(VIDEO_EVENT);
  AdvanceClocks(base::Milliseconds(123));

  RtpTimeTicks rtp_timestamp = RtpTimeTicks().Expand(UINT32_C(123));
  FrameId frame_id = FrameId::first();

  // 10 Frames with capture latency in the bucket of "10-14"ms.
  // 10 Frames with encode time in the bucket of "15-19"ms.
  for (int i = 0; i < 10; ++i) {
    ++frame_id;
    rtp_timestamp += RtpTimeDelta::FromTicks(1);

    std::unique_ptr<FrameEvent> capture_begin_event(new FrameEvent());
    capture_begin_event->timestamp = NowTicks();
    capture_begin_event->type = FRAME_CAPTURE_BEGIN;
    capture_begin_event->media_type = VIDEO_EVENT;
    capture_begin_event->rtp_timestamp = rtp_timestamp;
    log_dispatcher().DispatchFrameEvent(std::move(capture_begin_event));

    AdvanceClocks(base::Milliseconds(10));
    std::unique_ptr<FrameEvent> capture_end_event(new FrameEvent());
    capture_end_event->timestamp = NowTicks();
    capture_end_event->type = FRAME_CAPTURE_END;
    capture_end_event->media_type = VIDEO_EVENT;
    capture_end_event->rtp_timestamp = rtp_timestamp;
    log_dispatcher().DispatchFrameEvent(std::move(capture_end_event));

    AdvanceClocks(base::Milliseconds(15));
    std::unique_ptr<FrameEvent> encode_event(new FrameEvent());
    encode_event->timestamp = NowTicks();
    encode_event->type = FRAME_ENCODED;
    encode_event->media_type = VIDEO_EVENT;
    encode_event->rtp_timestamp = rtp_timestamp;
    encode_event->frame_id = frame_id;
    encode_event->size = 1024;
    encode_event->key_frame = true;
    encode_event->target_bitrate = 5678;
    encode_event->encoder_cpu_utilization = 9.10;
    encode_event->idealized_bitrate_utilization = 11.12;
    log_dispatcher().DispatchFrameEvent(std::move(encode_event));
  }

  // Send 3 packets for the last frame.
  // Queueing latencies are 100ms, 200ms and 300ms.
  for (int i = 0; i < 3; ++i) {
    AdvanceClocks(base::Milliseconds(100));
    std::unique_ptr<PacketEvent> send_event(new PacketEvent());
    send_event->timestamp = NowTicks();
    send_event->type = PACKET_SENT_TO_NETWORK;
    send_event->media_type = VIDEO_EVENT;
    send_event->rtp_timestamp = rtp_timestamp;
    send_event->frame_id = FrameId::first();
    send_event->packet_id = i;
    send_event->max_packet_id = 2;
    send_event->size = 123;
    log_dispatcher().DispatchPacketEvent(std::move(send_event));
  }

  // Receive 3 packets for the last frame.
  // Network latencies are 100ms, 200ms and 300ms.
  // Packet latencies are 400ms.
  AdvanceClocks(base::Milliseconds(100));
  for (int i = 0; i < 3; ++i) {
    std::unique_ptr<PacketEvent> receive_event(new PacketEvent());
    receive_event->timestamp = receiver_clock_.NowTicks();
    receive_event->type = PACKET_RECEIVED;
    receive_event->media_type = VIDEO_EVENT;
    receive_event->rtp_timestamp = rtp_timestamp;
    receive_event->frame_id = FrameId::first();
    receive_event->packet_id = i;
    receive_event->max_packet_id = 2;
    receive_event->size = 123;
    log_dispatcher().DispatchPacketEvent(std::move(receive_event));
  }

  std::unique_ptr<FrameEvent> playout_event(new FrameEvent());
  playout_event->timestamp = receiver_clock_.NowTicks();
  playout_event->type = FRAME_PLAYOUT;
  playout_event->media_type = VIDEO_EVENT;
  playout_event->rtp_timestamp = rtp_timestamp;
  playout_event->frame_id = frame_id;
  playout_event->delay_delta = base::Milliseconds(100);
  log_dispatcher().DispatchFrameEvent(std::move(playout_event));

  StatsEventSubscriber::SimpleHistogram* histogram;
  base::Value::List values;

  histogram = subscriber_->GetHistogramForTesting(
      StatsEventSubscriber::CAPTURE_LATENCY_MS_HISTO);
  ASSERT_TRUE(histogram);
  values = histogram->GetHistogram();
  EXPECT_EQ(GetBucketCount(values, "10-14"), 10);

  histogram = subscriber_->GetHistogramForTesting(
      StatsEventSubscriber::ENCODE_TIME_MS_HISTO);
  ASSERT_TRUE(histogram);
  values = histogram->GetHistogram();
  EXPECT_EQ(GetBucketCount(values, "15-19"), 10);

  histogram = subscriber_->GetHistogramForTesting(
      StatsEventSubscriber::QUEUEING_LATENCY_MS_HISTO);
  ASSERT_TRUE(histogram);
  values = histogram->GetHistogram();
  EXPECT_EQ(GetBucketCount(values, "100-119"), 1);
  EXPECT_EQ(GetBucketCount(values, "200-219"), 1);
  EXPECT_EQ(GetBucketCount(values, "300-319"), 1);

  histogram = subscriber_->GetHistogramForTesting(
      StatsEventSubscriber::NETWORK_LATENCY_MS_HISTO);
  ASSERT_TRUE(histogram);
  values = histogram->GetHistogram();
  EXPECT_EQ(GetBucketCount(values, "100-119"), 1);
  EXPECT_EQ(GetBucketCount(values, "200-219"), 1);
  EXPECT_EQ(GetBucketCount(values, "300-319"), 1);

  histogram = subscriber_->GetHistogramForTesting(
      StatsEventSubscriber::PACKET_LATENCY_MS_HISTO);
  ASSERT_TRUE(histogram);
  values = histogram->GetHistogram();
  EXPECT_EQ(GetBucketCount(values, "400-419"), 3);

  histogram = subscriber_->GetHistogramForTesting(
      StatsEventSubscriber::LATE_FRAME_MS_HISTO);
  ASSERT_TRUE(histogram);
  values = histogram->GetHistogram();
  EXPECT_EQ(GetBucketCount(values, "100-119"), 1);
}

}  // namespace media::cast

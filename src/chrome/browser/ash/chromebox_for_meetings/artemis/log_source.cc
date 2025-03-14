// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ash/chromebox_for_meetings/artemis/log_source.h"

#include <sys/stat.h>

#include "base/logging.h"
#include "base/strings/string_split.h"
#include "base/task/thread_pool.h"
#include "base/threading/scoped_blocking_call.h"
#include "base/time/time.h"
#include "chrome/browser/ash/chromebox_for_meetings/artemis/specialized_log_sources.h"

// Some files won't appear until after Chrome starts up. We'll try to open
// the file at every `Fetch()` request up to `kMaxFileOpenAttempts` times.
inline constexpr int kMaxFileOpenAttempts = 3;

namespace ash::cfm {

namespace {

// Byte cap for each call to log source's `RetrieveNextLogs()`.
// Ensures that we are not sending back a large amount of data
// when calling `GetNextData()`. Example: we have a log file
// that has unexpectedly large lines consecutively, which leads
// to a larger-than-usual batch.
constexpr size_t kLogBatchByteLimit = 100 * 1000;  // 100Kb

}  // namespace

LogSource::LogSource(const std::string& filepath,
                     base::TimeDelta poll_rate,
                     size_t batch_size)
    : LocalDataSource(poll_rate,
                      /*data_needs_redacting=*/true,
                      /*is_incremental=*/true),
      log_file_(filepath),
      batch_size_(batch_size) {
  recovery_offset_ = GetLastKnownOffsetFromStorage();
  InitializeFile();
}

LogSource::~LogSource() = default;

bool LogSource::InitializeFile() {
  if (!log_file_.OpenAtOffset(recovery_offset_)) {
    num_failed_open_attempts_ += 1;
    LOG(ERROR) << "Unable to open file " << GetDisplayName() << ". Trying "
               << kMaxFileOpenAttempts - num_failed_open_attempts_
               << " more times.";
    return false;
  }

  // Store this now so we can detect rotations later.
  last_known_inode_ = GetCurrentFileInode();
  return true;
}

std::unique_ptr<LogSource> LogSource::Create(const std::string& filename,
                                             base::TimeDelta poll_rate,
                                             size_t batch_size) {
  if (filename == kCfmAuditLogFile) {
    return std::make_unique<AuditLogSource>(poll_rate, batch_size);
  } else if (filename == kCfmBiosInfoLogFile) {
    return std::make_unique<BiosInfoLogSource>(poll_rate, batch_size);
  } else if (filename == kCfmEventlogLogFile) {
    return std::make_unique<EventlogLogSource>(poll_rate, batch_size);
  } else if (filename == kCfmVariationsListLogFile) {
    return std::make_unique<VariationsListLogSource>(poll_rate, batch_size);
  }

  return std::make_unique<LogSource>(filename, poll_rate, batch_size);
}

void LogSource::Fetch(FetchCallback callback) {
  // If the log file is not open by this point, and we're under our
  // max retry attempts, try to open it again.
  if (!log_file_.IsOpen()) {
    if (num_failed_open_attempts_ >= kMaxFileOpenAttempts ||
        !InitializeFile()) {
      std::move(callback).Run({});
      return;
    }
  }

  // Cache the current offset to use as a recovery offset in the
  // event of a crash. Note that this will NOT be flushed to disk
  // until we get a call to Flush(), so if we crash before then,
  // the last flushed offset will be used.
  //
  // Since the data buffer will continue filling up between this
  // call to Fetch() and the next call to Flush(), we MUST cache
  // this value here, or we risk dropping those logs.
  recovery_offset_ = log_file_.GetCurrentOffset();
  LocalDataSource::Fetch(std::move(callback));
}

void LogSource::Flush() {
  if (!log_file_.IsOpen()) {
    return;
  }
  // The upload succeeded, so update our recovery offset.
  PersistCurrentOffsetToStorage();
  LocalDataSource::Flush();
}

const std::string& LogSource::GetDisplayName() {
  return log_file_.GetFilePath();
}

std::vector<std::string> LogSource::GetNextData() {
  if (!log_file_.IsOpen()) {
    return {};
  }

  if (log_file_.IsInFailState()) {
    LOG(ERROR) << "Attempted to fetch logs for '" << log_file_.GetFilePath()
               << "', but the stream is dead";
    return {};
  }

  // If the file rotated from under us, reset it to start following the
  // new file. TODO(b/320996557): this might drop newest logs from old
  // rotated file.
  if (DidFileRotate()) {
    VLOG(1) << "Detected rotation in file '" << log_file_.GetFilePath() << "'";
    log_file_.CloseStream();
    log_file_.OpenAtOffset(0);
  }

  // ifstreams for files that have reached an EOF will not yield
  // newly-written lines unless the file is explicitly reset.
  // If we've hit an EOF, refresh the stream (close & re-open).
  //
  // NB: if the last read didn't cause an EOF, new lines will be
  // available immediately without the need to Refresh() first.
  if (log_file_.IsAtEOF()) {
    VLOG(3) << "Refreshing log file '" << log_file_.GetFilePath() << "'";
    log_file_.Refresh();
  }

  return log_file_.RetrieveNextLogs(batch_size_, kLogBatchByteLimit);
}

int LogSource::GetCurrentFileInode() {
  base::ScopedBlockingCall scoped_blocking_call(FROM_HERE,
                                                base::BlockingType::MAY_BLOCK);
  struct stat file_info;
  const std::string& filepath = log_file_.GetFilePath();

  if (stat(filepath.c_str(), &file_info) != 0) {
    LOG(ERROR) << "Unable to get inode of " << filepath;
    return kInvalidFileInode;
  }

  return file_info.st_ino;
}

bool LogSource::DidFileRotate() {
  int curr_inode = GetCurrentFileInode();

  if (curr_inode != kInvalidFileInode && last_known_inode_ != curr_inode) {
    if (PersistentDb::IsInitialized()) {
      PersistentDb::Get()->DeleteKeyIfExists(last_known_inode_);
    }
    last_known_inode_ = curr_inode;
    return true;
  }

  return false;
}

std::streampos LogSource::GetLastKnownOffsetFromStorage() {
  int default_value = 0;

  if (!PersistentDb::IsInitialized()) {
    return default_value;
  }

  int inode = GetCurrentFileInode();
  return PersistentDb::Get()->GetValueFromKey(inode, default_value);
}

void LogSource::PersistCurrentOffsetToStorage() {
  if (!PersistentDb::IsInitialized()) {
    LOG(WARNING) << "PersistentDb is inactive; recovery feature is disabled";
    return;
  }
  int inode = GetCurrentFileInode();
  PersistentDb::Get()->SaveValueToKey(inode, recovery_offset_);
}

}  // namespace ash::cfm

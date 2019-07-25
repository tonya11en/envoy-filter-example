#pragma once

#include <string>
#include <iostream>
#include <algorithm>
#include <mutex>
#include <thread>
#include <cmath>
#include <shared_mutex>

#include "envoy/server/filter_config.h"
#include "common/common/logger.h"

#include "http-filter-example/http_filter.pb.h"

namespace Envoy {
namespace Http {

typedef std::chrono::time_point<std::chrono::high_resolution_clock> TimePoint;

class TonyFilterSharedState : public Logger::Loggable<Logger::Id::filter> {
 public:
  TonyFilterSharedState() :
    update_interval_(std::chrono::milliseconds(100)),
    time_to_eval_(std::chrono::high_resolution_clock::now()),
    divisor_(1),
    target_latency_(std::chrono::microseconds(350000)),
    min_window_latency_(target_latency_) {
  }
  ~TonyFilterSharedState() {}

  bool shouldDrop() {
    // Returns true if successfully locked the mutex. This is a way to ensure
    // there's only a single request drop per decision to drop a request.
    return drop_mtx_.try_lock();
  }

  void update(const std::chrono::nanoseconds& rq_latency) {
    // Clunky hack, but fine for POC.
    std::unique_lock<std::mutex> ul(update_mtx_);

    const auto now = std::chrono::high_resolution_clock::now();

    const bool in_new_window = now > time_to_eval_;
    if (in_new_window) {
      // Determine if we should drop the next one.
      const bool drop_occurred = !drop_mtx_.try_lock();
      divisor_ = drop_occurred ? std::max(1, divisor_ - 1) : divisor_ + 1;
      time_to_eval_ = now + (update_interval_ / int(divisor_));
      if (min_window_latency_ > target_latency_) {
        // Update the divisor accordingly.
        drop_mtx_.unlock();
      }
      min_window_latency_ = rq_latency;
    } else {
      min_window_latency_ = std::min(min_window_latency_, rq_latency);
    }
  }

 private:
  std::chrono::nanoseconds update_interval_;

  TimePoint time_to_eval_;
  int divisor_;

  std::mutex drop_mtx_;
  std::mutex update_mtx_;

  std::atomic<int32_t> concurrency_;

  const std::chrono::nanoseconds target_latency_;
  std::chrono::nanoseconds min_window_latency_;
};
typedef std::shared_ptr<TonyFilterSharedState> TonyFilterSharedStatePtr;

class HttpSampleDecoderFilterConfig {
public:
  HttpSampleDecoderFilterConfig(const sample::Decoder& proto_config);

  const std::string& key() const { return key_; }
  const std::string& val() const { return val_; }

private:
  const std::string key_;
  const std::string val_;
};

typedef std::shared_ptr<HttpSampleDecoderFilterConfig> HttpSampleDecoderFilterConfigSharedPtr;

class HttpSampleDecoderFilter : public StreamFilter, Logger::Loggable<Logger::Id::filter> {
public:
  HttpSampleDecoderFilter(HttpSampleDecoderFilterConfigSharedPtr config, TonyFilterSharedStatePtr shared_state);
  ~HttpSampleDecoderFilter();

  // Http::StreamFilterBase
  void onDestroy() override;

  // Http::StreamDecoderFilter
  FilterHeadersStatus decodeHeaders(HeaderMap&, bool) override;
  FilterDataStatus decodeData(Buffer::Instance&, bool) override;
  FilterTrailersStatus decodeTrailers(HeaderMap&) override;
  void setDecoderFilterCallbacks(StreamDecoderFilterCallbacks&) override;

  // Http:StreamEncoderFilter
  FilterHeadersStatus encodeHeaders(HeaderMap& headers, bool end_stream) override;
  FilterDataStatus encodeData(Buffer::Instance& data, bool end_stream) override;
  FilterTrailersStatus encodeTrailers(HeaderMap& trailers) override;
  FilterMetadataStatus encodeMetadata(MetadataMap& metadata_map) override;
  void setEncoderFilterCallbacks(StreamEncoderFilterCallbacks& callbacks) override;
  Http::FilterHeadersStatus encode100ContinueHeaders(Http::HeaderMap&) override {
    return Http::FilterHeadersStatus::Continue;
  }

private:
  const HttpSampleDecoderFilterConfigSharedPtr config_;
  StreamDecoderFilterCallbacks* decoder_callbacks_;
  StreamEncoderFilterCallbacks* encoder_callbacks_;
  TonyFilterSharedStatePtr state_;

  TimePoint rq_start_time_;

  const LowerCaseString headerKey() const;
  const std::string headerValue() const;
};

} // namespace Http
} // namespace Envoy

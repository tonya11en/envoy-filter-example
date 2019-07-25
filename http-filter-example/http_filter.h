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

class TonyFilterSharedState {
 public:
  TonyFilterSharedState() :
    update_interval_(std::chrono::microseconds(500)),
    drop_(false),
    time_to_eval_(std::chrono::system_clock::now() + update_interval_),
    interval_divisor_(1),
    avg_measured_latency_(std::chrono::microseconds(0)),
    rq_encountered_(0),
    target_latency_(std::chrono::microseconds(2500000)) {
  }
  ~TonyFilterSharedState() {}

  bool dropEval() {
    // This is clunky, but fine for a POC.
    time_mtx_.lock();
    const auto drop = drop_;
    if (drop) {
      drop_ = false;
    }
    time_mtx_.unlock();
    return drop;
  }

  void update(const std::chrono::microseconds& rq_latency) {
    const auto now = std::chrono::system_clock::now();

    // This is clunky, but fine for a POC.
    time_mtx_.lock();

    const bool passed_time_window = now > time_to_eval_;
    if (passed_time_window) {
      drop_ = avg_measured_latency_ > target_latency_;

      if (drop_) {
        ++interval_divisor_;
      } else {
        interval_divisor_ = std::max(1, interval_divisor_ - 1);
      }

      time_to_eval_ = now + update_interval_ / int(sqrt(interval_divisor_));
      rq_encountered_ = 0;
    }

    ++rq_encountered_;
    avg_measured_latency_ = ((avg_measured_latency_ * (rq_encountered_ - 1)) + rq_latency) / (rq_encountered_);
    time_mtx_.unlock();

//    ENVOY_LOG(info, "@tallen avg measured latency {}", avg_measured_latency_.count());
  }

 private:
  std::chrono::microseconds update_interval_;
  bool drop_;

  std::chrono::time_point<std::chrono::system_clock> time_to_eval_;
  int interval_divisor_;
  std::mutex time_mtx_;

  std::chrono::microseconds avg_measured_latency_;
  int rq_encountered_;

  const std::chrono::microseconds target_latency_;
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

  std::chrono::time_point<std::chrono::system_clock> rq_start_time_;

  const LowerCaseString headerKey() const;
  const std::string headerValue() const;
};

} // namespace Http
} // namespace Envoy

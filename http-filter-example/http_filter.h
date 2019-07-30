#pragma once

#include <algorithm>
#include <cmath>
#include <future>
#include <iostream>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <thread>
#include <vector>

#include "envoy/server/filter_config.h"
#include "common/common/logger.h"

#include "http-filter-example/http_filter.pb.h"

namespace Envoy {
namespace Http {

typedef std::chrono::time_point<std::chrono::high_resolution_clock> TimePoint;

class TonyFilterSharedState : public Logger::Loggable<Logger::Id::filter> {
 public:
  TonyFilterSharedState() :
    allowed_queue_(8), // Q
    max_limit_(1000),
    window_sample_count_(0),
    in_flight_count_(0),
    concurrency_(1),
    time_window_(std::chrono::milliseconds(100)),
    shutdown_(false),
    calculating_min_rtt_(true),
    min_rtt_calculation_window_(std::chrono::seconds(5)) {
      sample_reset_thread_ = std::thread(&TonyFilterSharedState::resetSampleWorkerJob, this);
      min_rtt_calculator_thread_ = std::thread(&TonyFilterSharedState::minRTTCalculator, this);
    }

  ~TonyFilterSharedState() {
    shutdown_.store(true);

    // Wait for return.
    sample_reset_thread_.join();
//    min_rtt_calculator_thread_.join();
  }

  // Returns true if request has been allowed through. If let through, the
  // in-flight request count is incremented.
  bool letThrough();

  // Takes a latency sample of a completed request and decrements the in-flight
  // count.
  void sample(const std::chrono::nanoseconds& rq_latency);

  void sampleMinRTT(const std::chrono::nanoseconds& rq_latency);

  void minRTTCalculator();

  // Updates the running sample via average or p50/p95/p99.
  std::chrono::microseconds getRunningSample();
  uint32_t latencySamplePercentile(std::vector<uint32_t>& latency_samples, const int percentile);

  void resetSampleWorkerJob();



 private:
  std::vector<uint32_t> latency_samples_;

  std::chrono::nanoseconds min_rtt_;
  std::chrono::nanoseconds sample_rtt_;
  int allowed_queue_;
  int max_limit_;

//  std::chrono::nanoseconds running_sample_rtt_;
  std::atomic<int> window_sample_count_;
  std::mutex sample_mtx_;

  int in_flight_count_;
  std::atomic<int> concurrency_;
  std::mutex counter_mtx_;

  std::chrono::milliseconds time_window_;

  // The background threads will halt if this is true.
  std::atomic<bool> shutdown_;
  std::thread sample_reset_thread_;

  std::atomic<bool> calculating_min_rtt_;
  std::thread min_rtt_calculator_thread_;
  std::chrono::seconds min_rtt_calculation_window_;
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

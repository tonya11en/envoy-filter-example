#include <string>
#include <iostream>

#include "http_filter.h"

#include "common/common/logger.h"
#include "envoy/server/filter_config.h"

namespace Envoy {
namespace Http {

HttpSampleDecoderFilterConfig::HttpSampleDecoderFilterConfig(
    const sample::Decoder& proto_config)
    : key_(proto_config.key()), val_(proto_config.val()) {
    
    }

HttpSampleDecoderFilter::HttpSampleDecoderFilter(HttpSampleDecoderFilterConfigSharedPtr config, TonyFilterSharedStatePtr shared_state)
  : config_(config), state_(shared_state) {} 

HttpSampleDecoderFilter::~HttpSampleDecoderFilter() {}

void HttpSampleDecoderFilter::onDestroy() {}

const LowerCaseString HttpSampleDecoderFilter::headerKey() const {
  return LowerCaseString(config_->key());
}

const std::string HttpSampleDecoderFilter::headerValue() const {
  return config_->val();
}

FilterHeadersStatus HttpSampleDecoderFilter::decodeHeaders(HeaderMap& headers, bool end_stream) {
  // add a header
  headers.addCopy(headerKey(), headerValue());

  if (!end_stream) {
    return FilterHeadersStatus::Continue;
  }

  rq_start_time_ = std::chrono::high_resolution_clock::now();
  ENVOY_LOG(info, "@tallen set rq start time {}", rq_start_time_.time_since_epoch().count());

  if (state_->letThrough()) {
    return FilterHeadersStatus::Continue;
  }

  ENVOY_LOG(info, "@tallen ========== DROPPING ==========");
  decoder_callbacks_->sendLocalReply(
    Http::Code::ServiceUnavailable, "filler words", nullptr,
    absl::nullopt, "more filler words");
  return Http::FilterHeadersStatus::StopIteration;
}

FilterDataStatus HttpSampleDecoderFilter::decodeData(Buffer::Instance&, bool) {
  return FilterDataStatus::Continue;
}

FilterTrailersStatus HttpSampleDecoderFilter::decodeTrailers(HeaderMap&) {
  return FilterTrailersStatus::Continue;
}

void HttpSampleDecoderFilter::setDecoderFilterCallbacks(StreamDecoderFilterCallbacks& callbacks) {
  decoder_callbacks_ = &callbacks;
}

FilterHeadersStatus HttpSampleDecoderFilter::encodeHeaders(HeaderMap&, bool end_stream) {
  if (end_stream) {
    const std::chrono::microseconds rq_latency_ =
      std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - rq_start_time_);
    ENVOY_LOG(info, "@tallen measured rq latency {}", rq_latency_.count());
    state_->sample(rq_latency_);
    state_->finish();
  }

  return FilterHeadersStatus::Continue;
}

FilterDataStatus HttpSampleDecoderFilter::encodeData(Buffer::Instance&, bool) {
  return FilterDataStatus::Continue;
}

FilterTrailersStatus HttpSampleDecoderFilter::encodeTrailers(HeaderMap&) {
  return FilterTrailersStatus::Continue;
}

FilterMetadataStatus HttpSampleDecoderFilter::encodeMetadata(MetadataMap&) {
  return FilterMetadataStatus::Continue;
}

void HttpSampleDecoderFilter::setEncoderFilterCallbacks(StreamEncoderFilterCallbacks& callbacks) {
  encoder_callbacks_ = &callbacks;
}

bool TonyFilterSharedState::letThrough() {
  std::unique_lock<std::mutex> ul(counter_mtx_);
  if (in_flight_count_ < concurrency_.load()) {
    ++in_flight_count_;
    return true;
  }
  return false;
}

void TonyFilterSharedState::sample(const std::chrono::nanoseconds& rq_latency) {
  std::unique_lock<std::mutex> ul(sample_mtx_); 
  ++window_sample_count_;
  running_avg_rtt_ =
    (running_avg_rtt_ * (window_sample_count_ - 1) + rq_latency) / window_sample_count_;
}

void TonyFilterSharedState::finish() {
  std::unique_lock<std::mutex> ul(counter_mtx_);
  in_flight_count_--;
}

void asyncResetSamples(std::atomic<bool> shutdown, std::mutex sample_mtx, int& window_sample_count, std::chrono::nanoseconds& running_avg_rtt, std::chrono::nanoseconds sample_rtt, std::atomic<int> concurrency, const int allowed_queue) {
  while (!shutdown.load()) {
    std::this_thread::sleep_for(time_window_);
    std::unique_lock<std::mutex> ul(sample_mtx_);
    if (window_sample_count == 0) {
      continue;
    }

    sample_rtt = running_avg_rtt;

    // Gradient. TODO verify math.
    const double gradient =
      min(2.0, (double(min_rtt_.count()) / sample_rtt.count()));
    const int limit = concurrency.load() * gradient + allowed_queue;
    concurrency.store(limit);

    ENVOY_LOG(info, "setting new concurrency limit: {}", concurrency.load());

    running_avg_rtt = std::chrono::nanoseconds(0);
    window_sample_count = 0;
  }
}

} // namespace Http
} // namespace Envoy

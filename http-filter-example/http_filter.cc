#include <algorithm>
#include <future>
#include <iostream>
#include <string>
#include <vector>

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

  if (state_->letThrough()) {
    return FilterHeadersStatus::Continue;
  }

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
      std::chrono::duration_cast<std::chrono::microseconds>(
          std::chrono::high_resolution_clock::now() - rq_start_time_);
    state_->sample(rq_latency_);
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
  std::unique_lock<std::mutex> ul(sample_mtx_); 
  if (in_flight_count_ < concurrency_.load()) {
    ++in_flight_count_;
    return true;
  }
  return false;
}

uint32_t TonyFilterSharedState::latencySamplePercentile(std::vector<uint32_t>& latency_samples, const int percentile) {
  // TODO perf: use quickselect or something instead of sorting.
  std::sort(latency_samples.begin(), latency_samples.end());
  const int index = latency_samples.size() * (percentile / 100.0);
  return latency_samples[index];
}

void TonyFilterSharedState::minRTTCalculator() {
  while (!shutdown_.load()) {
    {
      absl::WriterMutexLock wml(&min_rtt_calc_mtx_);

      {
        std::unique_lock<std::mutex> sample_ul(sample_mtx_); 
        latency_samples_.clear();
      }

      concurrency_.store(1);

      // TODO: make this RTT gathering time configurable. Do it per-request.
      while (window_sample_count_.load() < 50 && !shutdown_.load()) {
        ENVOY_LOG(info, "@tallen waiting for latency sample at {}", window_sample_count_.load());
        std::this_thread::sleep_for(std::chrono::seconds(1));
      }

      min_rtt_ = getRunningSample();
      ENVOY_LOG(info, "@tallen got min rtt val of {}", min_rtt_.count());
      ENVOY_LOG(info, "@tallen sleeping for {} secs", min_rtt_calculation_window_.count());
    }

    std::this_thread::sleep_for(min_rtt_calculation_window_);

    ENVOY_LOG(info, "@tallen done sleeping");
  }
}

std::chrono::microseconds TonyFilterSharedState::getRunningSample() {
  // @tallen get the top kth latency here before locking.
  std::vector<uint32_t> latency_samples;

  {
    std::unique_lock<std::mutex> ul(sample_mtx_);
    latency_samples.reserve(latency_samples_.size());
    std::swap(latency_samples_, latency_samples);
  }

  return std::chrono::microseconds(latencySamplePercentile(latency_samples, 50));
}

void TonyFilterSharedState::sample(const std::chrono::nanoseconds& rq_latency) {
  if (!min_rtt_calc_mtx_.ReaderTryLock()) {
    sampleMinRTT(rq_latency);
    return;
  }

  {
    std::unique_lock<std::mutex> ul(sample_mtx_); 
    const uint32_t usec_count =
      std::chrono::duration_cast<std::chrono::microseconds>(rq_latency).count();
    latency_samples_.emplace_back(usec_count);
    window_sample_count_++;
    in_flight_count_--;
  }

  min_rtt_calc_mtx_.ReaderUnlock();
}

void TonyFilterSharedState::sampleMinRTT(const std::chrono::nanoseconds& rq_latency) {
  std::unique_lock<std::mutex> ul(sample_mtx_); 
  const uint32_t usec_count =
    std::chrono::duration_cast<std::chrono::microseconds>(rq_latency).count();
  latency_samples_.emplace_back(usec_count);
  window_sample_count_++;
  ENVOY_LOG(info, "@tallen added min rtt sample to vector");
  in_flight_count_--;
}

void TonyFilterSharedState::resetSampleWorkerJob() {
  while (!shutdown_.load()) {
    std::this_thread::sleep_for(time_window_);

    if (window_sample_count_.load() == 0) {
      continue;
    }

    absl::ReaderMutexLock rml(&min_rtt_calc_mtx_);

    const auto running_sample = getRunningSample();

    std::unique_lock<std::mutex> ul(sample_mtx_);
    ENVOY_LOG(info, "@tallen window sample count: {}", window_sample_count_);

    sample_rtt_ = std::chrono::duration_cast<std::chrono::nanoseconds>(running_sample);
    ENVOY_LOG(info, "@tallen min rtt: {}", min_rtt_.count());
    ENVOY_LOG(info, "@tallen sample rtt: {}", sample_rtt_.count());

    // Gradient. TODO verify math.
    const double gradient =
      std::min(2.0, (double(min_rtt_.count()) / sample_rtt_.count()));
    const int limit = concurrency_.load() * gradient + allowed_queue_;
    concurrency_.store(std::max(1, std::min(limit, max_limit_)));

    ENVOY_LOG(info, "@tallen gradient: {}", gradient);
    ENVOY_LOG(info, "@tallen limit: {}", limit);

    ENVOY_LOG(info, "setting new concurrency limit: {}", concurrency_.load());

    window_sample_count_ = 0;
  }
}

} // namespace Http
} // namespace Envoy

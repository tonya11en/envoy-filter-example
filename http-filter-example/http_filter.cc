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

  // @tallen REMOVE
//  return FilterHeadersStatus::Continue;

  if (!end_stream) {
    return FilterHeadersStatus::Continue;
  }

  rq_start_time_ = std::chrono::high_resolution_clock::now();
  ENVOY_LOG(info, "@tallen set rq start time {}", rq_start_time_.time_since_epoch().count());

  if (state_->shouldDrop()) {
    ENVOY_LOG(info, "@tallen ========== DROPPING ==========");
    decoder_callbacks_->sendLocalReply(
      Http::Code::ServiceUnavailable, "filler words", nullptr,
      absl::nullopt, "more filler words");
    return Http::FilterHeadersStatus::StopIteration;
  }

  return FilterHeadersStatus::Continue;
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
  // @tallen REMOVE
//  return FilterHeadersStatus::Continue;

  if (end_stream) {
    const std::chrono::microseconds rq_latency_ =
      std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - rq_start_time_);
    ENVOY_LOG(info, "@tallen measured rq latency {}", rq_latency_.count());
    state_->update(rq_latency_);
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

} // namespace Http
} // namespace Envoy

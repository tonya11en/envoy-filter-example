#pragma once

#include <string>
#include <mutex>
#include <shared_mutex>

#include "envoy/server/filter_config.h"

#include "http-filter-example/http_filter.pb.h"

namespace Envoy {
namespace Http {

class TonyFilterSharedState {
  TonyFilterSharedState() :
    update_interval_(std::chrono::duration_cast<std::chrono::microseconds>(500)),
    last_drop_time_(std::chrono::system_clock::now()),
    time_to_eval_(last_drop_time_ + update_interval_),
    target_latency_(std::chrono::duration_cast<std::chrono::milliseconds>(500)),
    interval_divisor_(1),
    should_drop_(false) {
  }

  private:
  bool shouldDrop() const {
    return should_drop_.load();
  }

  void update(const std::chono::duration& rq_latency) {
    
  }

  std::chrono::duration update_interval_;
  std::chrono::time_point<std::chrono::system_clock> last_drop_time_;
  std::chrono::time_point<std::chrono::system_clock> time_to_eval_;
  std::chrono::duration target_latency_;
  int interval_divisor_;
  std::shared_mutex time_mtx_;
  std::atomic<bool> should_drop_;
};

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

class HttpSampleDecoderFilter : public StreamFilter {
public:
  HttpSampleDecoderFilter(HttpSampleDecoderFilterConfigSharedPtr);
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

private:
  const HttpSampleDecoderFilterConfigSharedPtr config_;
  StreamDecoderFilterCallbacks* decoder_callbacks_;
  StreamEncoderFilterCallbacks* encoder_callbacks_;

  const LowerCaseString headerKey() const;
  const std::string headerValue() const;

  bool shouldDrop() const;
  void updateLogic();

  // @tallen
  std::chrono::time_point<std::chrono::system_clock> last_drop_time_;
  std::chrono::time_point<std::chrono::system_clock> next_drop_time_;
  std::chrono::duration update_interval_;
  std::chrono::duration target_latency_;
  int interval_divisor_;
  std::shared_mutex time_mtx_;
  bool should_drop_;
};

} // namespace Http
} // namespace Envoy

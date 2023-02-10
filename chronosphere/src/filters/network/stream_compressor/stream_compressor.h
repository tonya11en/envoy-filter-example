#pragma once

#include <vector>

#include "envoy/network/filter.h"
#include "source/common/common/logger.h"

#include "absl/strings/string_view.h"
#include "zstd.h"

namespace Envoy {
namespace Filter {

/**
 * Implementation of the zstd stream compressor filter.
 */
class StreamCompressorFilter : public Network::ReadFilter, 
                               Logger::Loggable<Logger::Id::filter> {
public:
  StreamCompressorFilter();

  ~StreamCompressorFilter() { 
    if (compression_ctx_ != nullptr) {
      ZSTD_freeCCtx(compression_ctx_); 
    }
  }

  // Network::ReadFilter
  Network::FilterStatus onData(Buffer::Instance& data, bool end_stream) override;
  Network::FilterStatus onNewConnection() override;
  void initializeReadFilterCallbacks(Network::ReadFilterCallbacks& callbacks) override {
    read_callbacks_ = &callbacks;
  }

  Network::FilterStatus encodeStream(Buffer::Instance& data, bool end_stream);
  Network::FilterStatus decodeStream(Buffer::Instance& data, bool end_stream);

private:
  Network::ReadFilterCallbacks* read_callbacks_{};

  ZSTD_CCtx* compression_ctx_;

  // Stores the compressed data.
  std::vector<char> input_buf_;
  std::vector<char> output_buf_;

  bool hasMagicNumber(Buffer::Instance& data) {
    static constexpr std::array<char, 4> mn = {
      static_cast<char>(0xFD), 
      static_cast<char>(0x2F), 
      static_cast<char>(0xB5), 
      static_cast<char>(0x28)};
    static absl::string_view zstdMagicNumber_(mn.data(), 4);
    return data.startsWith(zstdMagicNumber_);
  }

  enum StreamIdentification {
    UNIDENTIFIED,
    ZSTD_ENCODE,
    ZSTD_DECODE,
  };
  StreamIdentification stream_identification_{StreamIdentification::UNIDENTIFIED};

  bool bypass_{};
};

} // namespace Filter
} // namespace Envoy

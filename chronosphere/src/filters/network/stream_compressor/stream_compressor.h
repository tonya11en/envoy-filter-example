#pragma once

#include <vector>

#include "envoy/network/filter.h"
#include "source/common/common/logger.h"

#include "zstd.h"

namespace Envoy {
namespace Filter {

/**
 * Implementation of the zstd stream compressor filter.
 */
class StreamCompressorFilter : public Network::ReadFilter, 
                               public Network::WriteFilter, 
                               Logger::Loggable<Logger::Id::filter> {
public:
  StreamCompressorFilter();
  ~StreamCompressorFilter() { ZSTD_freeCCtx(compression_ctx_); }

  // Network::ReadFilter
  Network::FilterStatus onData(Buffer::Instance& data, bool end_stream) override;
  Network::FilterStatus onNewConnection() override;
  void initializeReadFilterCallbacks(Network::ReadFilterCallbacks& callbacks) override {
    read_callbacks_ = &callbacks;
  }

  // Network::WriteFilter
  Network::FilterStatus onWrite(Buffer::Instance& data, bool end_stream) override;

private:
  Network::ReadFilterCallbacks* read_callbacks_{};

  ZSTD_CCtx* compression_ctx_;

  // Stores the compressed data.
  std::vector<char> input_buf_;
  std::vector<char> output_buf_;

  bool bypass_{};
};

} // namespace Filter
} // namespace Envoy

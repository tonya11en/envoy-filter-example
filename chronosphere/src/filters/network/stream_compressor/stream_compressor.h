#pragma once

#include <vector>

#include "envoy/network/filter.h"
#include "source/common/common/logger.h"
#include "envoy/stats/stats_macros.h"
#include "envoy/stats/scope.h"
#include "envoy/stats/stats.h"


#include "absl/strings/string_view.h"
#include "zstd.h"

namespace Envoy {
namespace Filter {

#define ALL_STREAM_COMPRESSOR_STATS(COUNTER) \
  COUNTER(encoded_bytes) \
  COUNTER(decoded_bytes) \
  COUNTER(decoder_stream_flush) \
  COUNTER(encoder_stream_total) \
  COUNTER(decoder_stream_total)

struct StreamCompressorStats {
  ALL_STREAM_COMPRESSOR_STATS(GENERATE_COUNTER_STRUCT)
};

/**
 * Implementation of the zstd stream compressor filter.
 */
class StreamCompressorFilter : public Network::Filter, 
                               Logger::Loggable<Logger::Id::filter> {
 public:
  StreamCompressorFilter(Stats::Scope& scope);

  ~StreamCompressorFilter() { 
    if (compression_ctx_ != nullptr) {
      ZSTD_freeCCtx(compression_ctx_); 
    }

    if (decompression_ctx_ != nullptr) {
      ZSTD_freeDCtx(decompression_ctx_);
    }
  }

  // Network::Filter
  Network::FilterStatus onData(Buffer::Instance& data, bool end_stream) override;
  Network::FilterStatus onNewConnection() override;
  void initializeReadFilterCallbacks(Network::ReadFilterCallbacks& callbacks) override {
    read_callbacks_ = &callbacks;
  }
  void initializeWriteFilterCallbacks(Network::WriteFilterCallbacks& callbacks) override {
    write_callbacks_ = &callbacks;
  }
  Network::FilterStatus onWrite(Buffer::Instance& data, bool end_stream) override;

 protected:
  enum StreamIdentification {
    UNSET,
    ZSTD_ENCODE,
    ZSTD_DECODE,
  };

  // Returns true if the data buffer starts with the zstd magic number,
  // indicating the start of a frame. We'll use this to determine what to do
  // with a particular stream.
  bool hasMagicNumber(Buffer::Instance& data) const {
    static constexpr std::array<char, 4> magic = {
      static_cast<char>(0x28),
      static_cast<char>(0xB5), 
      static_cast<char>(0x2F), 
      static_cast<char>(0xFD)}; 
    static absl::string_view zstdMagicNumber(magic.data(), 4);
    return data.startsWith(zstdMagicNumber);
  }

  void maybeIdentifyStreamType(Buffer::Instance& data) {
    if (stream_identification_ == StreamIdentification::UNSET) {
      // Classify the stream. If the magic number exists in the first 4 bytes, we
      // will decode anything coming into this stream.
        if (hasMagicNumber(data)) {
          stats_.decoder_stream_total_.inc();
          stream_identification_ =  StreamIdentification::ZSTD_DECODE;
        } else {
          stats_.encoder_stream_total_.inc();
          stream_identification_ = StreamIdentification::ZSTD_ENCODE;
        }
    }
  }

  Network::FilterStatus encodeStream(Buffer::Instance& data, bool end_stream);
  Network::FilterStatus decodeStream(Buffer::Instance& data, bool end_stream);

 private:
  // Stores decoder state for this stream. Since we have no control over the
  // zstd frames in the decode stream, we'll need to maintain state across the
  // lifetime of this connection. 
  //
  // The `decoder_state_` will track the result of the previous decompression.
  // If it is a non-error code that is >0, it means there is still some
  // decoding or flushing to do to complete the current frame and the decode
  // operation will need to straddle multiple calls to `decodeStream`.
  // Therefore, we can't just reset the state of the output zbuf after every
  // call to `decodeStream`.
  //
  // We're able to get away with this for encodes because we control when to
  // flush frames and forward over the wire.
  std::vector<char> decoder_output_buf_;
  ZSTD_outBuffer decoder_zbuf_out_;
  size_t decoder_state_{0};

  void maybeFlushData(Buffer::Instance& data);
  size_t doCompress( Buffer::Instance& data, ZSTD_inBuffer& zbuf_in, ZSTD_outBuffer& zbuf_out, ZSTD_EndDirective mode);
  void doDecompress(Buffer::Instance& data, ZSTD_inBuffer& zbuf_in);

 private:
  StreamCompressorStats generateStats(Stats::Scope& scope);

  StreamIdentification stream_identification_{StreamIdentification::UNSET};

  Network::ReadFilterCallbacks* read_callbacks_{};
  Network::WriteFilterCallbacks* write_callbacks_{};

  ZSTD_CCtx* compression_ctx_;
  ZSTD_DCtx* decompression_ctx_;

  // Stores the compressed data.
  std::vector<char> encoder_output_buf_;

  // TODO: set this via config
  bool bypass_{false};

  StreamCompressorStats stats_;
};

} // namespace Filter
} // namespace Envoy

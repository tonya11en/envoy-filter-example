#include "stream_compressor.h"

#include "envoy/buffer/buffer.h"
#include "envoy/network/connection.h"

#include "source/common/common/assert.h"

#include "absl/strings/string_view.h"
#include "zstd.h"

namespace Envoy {
namespace Filter {

StreamCompressorFilter::StreamCompressorFilter(Stats::Scope& scope) :
  stats_(generateStats(scope)) {}

StreamCompressorStats StreamCompressorFilter::generateStats(Stats::Scope& scope) {
  const std::string prefix = "stream_compressor.";
  return {ALL_STREAM_COMPRESSOR_STATS(POOL_COUNTER_PREFIX(scope, prefix))};
}

Network::FilterStatus StreamCompressorFilter::onNewConnection() {
  // TODO(tallen): 
  // Allow configurable stream parameters in filter config such as:
  //   * Compression level.
  //   * Dictionary.
  compression_ctx_ = ZSTD_createCCtx();
  if (compression_ctx_ == nullptr) {
    ENVOY_CONN_LOG(error, "failed to create compression context, closing connection", read_callbacks_->connection());
    read_callbacks_->connection().close(Network::ConnectionCloseType::NoFlush);
    return Network::FilterStatus::StopIteration;
  }

  decompression_ctx_ = ZSTD_createDCtx();
  if (decompression_ctx_ == nullptr) {
    ENVOY_CONN_LOG(error, "failed to create decompression context, closing connection", read_callbacks_->connection());
    read_callbacks_->connection().close(Network::ConnectionCloseType::NoFlush);
    return Network::FilterStatus::StopIteration;
  }

  encoder_output_buf_.resize(ZSTD_CStreamOutSize());
  decoder_output_buf_.resize(ZSTD_DStreamOutSize());

  // We'll also initialize the decoder state to its proper values.
  decoder_zbuf_out_.dst = decoder_output_buf_.data();
  decoder_zbuf_out_.size = decoder_output_buf_.size(); 
  decoder_zbuf_out_.pos = 0;

  return Network::FilterStatus::Continue;
}

Network::FilterStatus StreamCompressorFilter::onWrite(Buffer::Instance& data, bool end_stream) {
  if (bypass_) {
    return Network::FilterStatus::Continue;
  }

  maybeIdentifyStreamType(data);
  if (stream_identification_ == StreamIdentification::ZSTD_ENCODE) {
    // We do the opposite here because this function is handling _response_
    // bytes. If we identified the stream as an encoder stream, meaning we
    // compress the data on the way to its upstream target, the responses will
    // be compressed and must be decoded.
    return decodeStream(data, end_stream);
  }

  return encodeStream(data, end_stream);
}

Network::FilterStatus StreamCompressorFilter::onData(Buffer::Instance& data, bool end_stream) {
  if (bypass_) {
    return Network::FilterStatus::Continue;
  }

  maybeIdentifyStreamType(data);
  if (stream_identification_ == StreamIdentification::ZSTD_ENCODE) {
    return encodeStream(data, end_stream);
  }

  return decodeStream(data, end_stream);
}

// ENCODE path.
Network::FilterStatus StreamCompressorFilter::encodeStream(Buffer::Instance& data, bool end_stream) {
  ZSTD_inBuffer zbuf_in{
    .src = nullptr, 
    .size = 0,
    .pos = 0,
  };
  ZSTD_outBuffer zbuf_out{
    .dst = encoder_output_buf_.data(),
    .size = encoder_output_buf_.size(),
    .pos = 0,
  };

  stats_.encoded_bytes_.add(data.length());

  // Iterate through the constituent slices of the data and feed them into the
  // compressor.
  const auto uncompressed_size = data.length();
  auto raw_slices = data.getRawSlices();
  for (size_t i = 0; i < raw_slices.size(); i++) {
    auto slice = raw_slices[i];
    zbuf_in.src = slice.mem_;
    zbuf_in.size = slice.len_; 
    zbuf_in.pos = 0;
    zbuf_out.dst = encoder_output_buf_.data();
    zbuf_out.size = encoder_output_buf_.size(); 
    zbuf_out.pos = 0;

    // Since there are no more slices, we want to flush out any of the buffers..
    // However, if the data stream is not concluded, we don't want to end
    // the zstd stream, so we want to use `ZSTD_e_flush` in that case.
    // Doing so will allow future data to reference previously compressed
    // data and improve the compression ratio, whereas `ZSTD_e_end` will
    // reset the context.
    ZSTD_EndDirective mode;
    const bool last_slice = (i == (raw_slices.size() - 1));
    mode = last_slice ? ZSTD_e_flush : ZSTD_e_continue;

    while (zbuf_in.pos < zbuf_in.size) {
      const size_t ret = doCompress(data, zbuf_in, zbuf_out, mode);
      if (ZSTD_isError(ret)) {
        // There's no coming back from an error like this. Kill the connection.
        ENVOY_CONN_LOG(error, "failed to compress data with error {}", read_callbacks_->connection(), ret);
        read_callbacks_->connection().close(Network::ConnectionCloseType::NoFlush);
        return Network::FilterStatus::StopIteration;
      }
    }
    ASSERT(zbuf_in.pos <= zbuf_in.size);
  }

  // If this is the end of the stream, we want to finalize the frame so that
  // the upstream knows there's no more data to be compressed. We do this by
  // passing `ZSTD_e_end` to the compressor.
  if (end_stream) {
    size_t ret;
    do {
      ret = doCompress(data, zbuf_in, zbuf_out, ZSTD_e_end);
    } while (ret != 0);
    if (ZSTD_isError(ret)) {
      // There's no coming back from an error like this. Kill the connection.
      ENVOY_CONN_LOG(error, "failed to compress data with error {}", read_callbacks_->connection(), ret);
      read_callbacks_->connection().close(Network::ConnectionCloseType::NoFlush);
      return Network::FilterStatus::StopIteration;
    }
  }

  // Discard the unneeded and uncompressed data, leaving only the compressed
  // bytes we want to write to the connection.
  data.drain(uncompressed_size);
  return Network::FilterStatus::Continue;
}

size_t StreamCompressorFilter::doCompress( 
    Buffer::Instance& data, ZSTD_inBuffer& zbuf_in, ZSTD_outBuffer& zbuf_out, ZSTD_EndDirective mode) {
  size_t ret = ZSTD_compressStream2(compression_ctx_, &zbuf_out, &zbuf_in, mode);

  // If we just completed a frame, we want to flush the compressed data to the
  // connection data buffer and reset the state of the output zbuf.
  const bool frame_complete = (ret == 0);
  if (frame_complete) {
    absl::string_view sv(static_cast<char*>(zbuf_out.dst), zbuf_out.pos);
    data.add(sv);
    zbuf_out.pos = 0;
  }

  return ret;
}

// DECODE path.
Network::FilterStatus StreamCompressorFilter::decodeStream(Buffer::Instance& data, bool end_stream) {
  ZSTD_inBuffer zbuf_in;

  stats_.decoded_bytes_.add(data.length());

  // Iterate through the constituent slices of the data and feed them into the
  // decompressor.
  const auto og_data_len = data.length();
  for (const auto& slice : data.getRawSlices()) {
    zbuf_in.src = slice.mem_;
    zbuf_in.size = slice.len_; 
    zbuf_in.pos = 0;

    // If input position is < input size, some input has not been consumed from the data buffer.
    while (zbuf_in.pos < zbuf_in.size) {
        doDecompress(data, zbuf_in);
        if (ZSTD_isError(decoder_state_)) {
          // There's no coming back from an error like this. Kill the connection.
          ENVOY_CONN_LOG(error, "failed to decompress data with error {}", read_callbacks_->connection(), decoder_state_);
          read_callbacks_->connection().close(Network::ConnectionCloseType::NoFlush);
          return Network::FilterStatus::StopIteration;
        }
    }
  }

  while (end_stream && decoder_state_ != 0) {
    doDecompress(data, zbuf_in);
    if (ZSTD_isError(decoder_state_)) {
      ENVOY_CONN_LOG(error, "failed to decompress data with error {}", read_callbacks_->connection(), decoder_state_);
      read_callbacks_->connection().close(Network::ConnectionCloseType::NoFlush);
      return Network::FilterStatus::StopIteration;
    }
  }

  // Clear out the old compressed data.
  data.drain(og_data_len);
  return Network::FilterStatus::Continue;
}

void StreamCompressorFilter::doDecompress(Buffer::Instance& data, ZSTD_inBuffer& zbuf_in) {
  decoder_state_ = ZSTD_decompressStream(decompression_ctx_, &decoder_zbuf_out_, &zbuf_in);
  maybeFlushData(data);
}

void StreamCompressorFilter::maybeFlushData(Buffer::Instance& data) {
  if (decoder_zbuf_out_.pos == 0) {
    // Nothing to flush.
    return;
  }

  stats_.decoder_stream_flush_.inc();

  // Append the output zbuf into the data buffer.
  const absl::string_view sv(static_cast<char*>(decoder_zbuf_out_.dst), decoder_zbuf_out_.pos);
  data.add(sv);

  // Reset the output buffer state if the frame was completed.
  if (decoder_state_ == 0) {
    decoder_zbuf_out_.dst = decoder_output_buf_.data();
    decoder_zbuf_out_.size = decoder_output_buf_.size(); 
    decoder_zbuf_out_.pos = 0;
  }
}

} // namespace Filter
} // namespace Envoy

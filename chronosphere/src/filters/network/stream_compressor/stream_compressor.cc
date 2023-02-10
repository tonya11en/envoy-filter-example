#include "stream_compressor.h"

#include "envoy/buffer/buffer.h"
#include "envoy/network/connection.h"

#include "source/common/common/assert.h"

#include "zstd.h"

namespace Envoy {
namespace Filter {

Network::FilterStatus StreamCompressorFilter::onNewConnection() {
  // TODO(tallen): 
  // Allow configurable stream parameters in filter config such as:
  //   * Compression level.
  //   * Dictionary.
  compression_ctx_ = ZSTD_createCCtx();
  if (compression_ctx_ == nullptr) {
    ENVOY_CONN_LOG(error, "failed to create compression context, bypassing compression", read_callbacks_->connection());
    bypass_ = true;
    return Network::FilterStatus::Continue;
  }

  input_buf_.resize(ZSTD_CStreamInSize());
  output_buf_.resize(ZSTD_CStreamOutSize());
  return Network::FilterStatus::Continue;
}

Network::FilterStatus StreamCompressorFilter::onData(Buffer::Instance& data, bool end_stream) {
  if (stream_identification_ == StreamIdentification::UNIDENTIFIED) {
      stream_identification_ = (hasMagicNumber(data) ? 
          StreamIdentification::ZSTD_DECODE : StreamIdentification::ZSTD_ENCODE;
  }

  if stream_identification_ == StreamIdentification::ZSTD_ENCODE) {
    return encodeStream(data, end_stream);
  }

  return decodeStream(data, end_stream);
}

Network::FilterStatus StreamCompressorFilter::encodeStream(Buffer::Instance& data, bool end_stream) {
  ZSTD_inBuffer zbuf_in;
  ZSTD_outBuffer zbuf_out;

  if (bypass_) {
    read_callbacks_->connection().write(data, end_stream);
    return Network::FilterStatus::Continue;
  }

  // Iterate through the constituent slices of the data and feed them into the
  // compressor.
  const auto uncompressed_size = data.length();
  auto raw_slices = data.getRawSlices();
  for (size_t i = 0; i < raw_slices.size(); i++) {
    auto slice = raw_slices[i];
    zbuf_in.src = slice.mem_;
    zbuf_in.size = slice.len_; 
    zbuf_in.pos = 0;
    zbuf_out.dst = output_buf_.data();
    zbuf_out.size = output_buf_.size(); 
    zbuf_out.pos = 0;

    ZSTD_EndDirective mode;
    const bool last_slice = (i == raw_slices.size() - 1);
    while (zbuf_in.pos != zbuf_in.size) {
      if (last_slice) {
        // Since there are no more slices, we want to conclude the block.
        // However, if the data stream is not concluded, we don't want to end
        // the zstd stream, so we want to use `ZSTD_e_flush` in that case.
        // Doing so will allow future data to reference previously compressed
        // data and improve the compression ratio, whereas `ZSTD_e_end` will
        // reset the context.
        mode = end_stream ? ZSTD_e_end : ZSTD_e_flush;
      } else {
        mode = ZSTD_e_continue;
      }

      auto remaining = ZSTD_compressStream2(compression_ctx_, &zbuf_out, &zbuf_in, mode);
      if (ZSTD_isError(remaining)) {
        // There's no coming back from an error like this. Kill the connection.
        ENVOY_CONN_LOG(error, "failed to compress data with error {}", read_callbacks_->connection(), remaining);
        read_callbacks_->connection().close(Network::ConnectionCloseType::NoFlush);
        return Network::FilterStatus::StopIteration;
      }
    }

    data.add(zbuf_out.dst, zbuf_out.pos);
  }

  // Discard the unneeded and uncompressed data, leaving only the compressed
  // bytes we want to write to the connection.
  data.drain(uncompressed_size);
  read_callbacks_->connection().write(data, end_stream);
  return Network::FilterStatus::Continue;
}

Network::FilterStatus StreamCompressorFilter::decodeStream(Buffer::Instance& , bool ) {
  return Network::FilterStatus::Continue;
}

} // namespace Filter
} // namespace Envoy

#include "stream_compressor.h"

#include "test/test_common/utility.h"
#include "source/common/stats/isolated_store_impl.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using testing::NiceMock;
using testing::Return;

namespace Envoy {
namespace Filter {
namespace {

class StreamCompressorTest : public testing::Test {
public:
  StreamCompressorTest() = default;

  void setupFilter() {
    filter_ = std::make_shared<StreamCompressorFilter>(scope_);
  }

protected:
  Stats::IsolatedStoreImpl store_;
  Stats::Scope& scope_ {*store_.rootScope()};
  std::shared_ptr<StreamCompressorFilter> filter_;
};

TEST_F(StreamCompressorTest, NoopTest) {
  setupFilter();

  absl::string_view data = "1234567890123456789023456789012345678909876543234567";
  Buffer::OwnedImpl buf(data);
  buf.add(data);
  buf.add(data);
  Buffer::OwnedImpl buf2(data);
  buf2.add(data);
  buf2.add(data);

  filter_->onNewConnection();

  filter_->onData(buf, false);
  filter_->onWrite(buf, false);
  filter_->onData(buf2, true);
  filter_->onWrite(buf2, true);

}

} // namespace
} // namespace Filter
} // namespace Envoy


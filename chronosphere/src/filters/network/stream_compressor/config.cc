#include <string>

#include "stream_compressor.h"

#include "envoy/registry/registry.h"
#include "envoy/server/filter_config.h"

namespace Envoy {
namespace Server {
namespace Configuration {

/**
 * Config registration for the stream_compressor filter. @see NamedNetworkFilterConfigFactory.
 */
class StreamCompressorConfigFactory : public NamedNetworkFilterConfigFactory {
public:
  Network::FilterFactoryCb createFilterFactoryFromProto(const Protobuf::Message&,
                                                        FactoryContext& ctx) override {
    return [&ctx](Network::FilterManager& filter_manager) -> void {
      filter_manager.addFilter(Network::FilterSharedPtr{
          new Filter::StreamCompressorFilter(ctx.scope())});
    };
  }

  ProtobufTypes::MessagePtr createEmptyConfigProto() override {
    return ProtobufTypes::MessagePtr{new Envoy::ProtobufWkt::Struct()};
  }

  std::string name() const override { return "chronosphere.stream_compressor"; }
};

/**
 * Static registration for the stream_compressor filter. @see RegisterFactory.
 */
static Registry::RegisterFactory<StreamCompressorConfigFactory, NamedNetworkFilterConfigFactory> registered_;

} // namespace Configuration
} // namespace Server
} // namespace Envoy

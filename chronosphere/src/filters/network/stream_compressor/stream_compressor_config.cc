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
                                                        FactoryContext&) override {
    return [](Network::FilterManager& filter_manager) -> void {
      filter_manager.addReadFilter(Network::ReadFilterSharedPtr{new Filter::StreamCompressorFilter()});
    };
  }

  ProtobufTypes::MessagePtr createEmptyConfigProto() override {
    return ProtobufTypes::MessagePtr{new Envoy::ProtobufWkt::Struct()};
  }

  std::string name() const override { return "stream_compressor"; }

  bool isTerminalFilterByProto(const Protobuf::Message&, ServerFactoryContext&) override { return true; }
};

/**
 * Static registration for the stream_compressor filter. @see RegisterFactory.
 */
static Registry::RegisterFactory<StreamCompressorConfigFactory, NamedNetworkFilterConfigFactory> registered_;

} // namespace Configuration
} // namespace Server
} // namespace Envoy

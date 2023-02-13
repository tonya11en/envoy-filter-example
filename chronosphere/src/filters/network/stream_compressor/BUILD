package(default_visibility = ["//visibility:public"])

load(
    "@envoy//bazel:envoy_build_system.bzl",
    "envoy_cc_library",
    "envoy_cc_test")

envoy_cc_library(
    name = "stream_compressor_lib",
    srcs = ["stream_compressor.cc"],
    hdrs = ["stream_compressor.h"],
    repository = "@envoy",
    deps = [
        "@envoy//envoy/stats:stats_macros",
        "@envoy//envoy/stats:stats_interface",
        "@envoy//envoy/network:connection_interface",
        "@envoy//envoy/network:filter_interface",
        "@envoy//source/common/common:assert_lib",
        "@envoy//source/common/common:logger_lib",
    ],
    external_deps = ["zstd"],
)

envoy_cc_library(
    name = "stream_compressor_config",
    srcs = ["config.cc"],
    repository = "@envoy",
    deps = [
        ":stream_compressor_lib",
        "@envoy//envoy/network:filter_interface",
        "@envoy//envoy/registry:registry",
        "@envoy//envoy/server:filter_config_interface",
    ],
)

# envoy_cc_test(
#     name = "stream_compressor_integration_test",
#     srcs = ["stream_compressor_integration_test.cc"],
#     data =  ["stream_compressor_server.yaml"],
#     repository = "@envoy",
#     deps = [
#         ":stream_compressor_config",
#         "@envoy//test/integration:integration_lib"
#     ],
# )

# See bazel/README.md for details on how this system works.
EXTENSIONS = {
    #
    # Access loggers
    #

    "envoy.access_loggers.file":                        "@envoy//source/extensions/access_loggers/file:config",
#    "envoy.access_loggers.extension_filters.cel":       "@envoy//source/extensions/access_loggers/filters/cel:config",
    "envoy.access_loggers.http_grpc":                   "@envoy//source/extensions/access_loggers/grpc:http_config",
    "envoy.access_loggers.tcp_grpc":                    "@envoy//source/extensions/access_loggers/grpc:tcp_config",
#    "envoy.access_loggers.open_telemetry":              "@envoy//source/extensions/access_loggers/open_telemetry:config",
    "envoy.access_loggers.stdout":                      "@envoy//source/extensions/access_loggers/stream:config",
    "envoy.access_loggers.stderr":                      "@envoy//source/extensions/access_loggers/stream:config",
#    "envoy.access_loggers.wasm":                        "@envoy//source/extensions/access_loggers/wasm:config",

    #
    # Clusters
    #

    "envoy.clusters.aggregate":                         "@envoy//source/extensions/clusters/aggregate:cluster",
    "envoy.clusters.dynamic_forward_proxy":             "@envoy//source/extensions/clusters/dynamic_forward_proxy:cluster",
    "envoy.clusters.eds":                               "@envoy//source/extensions/clusters/eds:eds_lib",
#    "envoy.clusters.redis":                             "@envoy//source/extensions/clusters/redis:redis_cluster",
    "envoy.clusters.static":                            "@envoy//source/extensions/clusters/static:static_cluster_lib",
    "envoy.clusters.strict_dns":                        "@envoy//source/extensions/clusters/strict_dns:strict_dns_cluster_lib",
    "envoy.clusters.original_dst":                      "@envoy//source/extensions/clusters/original_dst:original_dst_cluster_lib",
    "envoy.clusters.logical_dns":                       "@envoy//source/extensions/clusters/logical_dns:logical_dns_cluster_lib",

    #
    # Compression
    #

#    "envoy.compression.gzip.compressor":                "@envoy//source/extensions/compression/gzip/compressor:config",
#    "envoy.compression.gzip.decompressor":              "@envoy//source/extensions/compression/gzip/decompressor:config",
#    "envoy.compression.brotli.compressor":              "@envoy//source/extensions/compression/brotli/compressor:config",
#    "envoy.compression.brotli.decompressor":            "@envoy//source/extensions/compression/brotli/decompressor:config",
    "envoy.compression.zstd.compressor":                "@envoy//source/extensions/compression/zstd/compressor:config",
    "envoy.compression.zstd.decompressor":              "@envoy//source/extensions/compression/zstd/decompressor:config",

    #
    # Config validators
    #

    "envoy.config.validators.minimum_clusters_validator":     "@envoy//source/extensions/config/validators/minimum_clusters:config",

    #
    # gRPC Credentials Plugins
    #

#    "envoy.grpc_credentials.file_based_metadata":       "@envoy//source/extensions/grpc_credentials/file_based_metadata:config",
#    "envoy.grpc_credentials.aws_iam":                   "@envoy//source/extensions/grpc_credentials/aws_iam:config",

    #
    # WASM
    #

#    "envoy.bootstrap.wasm":                             "@envoy//source/extensions/bootstrap/wasm:config",

    #
    # Health checkers
    #

#    "envoy.health_checkers.redis":                      "@envoy//source/extensions/health_checkers/redis:config",
#    "envoy.health_checkers.thrift":                     "@envoy//source/extensions/health_checkers/thrift:config",

    #
    # Input Matchers
    #

    "envoy.matching.matchers.consistent_hashing":       "@envoy//source/extensions/matching/input_matchers/consistent_hashing:config",
    "envoy.matching.matchers.ip":                       "@envoy//source/extensions/matching/input_matchers/ip:config",

    #
    # Generic Inputs
    #

    "envoy.matching.common_inputs.environment_variable":       "@envoy//source/extensions/matching/common_inputs/environment_variable:config",

    #
    # Matching actions
    #

    "envoy.matching.actions.format_string":             "@envoy//source/extensions/matching/actions/format_string:config",

    #
    # HTTP filters
    #

    "envoy.filters.http.adaptive_concurrency":          "@envoy//source/extensions/filters/http/adaptive_concurrency:config",
    "envoy.filters.http.admission_control":             "@envoy//source/extensions/filters/http/admission_control:config",
#    "envoy.filters.http.alternate_protocols_cache":     "@envoy//source/extensions/filters/http/alternate_protocols_cache:config",
#    "envoy.filters.http.aws_lambda":                    "@envoy//source/extensions/filters/http/aws_lambda:config",
#    "envoy.filters.http.aws_request_signing":           "@envoy//source/extensions/filters/http/aws_request_signing:config",
#    "envoy.filters.http.bandwidth_limit":               "@envoy//source/extensions/filters/http/bandwidth_limit:config",
    "envoy.filters.http.buffer":                        "@envoy//source/extensions/filters/http/buffer:config",
#    "envoy.filters.http.cache":                         "@envoy//source/extensions/filters/http/cache:config",
#    "envoy.filters.http.cdn_loop":                      "@envoy//source/extensions/filters/http/cdn_loop:config",
    "envoy.filters.http.compressor":                    "@envoy//source/extensions/filters/http/compressor:config",
#    "envoy.filters.http.cors":                          "@envoy//source/extensions/filters/http/cors:config",
    "envoy.filters.http.composite":                     "@envoy//source/extensions/filters/http/composite:config",
#    "envoy.filters.http.csrf":                          "@envoy//source/extensions/filters/http/csrf:config",
    "envoy.filters.http.custom_response":               "@envoy//source/extensions/filters/http/custom_response:factory",
    "envoy.filters.http.decompressor":                  "@envoy//source/extensions/filters/http/decompressor:config",
#    "envoy.filters.http.dynamic_forward_proxy":         "@envoy//source/extensions/filters/http/dynamic_forward_proxy:config",
#    "envoy.filters.http.ext_authz":                     "@envoy//source/extensions/filters/http/ext_authz:config",
#    "envoy.filters.http.ext_proc":                      "@envoy//source/extensions/filters/http/ext_proc:config",
    "envoy.filters.http.fault":                         "@envoy//source/extensions/filters/http/fault:config",
    "envoy.filters.http.file_system_buffer":            "@envoy//source/extensions/filters/http/file_system_buffer:config",
#    "envoy.filters.http.gcp_authn":                     "@envoy//source/extensions/filters/http/gcp_authn:config",
#    "envoy.filters.http.grpc_http1_bridge":             "@envoy//source/extensions/filters/http/grpc_http1_bridge:config",
#    "envoy.filters.http.grpc_http1_reverse_bridge":     "@envoy//source/extensions/filters/http/grpc_http1_reverse_bridge:config",
#    "envoy.filters.http.grpc_json_transcoder":          "@envoy//source/extensions/filters/http/grpc_json_transcoder:config",
    "envoy.filters.http.grpc_stats":                    "@envoy//source/extensions/filters/http/grpc_stats:config",
#    "envoy.filters.http.grpc_web":                      "@envoy//source/extensions/filters/http/grpc_web:config",
    "envoy.filters.http.header_to_metadata":            "@envoy//source/extensions/filters/http/header_to_metadata:config",
    "envoy.filters.http.health_check":                  "@envoy//source/extensions/filters/http/health_check:config",
    "envoy.filters.http.ip_tagging":                    "@envoy//source/extensions/filters/http/ip_tagging:config",
#    "envoy.filters.http.jwt_authn":                     "@envoy//source/extensions/filters/http/jwt_authn:config",
#    "envoy.filters.http.rate_limit_quota":              "@envoy//source/extensions/filters/http/rate_limit_quota:config",
    # Disabled by default
#    "envoy.filters.http.kill_request":                  "@envoy//source/extensions/filters/http/kill_request:kill_request_config",
#    "envoy.filters.http.local_ratelimit":               "@envoy//source/extensions/filters/http/local_ratelimit:config",
#    "envoy.filters.http.lua":                           "@envoy//source/extensions/filters/http/lua:config",
#    "envoy.filters.http.oauth2":                        "@envoy//source/extensions/filters/http/oauth2:config",
#    "envoy.filters.http.on_demand":                     "@envoy//source/extensions/filters/http/on_demand:config",
    "envoy.filters.http.original_src":                  "@envoy//source/extensions/filters/http/original_src:config",
#    "envoy.filters.http.ratelimit":                     "@envoy//source/extensions/filters/http/ratelimit:config",
#    "envoy.filters.http.rbac":                          "@envoy//source/extensions/filters/http/rbac:config",
    "envoy.filters.http.router":                        "@envoy//source/extensions/filters/http/router:config",
    "envoy.filters.http.set_metadata":                  "@envoy//source/extensions/filters/http/set_metadata:config",
#    "envoy.filters.http.tap":                           "@envoy//source/extensions/filters/http/tap:config",
#    "envoy.filters.http.wasm":                          "@envoy//source/extensions/filters/http/wasm:config",
#    "envoy.filters.http.stateful_session":              "@envoy//source/extensions/filters/http/stateful_session:config",

    #
    # Listener filters
    #

    "envoy.filters.listener.http_inspector":            "@envoy//source/extensions/filters/listener/http_inspector:config",
    # NOTE: The original_dst filter is implicitly loaded if original_dst functionality is
    #       configured on the listener. Do not remove it in that case or configs will fail to load.
    "envoy.filters.listener.original_dst":              "@envoy//source/extensions/filters/listener/original_dst:config",
    "envoy.filters.listener.original_src":              "@envoy//source/extensions/filters/listener/original_src:config",
    # NOTE: The proxy_protocol filter is implicitly loaded if proxy_protocol functionality is
    #       configured on the listener. Do not remove it in that case or configs will fail to load.
    "envoy.filters.listener.proxy_protocol":            "@envoy//source/extensions/filters/listener/proxy_protocol:config",
    "envoy.filters.listener.tls_inspector":             "@envoy//source/extensions/filters/listener/tls_inspector:config",

    #
    # Network filters
    #

    "envoy.filters.network.connection_limit":                     "@envoy//source/extensions/filters/network/connection_limit:config",
    "envoy.filters.network.direct_response":                      "@envoy//source/extensions/filters/network/direct_response:config",
#    "envoy.filters.network.dubbo_proxy":                          "@envoy//source/extensions/filters/network/dubbo_proxy:config",
    "envoy.filters.network.echo":                                 "@envoy//source/extensions/filters/network/echo:config",
#    "envoy.filters.network.ext_authz":                            "@envoy//source/extensions/filters/network/ext_authz:config",
    "envoy.filters.network.http_connection_manager":              "@envoy//source/extensions/filters/network/http_connection_manager:config",
    "envoy.filters.network.local_ratelimit":                      "@envoy//source/extensions/filters/network/local_ratelimit:config",
#    "envoy.filters.network.mongo_proxy":                          "@envoy//source/extensions/filters/network/mongo_proxy:config",
    "envoy.filters.network.ratelimit":                            "@envoy//source/extensions/filters/network/ratelimit:config",
    "envoy.filters.network.rbac":                                 "@envoy//source/extensions/filters/network/rbac:config",
#    "envoy.filters.network.redis_proxy":                          "@envoy//source/extensions/filters/network/redis_proxy:config",
    "envoy.filters.network.tcp_proxy":                            "@envoy//source/extensions/filters/network/tcp_proxy:config",
#    "envoy.filters.network.thrift_proxy":                         "@envoy//source/extensions/filters/network/thrift_proxy:config",
#    "envoy.filters.network.sni_cluster":                          "@envoy//source/extensions/filters/network/sni_cluster:config",
#    "envoy.filters.network.sni_dynamic_forward_proxy":            "@envoy//source/extensions/filters/network/sni_dynamic_forward_proxy:config",
#    "envoy.filters.network.wasm":                                 "@envoy//source/extensions/filters/network/wasm:config",
#    "envoy.filters.network.zookeeper_proxy":                      "@envoy//source/extensions/filters/network/zookeeper_proxy:config",

    #
    # UDP filters
    #

#    "envoy.filters.udp.dns_filter":                     "@envoy//source/extensions/filters/udp/dns_filter:config",
#    "envoy.filters.udp_listener.udp_proxy":             "@envoy//source/extensions/filters/udp/udp_proxy:config",

    #
    # Resource monitors
    #

    "envoy.resource_monitors.fixed_heap":               "@envoy//source/extensions/resource_monitors/fixed_heap:config",
    "envoy.resource_monitors.injected_resource":        "@envoy//source/extensions/resource_monitors/injected_resource:config",
    "envoy.resource_monitors.downstream_connections":   "@envoy//source/extensions/resource_monitors/downstream_connections:config",

    #
    # Stat sinks
    #

#    "envoy.stat_sinks.dog_statsd":                      "@envoy//source/extensions/stat_sinks/dog_statsd:config",
#    "envoy.stat_sinks.graphite_statsd":                 "@envoy//source/extensions/stat_sinks/graphite_statsd:config",
#    "envoy.stat_sinks.hystrix":                         "@envoy//source/extensions/stat_sinks/hystrix:config",
    "envoy.stat_sinks.metrics_service":                 "@envoy//source/extensions/stat_sinks/metrics_service:config",
    "envoy.stat_sinks.statsd":                          "@envoy//source/extensions/stat_sinks/statsd:config",
#    "envoy.stat_sinks.wasm":                            "@envoy//source/extensions/stat_sinks/wasm:config",

    #
    # Thrift filters
    #

#    "envoy.filters.thrift.router":                      "@envoy//source/extensions/filters/network/thrift_proxy/router:config",
#    "envoy.filters.thrift.header_to_metadata":          "@envoy//source/extensions/filters/network/thrift_proxy/filters/header_to_metadata:config",
#    "envoy.filters.thrift.payload_to_metadata":         "@envoy//source/extensions/filters/network/thrift_proxy/filters/payload_to_metadata:config",
#    "envoy.filters.thrift.rate_limit":                  "@envoy//source/extensions/filters/network/thrift_proxy/filters/ratelimit:config",

    #
    # Tracers
    #

#    "envoy.tracers.dynamic_ot":                         "@envoy//source/extensions/tracers/dynamic_ot:config",
#    "envoy.tracers.datadog":                            "@envoy//source/extensions/tracers/datadog:config",
#   "envoy.tracers.zipkin":                             "@envoy//source/extensions/tracers/zipkin:config",
#   "envoy.tracers.opencensus":                         "@envoy//source/extensions/tracers/opencensus:config",
#   "envoy.tracers.xray":                               "@envoy//source/extensions/tracers/xray:config",
#   "envoy.tracers.skywalking":                         "@envoy//source/extensions/tracers/skywalking:config",
#   "envoy.tracers.opentelemetry":                      "@envoy//source/extensions/tracers/opentelemetry:config",

    #
    # Transport sockets
    #

#    "envoy.transport_sockets.alts":                     "@envoy//source/extensions/transport_sockets/alts:config",
#    "envoy.transport_sockets.http_11_proxy":            "@envoy//source/extensions/transport_sockets/http_11_proxy:upstream_config",
    "envoy.transport_sockets.upstream_proxy_protocol":  "@envoy//source/extensions/transport_sockets/proxy_protocol:upstream_config",
    "envoy.transport_sockets.raw_buffer":               "@envoy//source/extensions/transport_sockets/raw_buffer:config",
#    "envoy.transport_sockets.tap":                      "@envoy//source/extensions/transport_sockets/tap:config",
#    "envoy.transport_sockets.starttls":                 "@envoy//source/extensions/transport_sockets/starttls:config",
    "envoy.transport_sockets.tcp_stats":                "@envoy//source/extensions/transport_sockets/tcp_stats:config",
    "envoy.transport_sockets.internal_upstream":        "@envoy//source/extensions/transport_sockets/internal_upstream:config",

    #
    # Retry host predicates
    #

    "envoy.retry_host_predicates.previous_hosts":       "@envoy//source/extensions/retry/host/previous_hosts:config",
    "envoy.retry_host_predicates.omit_canary_hosts":    "@envoy//source/extensions/retry/host/omit_canary_hosts:config",
    "envoy.retry_host_predicates.omit_host_metadata":   "@envoy//source/extensions/retry/host/omit_host_metadata:config",

    #
    # Retry priorities
    #

    "envoy.retry_priorities.previous_priorities":       "@envoy//source/extensions/retry/priority/previous_priorities:config",

    #
    # CacheFilter plugins
    #
#    "envoy.extensions.http.cache.file_system_http_cache": "@envoy//source/extensions/http/cache/file_system_http_cache:config",
#    "envoy.extensions.http.cache.simple":               "@envoy//source/extensions/http/cache/simple_http_cache:config",

    #
    # Internal redirect predicates
    #

    "envoy.internal_redirect_predicates.allow_listed_routes": "@envoy//source/extensions/internal_redirect/allow_listed_routes:config",
    "envoy.internal_redirect_predicates.previous_routes":     "@envoy//source/extensions/internal_redirect/previous_routes:config",
    "envoy.internal_redirect_predicates.safe_cross_scheme":   "@envoy//source/extensions/internal_redirect/safe_cross_scheme:config",

    #
    # Http Upstreams (excepting envoy.upstreams.http.generic which is hard-coded into the build so not registered here)
    #

    "envoy.upstreams.http.http":                        "@envoy//source/extensions/upstreams/http/http:config",
    "envoy.upstreams.http.tcp":                         "@envoy//source/extensions/upstreams/http/tcp:config",

    #
    # Watchdog actions
    #

    "envoy.watchdog.profile_action":                    "@envoy//source/extensions/watchdog/profile_action:config",

    #
    # WebAssembly runtimes
    #

#    "envoy.wasm.runtime.null":                          "@envoy//source/extensions/wasm_runtime/null:config",
#    "envoy.wasm.runtime.v8":                            "@envoy//source/extensions/wasm_runtime/v8:config",
#    "envoy.wasm.runtime.wamr":                          "@envoy//source/extensions/wasm_runtime/wamr:config",
#    "envoy.wasm.runtime.wavm":                          "@envoy//source/extensions/wasm_runtime/wavm:config",
#    "envoy.wasm.runtime.wasmtime":                      "@envoy//source/extensions/wasm_runtime/wasmtime:config",

    #
    # Rate limit descriptors
    #

#    "envoy.rate_limit_descriptors.expr":                "@envoy//source/extensions/rate_limit_descriptors/expr:config",

    #
    # IO socket
    #

    "envoy.io_socket.user_space":                       "@envoy//source/extensions/io_socket/user_space:config",
    "envoy.bootstrap.internal_listener":                "@envoy//source/extensions/bootstrap/internal_listener:config",

    #
    # TLS peer certification validators
    #

#    "envoy.tls.cert_validator.spiffe":                  "@envoy//source/extensions/transport_sockets/tls/cert_validator/spiffe:config",

    #
    # HTTP header formatters
    #

    "envoy.http.stateful_header_formatters.preserve_case":       "@envoy//source/extensions/http/header_formatters/preserve_case:config",

    #
    # Original IP detection
    #

    "envoy.http.original_ip_detection.custom_header":        "@envoy//source/extensions/http/original_ip_detection/custom_header:config",
    "envoy.http.original_ip_detection.xff":                  "@envoy//source/extensions/http/original_ip_detection/xff:config",

    #
    # Stateful session
    #

#    "envoy.http.stateful_session.cookie":                "@envoy//source/extensions/http/stateful_session/cookie:config",
#    "envoy.http.stateful_session.header":                "@envoy//source/extensions/http/stateful_session/header:config",

    #
    # Custom response policies
    #

    "envoy.http.custom_response.redirect_policy":             "@envoy//source/extensions/http/custom_response/redirect_policy:redirect_policy_lib",
    "envoy.http.custom_response.local_response_policy":       "@envoy//source/extensions/http/custom_response/local_response_policy:local_response_policy_lib",

    #
    # QUIC extensions
    #

#    "envoy.quic.deterministic_connection_id_generator": "@envoy//source/extensions/quic/connection_id_generator:envoy_deterministic_connection_id_generator_config",
#    "envoy.quic.crypto_stream.server.quiche":           "@envoy//source/extensions/quic/crypto_stream:envoy_quic_default_crypto_server_stream",
#    "envoy.quic.proof_source.filter_chain":             "@envoy//source/extensions/quic/proof_source:envoy_quic_default_proof_source",

    #
    # UDP packet writers
    #
#    "envoy.udp_packet_writer.default":                  "@envoy//source/extensions/udp_packet_writer/default:config",
#    "envoy.udp_packet_writer.gso":                      "@envoy//source/extensions/udp_packet_writer/gso:config",

    #
    # Formatter
    #

    "envoy.formatter.metadata":                         "@envoy//source/extensions/formatter/metadata:config",
    "envoy.formatter.req_without_query":                "@envoy//source/extensions/formatter/req_without_query:config",

    #
    # Key value store
    #

#    "envoy.key_value.file_based":     "@envoy//source/extensions/key_value/file_based:config_lib",

    #
    # RBAC matchers
    #

    "envoy.rbac.matchers.upstream_ip_port":     "@envoy//source/extensions/filters/common/rbac/matchers:upstream_ip_port_lib",

    #
    # DNS Resolver
    #

    # c-ares DNS resolver extension is recommended to be enabled to maintain the legacy DNS resolving behavior.
    "envoy.network.dns_resolver.cares":                "@envoy//source/extensions/network/dns_resolver/cares:config",
    # apple DNS resolver extension is only needed in MacOS build plus one want to use apple library for DNS resolving.
    "envoy.network.dns_resolver.apple":                "@envoy//source/extensions/network/dns_resolver/apple:config",
    # getaddrinfo DNS resolver extension can be used when the system resolver is desired (e.g., Android)
    "envoy.network.dns_resolver.getaddrinfo":          "@envoy//source/extensions/network/dns_resolver/getaddrinfo:config",

    #
    # Custom matchers
    #

    "envoy.matching.custom_matchers.trie_matcher":     "@envoy//source/extensions/common/matcher:trie_matcher_lib",

    #
    # Header Validators
    #

    "envoy.http.header_validators.envoy_default":        "@envoy//source/extensions/http/header_validators/envoy_default:config",

    #
    # Path Pattern Match and Path Pattern Rewrite
    #
    "envoy.path.match.uri_template.uri_template_matcher": "@envoy//source/extensions/path/match/uri_template:config",
    "envoy.path.rewrite.uri_template.uri_template_rewriter": "@envoy//source/extensions/path/rewrite/uri_template:config",
    #
    # Early Data option
    #

    "envoy.route.early_data_policy.default":           "@envoy//source/extensions/early_data:default_early_data_policy_lib",

    #
    # Load balancing policies for upstream
    #
    "envoy.load_balancing_policies.least_request":     "@envoy//source/extensions/load_balancing_policies/least_request:config",
    "envoy.load_balancing_policies.random":            "@envoy//source/extensions/load_balancing_policies/random:config",
    "envoy.load_balancing_policies.round_robin":       "@envoy//source/extensions/load_balancing_policies/round_robin:config",
    "envoy.load_balancing_policies.maglev":            "@envoy//source/extensions/load_balancing_policies/maglev:config",
    "envoy.load_balancing_policies.ring_hash":       "@envoy//source/extensions/load_balancing_policies/ring_hash:config",

    # HTTP Early Header Mutation
    #
    "envoy.http.early_header_mutation.header_mutation": "@envoy//source/extensions/http/early_header_mutation/header_mutation:config",
}

# These can be changed to ["@envoy//visibility:public"], for  downstream builds which
# need to directly reference Envoy extensions.
EXTENSION_CONFIG_VISIBILITY = ["@envoy//:extension_config", "@envoy//:contrib_library", "@envoy//:examples_library", "@envoy//:mobile_library"]
EXTENSION_PACKAGE_VISIBILITY = ["@envoy//:extension_library", "@envoy//:contrib_library", "@envoy//:examples_library", "@envoy//:mobile_library"]
CONTRIB_EXTENSION_PACKAGE_VISIBILITY = ["@envoy//:contrib_library"]
MOBILE_PACKAGE_VISIBILITY = ["@envoy//:mobile_library"]

# Set this variable to true to disable alwayslink for envoy_cc_library.
# TODO(alyssawilk) audit uses of this in source/ and migrate all libraries to extensions.
LEGACY_ALWAYSLINK = 1

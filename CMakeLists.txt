cmake_minimum_required(VERSION 3.10)
project(StreamingNetworkFramework)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

# Core module
add_library(core
    src/core/stream_manager.cpp
    src/core/stream_session.cpp
    src/core/logger.cpp
)

# Plugins system
add_library(plugins
    src/plugins/rate_control/rate_control_interface.cpp
    src/plugins/codec/codec_interface.cpp
)

# Network layer
add_library(network
    src/network/network_transport.cpp
    src/network/packet_manager.cpp
)

# Example AI rate control algorithm
add_library(example_ai_rate_control
    src/plugins/rate_control/example_ai_rate_control.cpp
)

# Example custom codec
add_library(example_custom_codec
    src/plugins/codec/example_custom_codec.cpp
)

# Test executable
add_executable(test_streaming
    examples/test/test_streaming.cpp
)

target_link_libraries(test_streaming
    core
    plugins
    network
    example_ai_rate_control
    example_custom_codec
)

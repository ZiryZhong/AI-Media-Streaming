cmake_minimum_required(VERSION 3.10)
project(StreamingNetworkFramework)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

# Find FFmpeg packages
find_package(PkgConfig REQUIRED)
pkg_check_modules(AVCODEC REQUIRED libavcodec)
pkg_check_modules(AVFORMAT REQUIRED libavformat)
pkg_check_modules(AVUTIL REQUIRED libavutil)

include_directories(${AVCODEC_INCLUDE_DIRS} ${AVFORMAT_INCLUDE_DIRS} ${AVUTIL_INCLUDE_DIRS})
link_directories(${AVCODEC_LIBRARY_DIRS} ${AVFORMAT_LIBRARY_DIRS} ${AVUTIL_LIBRARY_DIRS})

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
    src/network/rtp_transport.cpp
)

# Example AI rate control algorithm
add_library(example_ai_rate_control
    src/plugins/rate_control/example_ai_rate_control.cpp
)

# Example custom codec
add_library(example_custom_codec
    src/plugins/codec/example_custom_codec.cpp
)

# FFmpeg codec
add_library(ffmpeg_codec
    src/plugins/codec/ffmpeg_codec.cpp
)

target_link_libraries(ffmpeg_codec
    ${AVCODEC_LIBRARIES}
    ${AVFORMAT_LIBRARIES}
    ${AVUTIL_LIBRARIES}
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

# UDP server executable
add_executable(udp_server
    examples/test/udp_server.cpp
)

# FFmpeg test executable
add_executable(test_ffmpeg_streaming
    examples/test/test_ffmpeg_streaming.cpp
)

target_link_libraries(test_ffmpeg_streaming
    core
    plugins
    network
    example_ai_rate_control
    ffmpeg_codec
)

# Performance test executable
add_executable(performance_test
    examples/test/performance_test.cpp
)

target_link_libraries(performance_test
    core
    plugins
    network
    example_ai_rate_control
    ffmpeg_codec
)

# RTP streaming test executable
add_executable(test_rtp_streaming
    examples/test/test_rtp_streaming.cpp
)

target_link_libraries(test_rtp_streaming
    core
    plugins
    network
)

# RTP YUV sender executable
add_executable(rtp_yuv_sender
    examples/test/rtp_yuv_sender.cpp
)

target_link_libraries(rtp_yuv_sender
    core
    plugins
    network
)

# RTP YUV receiver executable
add_executable(rtp_yuv_receiver
    examples/test/rtp_yuv_receiver.cpp
)

target_link_libraries(rtp_yuv_receiver
    core
    plugins
    network
)

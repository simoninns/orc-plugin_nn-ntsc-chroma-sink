cmake_minimum_required(VERSION 3.20)

project(nn-ntsc-chroma-sink
    VERSION 1.0.0
    DESCRIPTION "NN NTSC Chroma Sink — neural-network NTSC chroma decoder plugin for Decode-Orc"
    LANGUAGES CXX
)

set(ORC_PLUGIN_VERSION "${PROJECT_VERSION}" CACHE STRING
    "Plugin version string embedded in StagePluginDescriptor")

# On tagged CI builds, derive plugin version from tag names like v1.2.3
# unless the caller explicitly overrides ORC_PLUGIN_VERSION.
if(ORC_PLUGIN_VERSION STREQUAL PROJECT_VERSION)
    if(DEFINED ENV{GITHUB_REF_TYPE} AND "$ENV{GITHUB_REF_TYPE}" STREQUAL "tag" AND DEFINED ENV{GITHUB_REF_NAME})
        set(_orc_ref_name "$ENV{GITHUB_REF_NAME}")
        if(_orc_ref_name MATCHES "^v(.+)$")
            set(ORC_PLUGIN_VERSION "${CMAKE_MATCH_1}")
            set(ORC_PLUGIN_VERSION "${ORC_PLUGIN_VERSION}" CACHE STRING
                "Plugin version string embedded in StagePluginDescriptor" FORCE)
        endif()
    endif()
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_TESTS "Build unit tests for the nn-ntsc-chroma-sink plugin" ON)

if(DEFINED ORC_INTREE_SDK_DIR)
    include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/DecodeOrcPluginSDKHelpers.cmake")

    add_library(orc-sdk-includes INTERFACE)
    target_include_directories(orc-sdk-includes INTERFACE
        ${ORC_INTREE_SDK_DIR}/orc/common/include
        ${ORC_INTREE_SDK_DIR}/orc/core/include
        ${ORC_INTREE_SDK_DIR}/orc/core/stages
        ${ORC_INTREE_SDK_DIR}/orc/view-types
    )

    # Try to find fmt via CMake config (works with vcpkg toolchain on Windows).
    # Fall back to a Nix store glob for Linux/macOS Nix dev shells.
    find_package(fmt CONFIG QUIET)
    if(fmt_FOUND)
        target_link_libraries(orc-sdk-includes INTERFACE fmt::fmt-header-only)
    else()
        file(GLOB _orc_fmt_include_candidates "/nix/store/*-fmt-*/include")
        list(LENGTH _orc_fmt_include_candidates _orc_fmt_include_count)
        if(_orc_fmt_include_count GREATER 0)
            list(GET _orc_fmt_include_candidates 0 _orc_fmt_include_dir)
            target_include_directories(orc-sdk-includes INTERFACE ${_orc_fmt_include_dir})
        endif()
    endif()

    add_library(orc-plugin-sdk INTERFACE)
    target_include_directories(orc-plugin-sdk INTERFACE
        ${ORC_INTREE_SDK_DIR}/orc/sdk/include
    )
    target_link_libraries(orc-plugin-sdk INTERFACE orc-sdk-includes)
else()
    include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/DecodeOrcPluginSDKHelpers.cmake")
    find_package(decode-orc-plugin-sdk REQUIRED)
endif()

# ---------------------------------------------------------------------------
# Embed chroma_net_v2.onnx as a plain C byte array for in-memory ONNX session creation.
# ---------------------------------------------------------------------------

set(_onnx_model "${CMAKE_CURRENT_SOURCE_DIR}/resources/chroma_net_v2.onnx")
file(READ "${_onnx_model}" _onnx_hex HEX)
string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," _onnx_arr "${_onnx_hex}")
string(LENGTH "${_onnx_hex}" _onnx_hex_len)
math(EXPR _onnx_size "${_onnx_hex_len} / 2")

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/chroma_net_v2_onnx_data.h"
    "#pragma once\n"
    "#include <cstddef>\n"
    "extern const unsigned char kChromaNetV2OnnxData[];\n"
    "extern const std::size_t   kChromaNetV2OnnxDataSize;\n")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/chroma_net_v2_onnx_data.cpp"
    "#include \"chroma_net_v2_onnx_data.h\"\n"
    "const unsigned char kChromaNetV2OnnxData[] = {${_onnx_arr}};\n"
    "const std::size_t   kChromaNetV2OnnxDataSize = ${_onnx_size};\n")

set(NN_NTSC_PLUGIN_TARGET "orc-stage-plugin-nn-ntsc-chroma-sink")

set(NN_NTSC_PLUGIN_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/plugin.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/nn_ntsc_chroma_sink_stage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/nn_output_backend.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/vfr_to_source_field_adapter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/decoders/componentframe.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/decoders/decoder.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/decoders/comb.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/decoders/ntscdecoder.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/decoders/nn_output_writer.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/chroma_net_v2_onnx_data.cpp
)

if(NOT MSVC)
    set_source_files_properties(
        ${CMAKE_CURRENT_SOURCE_DIR}/decoders/componentframe.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/decoders/decoder.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/decoders/comb.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/decoders/ntscdecoder.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/decoders/nn_output_writer.cpp
        PROPERTIES COMPILE_FLAGS "-w -O2"
    )
endif()

orc_add_stage_plugin(
    ${NN_NTSC_PLUGIN_TARGET}
    OUTPUT_NAME ${NN_NTSC_PLUGIN_TARGET}
    PLUGIN_VERSION "${ORC_PLUGIN_VERSION}"
    SOURCES ${NN_NTSC_PLUGIN_SOURCES}
)

target_include_directories(${NN_NTSC_PLUGIN_TARGET} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/decoders
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_compile_definitions(${NN_NTSC_PLUGIN_TARGET} PRIVATE
    _USE_MATH_DEFINES
)

# ---------------------------------------------------------------------------
# Link ONNX Runtime
# ---------------------------------------------------------------------------

if(TARGET onnxruntime::onnxruntime)
    target_link_libraries(${NN_NTSC_PLUGIN_TARGET} PRIVATE onnxruntime::onnxruntime)
    if(APPLE)
        set_property(TARGET ${NN_NTSC_PLUGIN_TARGET} APPEND PROPERTY
            BUILD_RPATH "$<TARGET_FILE_DIR:onnxruntime::onnxruntime>")
    endif()
elseif(TARGET onnxruntime)
    target_link_libraries(${NN_NTSC_PLUGIN_TARGET} PRIVATE onnxruntime)
    if(APPLE)
        set_property(TARGET ${NN_NTSC_PLUGIN_TARGET} APPEND PROPERTY
            BUILD_RPATH "$<TARGET_FILE_DIR:onnxruntime>")
    endif()
else()
    find_library(ONNXRUNTIME_LIB NAMES onnxruntime)
    if(ONNXRUNTIME_LIB)
        target_link_libraries(${NN_NTSC_PLUGIN_TARGET} PRIVATE ${ONNXRUNTIME_LIB})

        # When ONNXRUNTIME_LIB is passed directly (for example on Windows CI),
        # resolve and add the corresponding ONNX Runtime include directory.
        set(_onnxruntime_include_hints)
        if(DEFINED ONNXRUNTIME_INCLUDE_DIR)
            list(APPEND _onnxruntime_include_hints "${ONNXRUNTIME_INCLUDE_DIR}")
        endif()

        get_filename_component(_onnxruntime_libdir "${ONNXRUNTIME_LIB}" DIRECTORY)
        get_filename_component(_onnxruntime_root "${_onnxruntime_libdir}" DIRECTORY)
        list(APPEND _onnxruntime_include_hints
            "${_onnxruntime_root}/include"
        )

        find_path(ONNXRUNTIME_INCLUDE_DIR
            NAMES onnxruntime_cxx_api.h
            HINTS ${_onnxruntime_include_hints}
        )

        if(ONNXRUNTIME_INCLUDE_DIR)
            target_include_directories(${NN_NTSC_PLUGIN_TARGET} PRIVATE
                ${ONNXRUNTIME_INCLUDE_DIR}
            )
        else()
            message(FATAL_ERROR
                "ONNX Runtime headers not found for ONNXRUNTIME_LIB=${ONNXRUNTIME_LIB}. "
                "Pass -DONNXRUNTIME_INCLUDE_DIR=<path-to-onnxruntime-include> if needed.")
        endif()

        if(APPLE)
            get_filename_component(_onnxruntime_dir "${ONNXRUNTIME_LIB}" DIRECTORY)
            set_property(TARGET ${NN_NTSC_PLUGIN_TARGET} APPEND PROPERTY
                BUILD_RPATH "${_onnxruntime_dir}")
        endif()
    else()
        message(FATAL_ERROR
            "ONNX Runtime not found. Install onnxruntime (>= 1.23.2) and ensure it is \n"
            "discoverable via find_library, a CMake config package, or a pkg-config file.")
    endif()
endif()

# ---------------------------------------------------------------------------
# Link FFTW3
# ---------------------------------------------------------------------------

if(TARGET PkgConfig::FFTW3)
    target_link_libraries(${NN_NTSC_PLUGIN_TARGET} PRIVATE PkgConfig::FFTW3)
elseif(TARGET PkgConfig::FFTW3_CORE)
    target_link_libraries(${NN_NTSC_PLUGIN_TARGET} PRIVATE PkgConfig::FFTW3_CORE)
elseif(TARGET FFTW3::fftw3)
    target_link_libraries(${NN_NTSC_PLUGIN_TARGET} PRIVATE FFTW3::fftw3)
else()
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(FFTW3_NN IMPORTED_TARGET fftw3)
        if(FFTW3_NN_FOUND)
            target_link_libraries(${NN_NTSC_PLUGIN_TARGET} PRIVATE PkgConfig::FFTW3_NN)
        endif()
    endif()
    if(NOT TARGET PkgConfig::FFTW3_NN)
        find_package(FFTW3 CONFIG QUIET)
        if(TARGET FFTW3::fftw3)
            target_link_libraries(${NN_NTSC_PLUGIN_TARGET} PRIVATE FFTW3::fftw3)
        endif()
    endif()
endif()

# ---------------------------------------------------------------------------
# Optional unit tests
# ---------------------------------------------------------------------------

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

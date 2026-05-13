cmake_minimum_required(VERSION 3.20)

if(DEFINED ORC_INTREE_SDK_DIR AND TARGET orc-plugin-sdk)
    set(NN_NTSC_TEST_SDK_TARGET orc-plugin-sdk)
elseif(TARGET orc::plugin-sdk)
    set(NN_NTSC_TEST_SDK_TARGET orc::plugin-sdk)
elseif(TARGET orc-plugin-sdk)
    set(NN_NTSC_TEST_SDK_TARGET orc-plugin-sdk)
else()
    message(FATAL_ERROR "Neither orc::plugin-sdk nor orc-plugin-sdk is available for tests")
endif()

add_executable(nn_ntsc_stage_smoke_test
    stage_smoke_test.cpp
)

target_link_libraries(nn_ntsc_stage_smoke_test PRIVATE
    ${NN_NTSC_TEST_SDK_TARGET}
)

target_include_directories(nn_ntsc_stage_smoke_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
)

add_test(NAME nn_ntsc_stage_smoke_test COMMAND nn_ntsc_stage_smoke_test)

add_executable(nn_ntsc_plugin_entrypoints_test
    plugin_entrypoints_test.cpp
)

target_link_libraries(nn_ntsc_plugin_entrypoints_test PRIVATE
    orc-stage-plugin-nn-ntsc-chroma-sink
    ${NN_NTSC_TEST_SDK_TARGET}
)

target_include_directories(nn_ntsc_plugin_entrypoints_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
)

add_test(NAME nn_ntsc_plugin_entrypoints_test COMMAND nn_ntsc_plugin_entrypoints_test)

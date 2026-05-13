include_guard(GLOBAL)
include(CMakeParseArguments)

if(NOT DEFINED ORC_STAGE_PLUGIN_INSTALL_DIR)
    if(APPLE)
        set(ORC_STAGE_PLUGIN_INSTALL_DIR "orc-gui.app/Contents/PlugIns/orc-stage-plugins")
    elseif(WIN32)
        set(ORC_STAGE_PLUGIN_INSTALL_DIR "bin/orc-stage-plugins")
    else()
        set(ORC_STAGE_PLUGIN_INSTALL_DIR "lib/orc-stage-plugins")
    endif()
endif()

function(orc_add_stage_plugin target)
    set(options)
    set(oneValueArgs OUTPUT_NAME PLUGIN_VERSION)
    set(multiValueArgs SOURCES LINK_LIBRARIES RUNTIME_DEPENDENCIES)
    cmake_parse_arguments(ORCSP "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT ORCSP_SOURCES)
        message(FATAL_ERROR "orc_add_stage_plugin(${target}) requires SOURCES")
    endif()

    add_library(${target} SHARED ${ORCSP_SOURCES})

    if(ORCSP_OUTPUT_NAME)
        set_target_properties(${target} PROPERTIES OUTPUT_NAME "${ORCSP_OUTPUT_NAME}")
    endif()

    if(TARGET orc::plugin-sdk)
        set(_orc_sdk_target orc::plugin-sdk)
    elseif(TARGET orc-plugin-sdk)
        set(_orc_sdk_target orc-plugin-sdk)
    else()
        message(FATAL_ERROR
            "orc_add_stage_plugin: Neither orc::plugin-sdk nor orc-plugin-sdk is defined")
    endif()

    target_link_libraries(${target} PRIVATE ${_orc_sdk_target} ${ORCSP_LINK_LIBRARIES})
    target_include_directories(${target} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

    if(ORCSP_PLUGIN_VERSION)
        target_compile_definitions(${target} PRIVATE
            ORC_STAGE_PLUGIN_VERSION="${ORCSP_PLUGIN_VERSION}")
    endif()

    if(NOT DEFINED ORC_STAGE_PLUGIN_BUILD_DIR)
        set(ORC_STAGE_PLUGIN_BUILD_DIR "${CMAKE_BINARY_DIR}/lib/orc-stage-plugins")
    endif()

    set_target_properties(${target} PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY  "${ORC_STAGE_PLUGIN_BUILD_DIR}"
        RUNTIME_OUTPUT_DIRECTORY  "${ORC_STAGE_PLUGIN_BUILD_DIR}"
        ARCHIVE_OUTPUT_DIRECTORY  "${CMAKE_BINARY_DIR}/lib"
    )

    if(APPLE)
        set_target_properties(${target} PROPERTIES INSTALL_RPATH "@loader_path/../../Frameworks")
    elseif(UNIX AND NOT WIN32)
        set_target_properties(${target} PROPERTIES INSTALL_RPATH "\$ORIGIN/..")
    endif()
endfunction()

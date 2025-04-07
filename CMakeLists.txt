cmake_minimum_required(VERSION 3.10)

# Base version definition - single source of truth
set(VWIFI_BASE_VERSION "6.4.0")  # CMake requires X.Y.Z format
set(VWIFI_DEV_SUFFIX "-dev")

# Version handling
find_package(Git QUIET)
if(GIT_FOUND)
    # Get version from git describe
    execute_process(
        COMMAND ${GIT_EXECUTABLE} describe --dirty
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_DESCRIBE_VERSION
        ERROR_VARIABLE GIT_DESCRIBE_ERROR
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_STRIP_TRAILING_WHITESPACE
        RESULT_VARIABLE GIT_DESCRIBE_RESULT
    )

    if(GIT_DESCRIBE_RESULT EQUAL 0)
        # Use git describe output for display version
        set(VWIFI_DISPLAY_VERSION ${GIT_DESCRIBE_VERSION})
    else()
        # Fallback version with -dev suffix
        set(VWIFI_DISPLAY_VERSION "${VWIFI_BASE_VERSION}${VWIFI_DEV_SUFFIX}")
    endif()
else()
    # No git, use fallback version
    set(VWIFI_DISPLAY_VERSION "${VWIFI_BASE_VERSION}${VWIFI_DEV_SUFFIX}")
endif()

message(STATUS "Building version: ${VWIFI_DISPLAY_VERSION}")

# Project settings
project(vwifi 
    VERSION ${VWIFI_BASE_VERSION}  # CMake version must be X.Y.Z format
    DESCRIPTION "Simulate Wi-Fi (802.11) between Linux Virtual Machines"
    LANGUAGES CXX
)

# Global settings
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Build type configuration
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build" FORCE)
endif()

# Compiler options
add_compile_options(
    -Wall
    -Wextra
    -pedantic
    $<$<CONFIG:Release>:-O3>
    $<$<CONFIG:Release>:-s>
)

# Dependencies
find_package(Threads REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBNL REQUIRED IMPORTED_TARGET
    libnl-3.0
    libnl-genl-3.0
)

# Define source groups
set(CORE_HEADERS
    src/types.h
    src/config.h
    src/tools.h
    src/cdynbuffer.h
    src/csocket.h
    src/ccoordinate.h
    src/cwifi.h
    src/cselect.h
    src/cinfosocket.h
    src/cinfowifi.h
    src/clistinfo.h
    src/ieee80211.h
    src/hwsim.h
    src/config_hwsim.h
    src/mac80211_hwsim.h
)

set(CORE_SOURCES
    src/cdynbuffer.cc
    src/csocket.cc
    src/tools.cc
    src/ccoordinate.cc
    src/cwifi.cc
    src/cselect.cc
    src/cinfosocket.cc
    src/cinfowifi.cc
)

set(SOCKET_COMMON_HEADERS
    src/csocketserver.h
    src/csocketclient.h
    src/csocketclientitcp.h
    src/csocketclientvtcp.h
    src/csocketserverfunctionitcp.h
    src/csocketserverfunctionvtcp.h
)

set(SOCKET_COMMON_SOURCES
    src/csocketserver.cc
    src/csocketclient.cc
)

set(SERVER_HEADERS
    src/cwifiserver.h
    src/cwifiserveritcp.h
    src/cwifiservervtcp.h
    src/cctrlserver.h
)

set(SERVER_SOURCES
    src/vwifi-server.cc
    src/cctrlserver.cc
    src/csocketserverfunctionitcp.cc
    src/csocketserverfunctionvtcp.cc
    src/cwifiserveritcp.cc
    src/cwifiserver.cc
    src/cwifiservervtcp.cc
)

set(CLIENT_HEADERS
    src/cwificlient.h
    src/cwirelessdevice.h
    src/cwirelessdevicelist.h
    src/cmonwirelessdevice.h
    src/ckernelwifi.h
    src/cthread.h
    src/addinterfaces.h
)

set(CLIENT_SOURCES
    src/vwifi-client.cc
    src/addinterfaces.cc
    src/ckernelwifi.cc
    src/cmonwirelessdevice.cc
    src/csocketclientitcp.cc
    src/csocketclientvtcp.cc
    src/cthread.cc
    src/cwificlient.cc
    src/cwirelessdevicelist.cc
    src/cwirelessdevice.cc
)

set(CTRL_SOURCES
    src/vwifi-ctrl.cc
    src/csocketclientitcp.cc
)

set(ADD_INTERFACES_HEADERS
    src/addinterfaces.h
)

set(ADD_INTERFACES_SOURCES
    src/vwifi-add-interfaces.cc
    src/addinterfaces.cc
)

# Create core library
add_library(vwifi-core STATIC
    ${CORE_HEADERS}
    ${CORE_SOURCES}
    ${SOCKET_COMMON_HEADERS}
    ${SOCKET_COMMON_SOURCES}
)

target_include_directories(vwifi-core 
    PUBLIC 
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(vwifi-core
    PUBLIC
    PkgConfig::LIBNL
    Threads::Threads
)

# Function to create an executable target with common settings
function(add_vwifi_executable name sources headers)
    add_executable(${name} ${sources} ${headers})
    target_include_directories(${name} PRIVATE ${CMAKE_SOURCE_DIR}/src)
    target_link_libraries(${name} 
        PRIVATE 
        vwifi-core
        stdc++
        m
    )
    target_compile_definitions(${name} 
        PRIVATE 
        VERSION="${VWIFI_DISPLAY_VERSION}"
    )
    set_target_properties(${name} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )
endfunction()

# Create executables
add_vwifi_executable(vwifi-server "${SERVER_SOURCES}" "${SERVER_HEADERS}")
add_vwifi_executable(vwifi-client "${CLIENT_SOURCES}" "${CLIENT_HEADERS}")
add_vwifi_executable(vwifi-ctrl "${CTRL_SOURCES}" "")
add_vwifi_executable(vwifi-add-interfaces "${ADD_INTERFACES_SOURCES}" "${ADD_INTERFACES_HEADERS}")

# Installation
include(GNUInstallDirs)
install(TARGETS 
    vwifi-server 
    vwifi-client 
    vwifi-ctrl 
    vwifi-add-interfaces
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Install man pages if they exist
if(EXISTS "${CMAKE_SOURCE_DIR}/man/vwifi.1")
    install(FILES "${CMAKE_SOURCE_DIR}/man/vwifi.1"
        DESTINATION ${CMAKE_INSTALL_MANDIR}/man1
    )
endif()

# Add uninstall target
if(NOT TARGET uninstall)
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
        IMMEDIATE @ONLY)

    add_custom_target(uninstall
        COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
endif()

# Add cppcheck target
find_program(CPPCHECK cppcheck)
if(CPPCHECK)
    # Find xsltproc for HTML report generation
    find_program(XSLTPROC xsltproc)

    set(CPPCHECK_REPORT_DIR "${CMAKE_BINARY_DIR}/cppcheck-reports")
    set(CPPCHECK_XML_FILE "${CPPCHECK_REPORT_DIR}/report.xml")
    set(CPPCHECK_HTML_FILE "${CPPCHECK_REPORT_DIR}/report.html")

    add_custom_target(cppcheck
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CPPCHECK_REPORT_DIR}
        COMMAND ${CPPCHECK}
            --enable=all
            --enable=style
            --std=c++17
            --verbose
            --xml
            --xml-version=2
            --project=${CMAKE_BINARY_DIR}/compile_commands.json
            --output-file=${CPPCHECK_XML_FILE}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Running cppcheck static analysis"
    )

    if(XSLTPROC)
        add_custom_command(
            TARGET cppcheck
            POST_BUILD
            COMMAND ${XSLTPROC}
                ${CMAKE_SOURCE_DIR}/tools/cppcheck-html.xsl
                ${CPPCHECK_XML_FILE}
                > ${CPPCHECK_HTML_FILE}
            COMMENT "Generating HTML report from cppcheck XML"
        )
    else()
        message(STATUS "xsltproc not found - HTML report generation disabled")
    endif()
endif()

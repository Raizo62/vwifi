cmake_minimum_required(VERSION 3.10)

project(vwifi VERSION 7.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# --- Compiler Flags ---
# Add flags similar to the Makefile's MODE settings
# Using Release build type for optimizations by default
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose build type: Debug Release RelWithDebInfo MinSizeRel" FORCE)
endif()

option(ENABLE_VHOST "Enable vhost support" ON)
if(NOT ENABLE_VHOST)
  add_definitions(-DDISABLE_VHOST)
endif()

# Common flags (add more as needed based on Makefile MODE)
add_compile_options(-Wall -Wextra -pedantic)
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_options(-O3 -s)
    # add_definitions(-DNDEBUG) # Uncomment if asserts should be disabled in release
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-g)
    add_definitions(-D_DEBUG)
endif()

# Add version definition
# Check if this is a git repository and get the commit hash
find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.git")
    execute_process(
        COMMAND "${GIT_EXECUTABLE}" log --pretty=format:%h -n 1
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        OUTPUT_VARIABLE GIT_COMMIT_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
endif()

if(GIT_COMMIT_HASH)
    set(FULL_VERSION "${PROJECT_VERSION}-${GIT_COMMIT_HASH}")
    message(STATUS "Building Git version: ${FULL_VERSION}")
else()
    set(FULL_VERSION "${PROJECT_VERSION}")
    message(STATUS "Building version: ${FULL_VERSION} (Not a Git repo or git error)")
endif()

add_definitions(-DVERSION=\"${FULL_VERSION}\")

# --- Find Dependencies ---
find_package(Threads REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBNL3 REQUIRED IMPORTED_TARGET libnl-genl-3.0 libnl-3.0)

# --- Include Directories ---
include_directories(src)
include_directories(${LIBNL3_INCLUDE_DIRS})

# --- Common Static Library ---
# List all .cc files EXCEPT the main files for the executables
set(COMMON_SOURCES
    src/ccoordinate.cc
    src/cctrlserver.cc
    src/cdynbuffer.cc
    src/cinfosocket.cc
    src/cinfowifi.cc
    src/ckernelwifi.cc
    src/cmonwirelessdevice.cc
    src/cselect.cc
    src/csocket.cc
    src/csocketclient.cc
    src/csocketclientitcp.cc
    src/csocketclientvtcp.cc
    src/csocketserver.cc
    src/csocketserverfunctionitcp.cc
    src/csocketserverfunctionvtcp.cc
    src/cthread.cc
    src/cwifi.cc
    src/cwificlient.cc
    src/cwifiserver.cc
    src/cwifiserveritcp.cc
    src/cwifiservervtcp.cc
    src/cwirelessdevice.cc
    src/cwirelessdevicelist.cc
    src/tools.cc
    src/addinterfaces.cc
)

add_library(vwifi_common STATIC ${COMMON_SOURCES})
# Link common library dependencies once
target_link_libraries(vwifi_common PUBLIC Threads::Threads PkgConfig::LIBNL3)


# --- Executables ---
add_executable(vwifi-server src/vwifi-server.cc)
target_link_libraries(vwifi-server PRIVATE vwifi_common)

add_executable(vwifi-client src/vwifi-client.cc)
target_link_libraries(vwifi-client PRIVATE vwifi_common)

add_executable(vwifi-ctrl src/vwifi-ctrl.cc)
target_link_libraries(vwifi-ctrl PRIVATE vwifi_common)

add_executable(vwifi-add-interfaces src/vwifi-add-interfaces.cc)
# vwifi-add-interfaces seems to only need addinterfaces.o, which is in vwifi_common
target_link_libraries(vwifi-add-interfaces PRIVATE vwifi_common)

# Optional: vwifi-inet-monitor (based on commented out Makefile line)
# add_executable(vwifi-inet-monitor src/vwifi-inet-monitor.cc)
# target_link_libraries(vwifi-inet-monitor PRIVATE vwifi_common) # Check its specific dependencies if uncommenting

# --- Installation ---
include(GNUInstallDirs)
install(TARGETS vwifi-server vwifi-client vwifi-ctrl vwifi-add-interfaces
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# Optional: Install man pages if needed (requires CMake code)
# install(FILES man/vwifi.1 DESTINATION ${CMAKE_INSTALL_MANDIR}/man1 RENAME vwifi.1)

# --- Optional: Add tests ---
# enable_testing()
# add_subdirectory(tests) # If tests have their own CMakeLists.txt

# --- Custom Target for Cppcheck ---
find_program(CPPCHECK_EXECUTABLE cppcheck)
if(CPPCHECK_EXECUTABLE)
    file(GLOB_RECURSE ALL_CC_SOURCES "src/*.cc")

    get_property(GLOBAL_DEFINITIONS DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY COMPILE_DEFINITIONS)
    set(CPPCHECK_DEFS "")
    foreach(DEF ${GLOBAL_DEFINITIONS})
        if(DEF MATCHES "^-D")
            set(CPPCHECK_DEFS "${CPPCHECK_DEFS} ${DEF}")
        else()
            set(CPPCHECK_DEFS "${CPPCHECK_DEFS} -D${DEF}")
        endif()
    endforeach()
    set(CPPCHECK_DEFS "${CPPCHECK_DEFS} -D_DEBUG")

    get_property(COMMON_COMPILE_OPTIONS DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY COMPILE_OPTIONS)
    set(CPPCHECK_CFLAGS "")
    foreach(OPT ${COMMON_COMPILE_OPTIONS})
        set(CPPCHECK_CFLAGS "${CPPCHECK_CFLAGS} ${OPT}")
    endforeach()

    # Définition du chemin de sortie du fichier XML
    # CMAKE_CURRENT_BINARY_DIR pointe vers le répertoire de construction
    set(CPPCHECK_OUTPUT_FILE "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-cppcheck.xml")

    add_custom_target(
        cppcheck
        COMMAND ${CPPCHECK_EXECUTABLE} --verbose --enable=all --enable=style --xml
                ${CPPCHECK_CFLAGS} ${CPPCHECK_DEFS} ${ALL_CC_SOURCES} 2> ${CPPCHECK_OUTPUT_FILE}
        # WORKING_DIRECTORY reste le répertoire source pour que cppcheck trouve les fichiers
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Running Cppcheck static analysis and generating XML report in build directory."
        BYPRODUCTS ${CPPCHECK_OUTPUT_FILE}
    )
endif()

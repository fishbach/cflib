# Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
#
# This file is part of cflib.
#
# Licensed under the MIT License.

cmake_minimum_required(VERSION 3.28)

project(
    cflib
    VERSION 1.0.0
    LANGUAGES C CXX
)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# top-level project or subproject (e.g. used as git submodule)
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    set(CFLIB_IS_TOP_LEVEL ON)
else()
    set(CFLIB_IS_TOP_LEVEL OFF)
endif()

# library type: static by default, dynamic (shared) via -DBUILD_SHARED_LIBS=ON
option(BUILD_SHARED_LIBS "build cflib libraries as shared (dynamic) instead of static" OFF)

# tests, examples and benchmarks are built by default only when cflib is the top-level project
if(CFLIB_IS_TOP_LEVEL)
    option(BUILD_TESTS    "activate build for tests"      ON)
    option(BUILD_EXAMPLES "activate build for examples"   ON)
    option(BUILD_BENCH    "activate build for benchmarks" ON)
else()
    option(BUILD_TESTS    "activate build for tests"      OFF)
    option(BUILD_EXAMPLES "activate build for examples"   OFF)
    option(BUILD_BENCH    "activate build for benchmarks" OFF)
endif()

# install cflib (libraries, headers, cmake package) for use by other applications
option(CFLIB_INSTALL "install cflib (libraries, headers, cmake package)" ${CFLIB_IS_TOP_LEVEL})

if(NOT COMMAND cf_lib)
    include(cmake/ProjectConfig.cmake)
endif()
include(DependencyConfig)

cf_add_other(cflib_files
    .gitignore
    COPYING
    MODULES
    README.md
    js/*
)

# include root for cflib headers (cf_lib adds it to its targets),
# PIC so that static libraries can be linked into shared libraries
set(CFLIB_ROOT_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_subdirectory(cflib)
if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# ---------------------------------------------------------------------------
# install
# ---------------------------------------------------------------------------
if(CFLIB_INSTALL)
    # headers (public API, test and benchmark headers are excluded)
    install(FILES cflib/base.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/cflib)
    install(DIRECTORY
        cflib/base cflib/util cflib/crypt cflib/serialize cflib/db cflib/dao cflib/net
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/cflib
        FILES_MATCHING
            PATTERN "*.h"
            PATTERN "*_test"  EXCLUDE
            PATTERN "*_bench" EXCLUDE
            PATTERN "test"    EXCLUDE
    )

    # test main() for cf_test consumers
    install(FILES cflib/util/testmain/main.cpp
        DESTINATION ${CMAKE_INSTALL_DATADIR}/cflib/testmain)

    # libraries
    set(_cflib_lib_targets)
    foreach(_t base util crypt serialize db dao net fileserver)
        if(TARGET cflib_${_t})
            list(APPEND _cflib_lib_targets cflib_${_t})
        endif()
    endforeach()
    if(CFLIB_BOTAN_BUNDLED AND TARGET cflib_botan)
        list(APPEND _cflib_lib_targets cflib_botan)
    endif()
    if(CFLIB_SQLITE_BUNDLED AND TARGET cflib_sqlite)
        list(APPEND _cflib_lib_targets cflib_sqlite)
    endif()

    if(BUILD_SHARED_LIBS)
        # versioned sonames, and the libraries must find each other
        # after installation (transitive deps need RUNPATH, not just the
        # executable's)
        set_target_properties(${_cflib_lib_targets} PROPERTIES
            VERSION      ${cflib_VERSION}
            SOVERSION    ${cflib_VERSION_MAJOR}
            INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}"
        )
    endif()

    # cflib requires C++20, convey it to consumer targets
    foreach(_t IN LISTS _cflib_lib_targets)
        target_compile_features(${_t} PUBLIC cxx_std_20)
    endforeach()

    install(TARGETS ${_cflib_lib_targets}
        EXPORT cflibTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )

    # tools (ser, apiexporter and bin2src are needed at build time by consumers)
    set(_cflib_tools)
    foreach(_t ser apiexporter bin2src jscombiner asn1dump pwhash htmlexporter)
        if(TARGET ${_t})
            list(APPEND _cflib_tools ${_t})
        endif()
    endforeach()
    # with shared libraries the tools must find them after installation
    set_target_properties(${_cflib_tools} PROPERTIES
        INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}"
    )
    install(TARGETS ${_cflib_tools}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )

    # cmake package (find_package(cflib))
    configure_file(cmake/cflibConfig.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/cflibConfig.cmake @ONLY)
    write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/cflibConfigVersion.cmake
        VERSION ${cflib_VERSION}
        COMPATIBILITY SameMajorVersion
    )
    install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/cflibConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/cflibConfigVersion.cmake
        cmake/Build.cmake
        cmake/Util.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/cflib)
    install(EXPORT cflibTargets
        FILE cflibTargets.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/cflib)
endif()

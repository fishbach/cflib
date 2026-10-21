# Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
#
# This file is part of cflib.
#
# Licensed under the MIT License.

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# cflib version config
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/cflib-config-version.cmake"
    VERSION       ${cflib_VERSION}
    COMPATIBILITY SameMajorVersion
)
install(
    FILES       "${CMAKE_CURRENT_BINARY_DIR}/cflib-config-version.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/cflib
)

# dependency config
set(CFLIB_INSTALL_CONFIG_DEPS
    "find_dependency(Threads REQUIRED)"
    "find_dependency(ZLIB REQUIRED)"
)
if(NOT USE_FETCHCONTENT)
    list(APPEND CFLIB_INSTALL_CONFIG_DEPS "find_dependency(Botan ${Botan_VERSION} REQUIRED)")
    if(ENABLE_SQLITE)
        list(APPEND CFLIB_INSTALL_CONFIG_DEPS "find_dependency(SQLite3 ${CFLIB_SQLITE_SYSTEM_VERSION} REQUIRED)")
    endif()
endif()
string(JOIN "\n" CFLIB_INSTALL_CONFIG_DEPS ${CFLIB_INSTALL_CONFIG_DEPS})
configure_package_config_file(
    "cmake/cflib-config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cflib-config.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_DATADIR}/cmake/cflib
)
install(
    FILES       "${CMAKE_CURRENT_BINARY_DIR}/cflib-config.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/cflib
)

# export headers
file(GLOB_RECURSE cflib_headers
    RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
    "cflib/*.h"
)
list(FILTER cflib_headers EXCLUDE REGEX "_test/|_bench/")
foreach(header ${cflib_headers})
    get_filename_component(dir ${header} DIRECTORY)
    install(FILES "${header}" DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${dir}")
endforeach()

# targets to be exported
set(cflib_targets
    cflib_apiexporter
    cflib_base
    cflib_bin2src
    cflib_crypt
    cflib_dao
    cflib_db
    cflib_fileserver
    cflib_htmlexporter
    cflib_net
    cflib_ser
    cflib_serialize
    cflib_util
)
if(USE_FETCHCONTENT)
    list(APPEND cflib_targets cflib_botan)
    if(ENABLE_SQLITE)
        list(APPEND cflib_targets cflib_sqlite)
    endif()
endif()

# export cflib targets
install(
    TARGETS  ${cflib_targets}
    EXPORT   cflib-targets
    ARCHIVE  DESTINATION ${CMAKE_INSTALL_LIBDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(
    EXPORT      cflib-targets
    FILE        cflib-targets.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/cflib
)

# testmain OBJECT lib
install(
    FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/cflib/util/testmain/main.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/cflib/util/testmain/pch.h"
    DESTINATION ${CMAKE_INSTALL_DATADIR}/cflib/util/testmain
)

# export CMake scripts
install(
    FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Build.cmake"
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/ProjectConfig.cmake"
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Util.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/cflib
)

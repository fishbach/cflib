# Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
#
# This file is part of cflib.
#
# Licensed under the MIT License.

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(cflib_targets
        cflib_base
        cflib_botan
        cflib_crypt
        cflib_dao
        cflib_db
        cflib_fileserver
        cflib_net
        cflib_serialize
        cflib_util
)
if(ENABLE_SQLITE)
    list(APPEND cflib_targets cflib_sqlite)
endif()

install(
    TARGETS ${cflib_targets}
    EXPORT cflib-targets
    ARCHIVE  DESTINATION ${CMAKE_INSTALL_LIBDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    LIBRARY  DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(
    EXPORT cflib-targets
    FILE cflib-targets.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/cflib
)

file(GLOB_RECURSE cflib_headers
    RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
    "cflib/*.h"
)
foreach(header ${cflib_headers})
    get_filename_component(dir ${header} DIRECTORY)
    install(FILES "${header}" DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${dir}")
endforeach()

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/cflib-config-version.cmake"
    VERSION ${cflib_VERSION}
    COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
    "cmake/cflib-config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cflib-config.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_DATADIR}/cmake/cflib
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/cflib-config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/cflib-config-version.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/cflib
)

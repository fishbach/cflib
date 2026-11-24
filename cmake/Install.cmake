# Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
#
# This file is part of cflib.
#
# Licensed under the MIT License.

block()

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# fetch targets
get_property(CFLIB_LIB_TARGETS       GLOBAL PROPERTY CFLIB_LIB_TARGETS      )
get_property(CFLIB_GENERATOR_TARGETS GLOBAL PROPERTY CFLIB_GENERATOR_TARGETS)

# export only generators
if(CFLIB_ONLY_GENERATORS)
    install(
        TARGETS ${CFLIB_GENERATOR_TARGETS}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
    return()
endif()

# add version config
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/cflib-config-version.cmake"
    VERSION       ${cflib_VERSION}
    COMPATIBILITY SameMajorVersion
)
install(
    FILES       "${CMAKE_CURRENT_BINARY_DIR}/cflib-config-version.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/cflib
)

# dependencies
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
if(ENABLE_PSQL)
    list(APPEND CFLIB_INSTALL_CONFIG_DEPS "find_dependency(PostgreSQL REQUIRED)")
endif()
string(JOIN "\n" CFLIB_INSTALL_CONFIG_DEPS ${CFLIB_INSTALL_CONFIG_DEPS})

# generators from PATH
if(CFLIB_USE_GENERATORS_FROM_PATH)
    set(CFLIB_INSTALL_GENERATORS_FROM_PATH "set(CFLIB_USE_GENERATORS_FROM_PATH ON)")
    foreach(gen IN LISTS CFLIB_GENERATOR_TARGETS)
        string(APPEND CFLIB_INSTALL_GENERATORS_FROM_PATH "\n    cf_app(${gen} CF_GENERATOR)")
    endforeach()
    set(install_targets ${CFLIB_LIB_TARGETS})
else()
    set(CFLIB_INSTALL_GENERATORS_FROM_PATH "set(CFLIB_USE_GENERATORS_FROM_PATH OFF)")
    set(install_targets ${CFLIB_LIB_TARGETS} ${CFLIB_GENERATOR_TARGETS})
endif()

# add config file
configure_package_config_file(
    "cmake/cflib-config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cflib-config.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_DATADIR}/cmake/cflib
)
install(
    FILES       "${CMAKE_CURRENT_BINARY_DIR}/cflib-config.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/cflib
)

# add header files
file(GLOB_RECURSE cflib_headers
    RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
    "cflib/*.h"
)
list(FILTER cflib_headers EXCLUDE REGEX "_test/|_bench/")
foreach(header ${cflib_headers})
    get_filename_component(dir ${header} DIRECTORY)
    install(FILES "${header}" DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${dir}")
endforeach()

# export cflib targets
install(
    TARGETS  ${install_targets}
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

# JavaScript lib
install(
    DIRECTORY js
    DESTINATION ${CMAKE_INSTALL_DATADIR}/cflib
)

endblock()

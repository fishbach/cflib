# Copyright (C) 2013-2023 Christian Fischbach <cf@cflib.de>
#
# This file is part of cflib.
#
# Licensed under the MIT License.

# python is needed for configure.py
find_package(Python COMPONENTS Interpreter REQUIRED)

# get sources
include(FetchContent)
FetchContent_Declare(
    botan_src
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    URL https://github.com/randombit/botan/archive/refs/tags/${Botan_FIND_VERSION}.tar.gz
)
FetchContent_MakeAvailable(botan_src)

# handle find_packet logic
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Botan DEFAULT_MSG botan_src_SOURCE_DIR)

# prepare parameters for configure.py
string(TOLOWER ${CMAKE_CXX_COMPILER_ID} botan_src_COMPILER_ID)
if (botan_src_COMPILER_ID STREQUAL "gnu")
    set(botan_src_COMPILER_ID "gcc")
elseif (botan_src_COMPILER_ID STREQUAL "appleclang")
    set(botan_src_COMPILER_ID "clang")
endif()
set(botan_src_DISABLE_MODULES pkcs11)
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    list(APPEND botan_src_DISABLE_MODULES simd_avx512)
endif()
if(APPLE)
    list(APPEND botan_src_DISABLE_MODULES certstor_system_macos)
endif()
list(JOIN botan_src_DISABLE_MODULES "," botan_src_DISABLE_MODULES)

# call configure.py
add_custom_command(
    OUTPUT ${botan_src_SOURCE_DIR}/botan_all.h ${botan_src_SOURCE_DIR}/botan_all.cpp
    WORKING_DIRECTORY ${botan_src_SOURCE_DIR}
    COMMAND ${Python_EXECUTABLE} configure.py
        --quiet
        --cc=${botan_src_COMPILER_ID}
        --cc-bin=${CMAKE_CXX_COMPILER}
        $<$<BOOL:${MINGW}>:--os=mingw>
        --disable-shared
        --amalgamation
        --no-store-vc-rev
        --disable-modules=${botan_src_DISABLE_MODULES}
)

# add target cflib_botan
add_library(cflib_botan ${botan_src_SOURCE_DIR}/botan_all.cpp)
target_include_directories(cflib_botan INTERFACE ${botan_src_SOURCE_DIR}/build/include)

# Copyright (C) 2013-2023 Christian Fischbach <cf@cflib.de>
#
# This file is part of cflib.
#
# Licensed under the MIT License.

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ccache
option(ENABLE_CCACHE "enable ccache" ON)
if(ENABLE_CCACHE)
	find_program(CCACHE_FOUND ccache)
	if(CCACHE_FOUND)
		set(CMAKE_CXX_COMPILER_LAUNCHER ccache)
	endif()
endif()

include_directories(${PROJECT_SOURCE_DIR})

# PCH
option(ENABLE_PCH "enable precompiled headers (PCH)" ON)
if(ENABLE_PCH)
	add_library(pch_core pch/dummy.cpp)
	target_precompile_headers(pch_core PRIVATE pch/core.h)
	target_link_libraries(pch_core PRIVATE Qt5::Core)
endif()

# Qt
find_package(
	Qt5 5.15
	COMPONENTS Core Test
	REQUIRED
)

# libraries
find_package(ZLIB REQUIRED)

# cflib targets
function(cf_configure_target target)
	if(ENABLE_PCH)
		target_precompile_headers(${target} REUSE_FROM pch_core)
	endif()

	set_target_properties(${target} PROPERTIES AUTOMOC ON)
endfunction()

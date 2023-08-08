find_package(
	Python
	COMPONENTS Interpreter
	REQUIRED
)

include(FetchContent)
FetchContent_Declare(
	botan_src
	URL https://github.com/randombit/botan/archive/refs/tags/${Botan_FIND_VERSION}.tar.gz
	DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(botan_src)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Botan DEFAULT_MSG botan_src_SOURCE_DIR)

string(TOLOWER ${CMAKE_CXX_COMPILER_ID} botan_src_COMPILER_ID)
if (botan_src_COMPILER_ID STREQUAL "gnu")
	set(botan_src_COMPILER_ID "gcc")
elseif (botan_src_COMPILER_ID STREQUAL "appleclang")
	set(botan_src_COMPILER_ID "clang")
endif()
list(JOIN Botan_FIND_COMPONENTS "," botan_src_ENABLE_MODULES)

add_custom_command(
	OUTPUT ${botan_src_SOURCE_DIR}/botan_all.h ${botan_src_SOURCE_DIR}/botan_all.cpp
	WORKING_DIRECTORY ${botan_src_SOURCE_DIR}
	COMMAND ${Python_EXECUTABLE}
		configure.py
		--quiet
		--cc=${botan_src_COMPILER_ID}
		--cc-bin=${CMAKE_CXX_COMPILER}
		$<$<BOOL:${MINGW}>:--os=mingw>
		--disable-shared
		--amalgamation
		--minimized-build
		--enable-modules=${botan_src_ENABLE_MODULES}
)

add_library(cflib_botan ${botan_src_SOURCE_DIR}/botan_all.cpp)
target_include_directories(cflib_botan INTERFACE ${botan_src_SOURCE_DIR}/build/include)

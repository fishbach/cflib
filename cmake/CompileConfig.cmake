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
	COMPONENTS
		Core
		Test
	REQUIRED
)

# Botan
find_package(
	Botan 3.1.1
	COMPONENTS
		aead
		asn1
		auto_rng
		bcrypt
		cbc
		cfb
		chacha_rng
		des
		filters
		keypair
		mode_pad
		pbes2
		pem
		prf_tls
		processor_rng
		pubkey
		rng
		rsa
		stateful_rng
		system_rng
		tls12
		tls13
		tls_cbc
		x509
		xts
	REQUIRED
)

# libraries
find_package(ZLIB REQUIRED)

# cflib targets
function(cf_configure_target target enable_moc enable_ser)
	if(ENABLE_PCH)
		target_precompile_headers(${target} REUSE_FROM pch_core)
	endif()

	set_target_properties(${target} PROPERTIES AUTOMOC ${enable_moc})

	# autogenerated serialization
	if(enable_ser)
		# get header files
		get_target_property(headers ${target} SOURCES)
		list(FILTER headers INCLUDE REGEX "\.h$")

		# filter by containing of SERIALIZE_CLASS
		foreach(header ${headers})
			file(STRINGS "${header}" lines REGEX "SERIALIZE_CLASS")
			if(NOT lines)
				list(REMOVE_ITEM headers "${header}")
			endif()
		endforeach()

		foreach(header ${headers})
			# get output filename (dir/header.h -> target_autogen/dir/header_ser.cpp)
			get_filename_component(source "${header}" NAME_WLE)
			get_filename_component(dir "${header}" DIRECTORY)
			if(dir)
				set(source "${dir}/${source}")
			endif()
			set(source "${target}_autogen/${source}_ser.cpp")

			# add generation step
			add_custom_command(
				OUTPUT "${source}"
				COMMAND ser serialize "${CMAKE_CURRENT_SOURCE_DIR}/${header}" "${source}"
				DEPENDS "${header}"
			)
			target_sources(${target} PRIVATE "${source}")
		endforeach()
	endif()
endfunction()

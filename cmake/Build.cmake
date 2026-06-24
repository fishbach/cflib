# Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
#
# This file is part of cflib.
#
# Licensed under the MIT License.

# library
function(cf_lib lib)
    cmake_parse_arguments(ARG "ENABLE_EXCEPTIONS;ENABLE_SER" "PCH" "PUBLIC;PRIVATE;DIRS;OTHER_FILES" ${ARGN})

    cf_find_sources(sources . ${ARG_DIRS} OTHER_FILES ${ARG_OTHER_FILES})
    add_library(${lib} ${sources})
    cf_configure_target(${lib} ${ARG_ENABLE_EXCEPTIONS} "${ARG_PCH}" ${ARG_ENABLE_SER})
    target_include_directories(${lib} PUBLIC "${PROJECT_SOURCE_DIR}")
    target_link_libraries(${lib} PUBLIC ${ARG_PUBLIC} PRIVATE ${ARG_PRIVATE})
endfunction()

# application
function(cf_app app)
    cmake_parse_arguments(ARG "ENABLE_EXCEPTIONS;ENABLE_SER;ENABLE_GIT_VERSION;CF_INTERN" "PCH;DAO" "DIRS;RESOURCES;OTHER_FILES;REMOTE_APIS" ${ARGN})
    if(ARG_REMOTE_APIS)
        set(ARG_ENABLE_SER TRUE)
    endif()

    # sources, libs and general config
    cf_find_sources(sources . ${ARG_DIRS} OTHER_FILES ${ARG_OTHER_FILES})
    add_executable(${app} ${sources})
    cf_configure_target(${app} ${ARG_ENABLE_EXCEPTIONS} "${ARG_PCH}" ${ARG_ENABLE_SER} ${ARG_RESOURCES})
    target_include_directories(${app} PRIVATE .)
    if(NOT ARG_CF_INTERN)
        set_target_properties(${app} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")
    endif()
    target_link_libraries(${app} PRIVATE ${ARG_UNPARSED_ARGUMENTS})

    # dao as lib
    if(ARG_DAO)
        cf_find_sources(sources ${ARG_DAO})
        add_library(${app}_dao ${sources})
        cf_configure_target(${app}_dao FALSE "" TRUE)
        target_include_directories(${app}_dao PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
        target_link_libraries(${app}_dao PUBLIC cflib_dao)
        target_link_libraries(${app}_dao PUBLIC cflib_serialize)
        target_link_libraries(${app} PRIVATE ${app}_dao)
        set_target_properties(${app} PROPERTIES
            DAO_LIB_PATH ${ARG_DAO}
        )
    endif()

    # remote APIs
    if(ARG_REMOTE_APIS)
        # collect all RMI_HEADERS
        get_target_property(RMI_HEADERS         ${app}    RMI_HEADERS        )
        get_target_property(CFLIB_DAO_HEADERS   cflib_dao RMI_HEADERS        )
        list(APPEND RMI_HEADERS ${CFLIB_DAO_HEADERS})
        if(ARG_DAO)
            get_target_property(DAO_RMI_HEADERS ${app}_dao RMI_HEADERS)
            list(APPEND RMI_HEADERS ${DAO_RMI_HEADERS})
        endif()

        # parse DEST from parameters
        set(REMOTE_DIR)
        set(REMOTE_APIS)
        set(found_DEST FALSE)
        foreach(api IN LISTS ARG_REMOTE_APIS)
            if(api STREQUAL "DEST")
                set(found_DEST TRUE)
                if(REMOTE_APIS)
                    cf_generate_api()
                    set(REMOTE_APIS)
                endif()
                continue()
            endif()
            if(found_DEST)
                set(found_DEST FALSE)
                set(REMOTE_DIR "${api}")
                continue()
            endif()
            list(APPEND REMOTE_APIS ${api})
        endforeach()
        if(REMOTE_APIS)
            cf_generate_api()
        endif()
    endif()

    # strip release builds and split debug info
    if(NOT APPLE)
        add_custom_command(TARGET "${app}" POST_BUILD
            COMMAND $<$<CONFIG:Release>:${CMAKE_STRIP}>           ARGS --strip-all $<TARGET_FILE:${app}>
            COMMAND $<$<CONFIG:RelWithDebInfo>:${CMAKE_OBJCOPY}>  ARGS --only-keep-debug $<TARGET_FILE:${app}> $<TARGET_FILE:${app}>.debug
            COMMAND $<$<CONFIG:RelWithDebInfo>:${CMAKE_STRIP}>    ARGS -g $<TARGET_FILE:${app}>
            COMMAND $<$<CONFIG:RelWithDebInfo>:${CMAKE_OBJCOPY}>  ARGS --add-gnu-debuglink=$<TARGET_FILE:${app}>.debug $<TARGET_FILE:${app}>
        )
    endif()

    # gitversion.h
    if(ARG_ENABLE_GIT_VERSION)
        set(dest "${app}_autogen")
        set(header "${dest}/gitversion.h")
        add_custom_target(ALL
            ${CMAKE_COMMAND} -E make_directory "${dest}"
            COMMAND gitversion create "${CMAKE_SOURCE_DIR}" "${header}"
            BYPRODUCTS "${header}"
            DEPENDS gitversion
        )
        target_sources(${app} PRIVATE "${header}")
        get_property(target_dir TARGET ${app} PROPERTY BINARY_DIR)
        target_include_directories(${app} PRIVATE "${target_dir}/${dest}")
    endif()
endfunction()

# test
function(cf_test test lib)
    cmake_parse_arguments(ARG "ENABLE_EXCEPTIONS;ENABLE_SER" "PCH" "DIRS;RESOURCES" ${ARGN})

    cf_find_sources(sources . ${ARG_DIRS})
    add_executable(${test} ${sources})
    cf_configure_target(${test} ${ARG_ENABLE_EXCEPTIONS} "${ARG_PCH}" ${ARG_ENABLE_SER} ${ARG_RESOURCES})
    target_link_libraries(${test} PRIVATE ${lib})
    add_test(NAME ${test} COMMAND ${test})
endfunction()

# configure target
function(cf_configure_target target enable_exceptions pch enable_ser)
    # exceptions
    if(NOT enable_exceptions)
        if(MSVC)
            target_compile_options(${target} PRIVATE /EHs- /D_HAS_EXCEPTIONS=0)
        else()
            target_compile_options(${target} PRIVATE -fno-exceptions)
        endif()
    endif()

    # PCH
    if(ENABLE_PCH AND NOT "${target}" STREQUAL "cflib_base")
        if(pch)
            target_precompile_headers(${target} PRIVATE ${pch})
        else()
            target_precompile_headers(${target} REUSE_FROM cflib_base)
        endif()
    endif()

    # autogenerated serialization
    if(enable_ser)
        # get header files
        get_target_property(headers ${target} SOURCES)
        list(FILTER headers INCLUDE REGEX "\.h$")

        # filter by containing of SERIALIZE_CLASS
        set(RMI_HEADERS)
        foreach(header ${headers})
            file(STRINGS "${header}" lines REGEX "SERIALIZE_CLASS")
            if(NOT lines)
                list(REMOVE_ITEM headers "${header}")
                continue()
            endif()

            cmake_path(APPEND CMAKE_CURRENT_SOURCE_DIR "${header}" OUTPUT_VARIABLE header_full_path)
            list(APPEND RMI_HEADERS "${header_full_path}")
        endforeach()

        # Something to do?
        if(NOT headers)
            return()
        endif()

        # store found headers
        set_target_properties(${target} PROPERTIES RMI_HEADERS "${RMI_HEADERS}")

        foreach(header ${headers})
            # get output filename (dir/header.h -> target_autogen/dir/header_ser.cpp)
            set(rel_dir "${target}_autogen")
            get_filename_component(dir "${header}" DIRECTORY)
            if(dir)
                set(rel_dir "${rel_dir}/${dir}")
            endif()
            get_filename_component(source "${header}" NAME_WLE)
            set(source "${rel_dir}/${source}_ser.cpp")

            # add generation step
            add_custom_command(
                OUTPUT "${source}"
                COMMAND ${CMAKE_COMMAND} -E make_directory "${rel_dir}"
                COMMAND ser serialize "${CMAKE_CURRENT_SOURCE_DIR}/${header}" "${source}"
                DEPENDS ser "${header}"
                VERBATIM
            )
            target_sources(${target} PRIVATE "${source}")
        endforeach()
    endif()

    # resources
    foreach(resource ${ARGN})
        # get output filename (dir/resource.bin -> target_autogen/dir/resource.bin_rc.cpp)
        set(rel_dir "${CMAKE_CURRENT_BINARY_DIR}/${target}_autogen")
        get_filename_component(dir "${resource}" DIRECTORY)
        if(dir)
            set(rel_dir "${rel_dir}/${dir}")
        endif()
        set(source "${rel_dir}/${resource}_rc.cpp")

        # add generation step
        add_custom_command(
            OUTPUT "${source}"
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
            COMMAND ${CMAKE_COMMAND} -E make_directory "${rel_dir}"
            COMMAND bin2src "${resource}" "${source}"
            DEPENDS bin2src "${resource}"
            VERBATIM
        )
        target_sources(${target} PRIVATE "${source}")
    endforeach()
endfunction()

# API generation
function(cf_generate_api)
    if(NOT REMOTE_DIR)
        message(FATAL_ERROR "No destination for REMOTE_APIS specified.")
    endif()

    # set REMOTE_ABS_DIR
    cmake_path(APPEND CMAKE_CURRENT_SOURCE_DIR "${REMOTE_DIR}" OUTPUT_VARIABLE REMOTE_ABS_DIR)

    # version.md will always be generated
    set(output)
    cmake_path(APPEND REMOTE_ABS_DIR version.md OUTPUT_VARIABLE version_full_path)
    list(APPEND output "${version_full_path}")

    if(cpp IN_LIST REMOTE_APIS)
        # add _services lib
        add_library(${app}_services)
        add_library(${app}::services ALIAS ${app}_services)
        target_include_directories(${app}_services PUBLIC "${REMOTE_ABS_DIR}/cpp")
        target_link_libraries(${app}_services PUBLIC ${app}_dao cflib_net)

        # create symlink for dao
        if(ARG_DAO)
            set(dao_link "${REMOTE_ABS_DIR}/cpp/${ARG_DAO}")
            get_filename_component(dao_base_dir "${dao_link}" DIRECTORY)
            file(MAKE_DIRECTORY "${dao_base_dir}")
            file(RELATIVE_PATH dao_rel_path "${dao_base_dir}" "${CMAKE_CURRENT_SOURCE_DIR}/${ARG_DAO}")
            execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink "${dao_rel_path}" "${dao_link}")
        endif()

        # add cpp output files
        foreach(header ${RMI_HEADERS})
            # check for service
            file(STRINGS "${header}" lines REGEX "RMIService")
            if(NOT lines)
                continue()
            endif()

            # remove .h
            get_filename_component(dir  "${header}" DIRECTORY)
            get_filename_component(file "${header}" NAME_WLE )
            file(RELATIVE_PATH dir "${CMAKE_CURRENT_SOURCE_DIR}" "${dir}")
            set(file "${dir}/${file}")

            list(APPEND output
                "${REMOTE_ABS_DIR}/cpp/${file}.h"
                "${REMOTE_ABS_DIR}/cpp/${file}.cpp"
            )
            target_sources(${app}_services
                PUBLIC  "${REMOTE_DIR}/cpp/${file}.h"
                PRIVATE "${REMOTE_DIR}/cpp/${file}.cpp"
            )
        endforeach()
    else()
        string(MAKE_C_IDENTIFIER "${app}_${REMOTE_DIR}_api" target)
        add_custom_target(${target} ALL DEPENDS ${output})
    endif()

    # format type parameters
    set(output_types)
    foreach(type ${REMOTE_APIS})
        list(APPEND output_types -t ${type})
    endforeach()

    # command
    cf_git_version(GIT_HASH)
    add_custom_command(
        OUTPUT ${output}
        COMMAND apiexporter --dest "${REMOTE_ABS_DIR}" --git ${GIT_HASH} --name ${app} ${output_types} ${RMI_HEADERS}
        DEPENDS apiexporter ${RMI_HEADERS}
        VERBATIM
    )
endfunction()

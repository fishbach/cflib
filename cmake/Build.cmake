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

# remote lib for application
function(cf_remote app)
    cmake_parse_arguments(ARG "" "" "REMOTE_APIS" ${ARGN})

    # Due to a bug in cmake (at least 4.3.2) it is not possible to implement this
    # as a parameter of the function cf_app. It must called be in a different directory.

    # vars from application
    get_target_property(APP_SOURCE_DIR      ${app}    SOURCE_DIR         )
    get_target_property(DAO_LIB_PATH        ${app}    DAO_LIB_PATH       )
    get_target_property(RMI_SERVICE_HEADERS ${app}    RMI_SERVICE_HEADERS)
    get_target_property(RMI_HEADERS         ${app}    RMI_HEADERS        )
    get_target_property(CFLIB_DAO_DIR       cflib_dao SOURCE_DIR         )
    get_target_property(CFLIB_DAO_HEADERS   cflib_dao RMI_HEADERS        )



    # generate remote
    if(NOT ARG_REMOTE_APIS)
        message(FATAL_ERROR "no REMOTE_APIS specified")
    endif()

    foreach(cflib_dao_header ${CFLIB_DAO_HEADERS})
        cmake_path(APPEND CFLIB_DAO_DIR "${cflib_dao_header}" OUTPUT_VARIABLE cflib_dao_header)
        list(APPEND RMI_HEADERS "${cflib_dao_header}")
    endforeach()

    if(TARGET ${app}_dao)
        get_target_property(DAO_RMI_HEADERS ${app}_dao RMI_HEADERS)
        list(APPEND RMI_HEADERS ${DAO_RMI_HEADERS})
    endif()

    set(output "${CMAKE_CURRENT_SOURCE_DIR}/dudi")

    set(output_types)
    foreach(type ${ARG_REMOTE_APIS})
        list(APPEND output_types -t ${type})
    endforeach()

    add_custom_command(
        OUTPUT ${output}
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        COMMAND apiexporter ${output_types} ${RMI_HEADERS}
        DEPENDS apiexporter ${RMI_HEADERS}
        VERBATIM
    )

    add_custom_target(${app}_doc ALL
        DEPENDS ${output}
    )




    # create symlink for dao
    if(DAO_LIB_PATH)
        set(dao_link "${CMAKE_CURRENT_SOURCE_DIR}/${DAO_LIB_PATH}")
        get_filename_component(dao_base_dir "${dao_link}" DIRECTORY)
        file(MAKE_DIRECTORY "${dao_base_dir}")
        file(RELATIVE_PATH dao_rel_path "${dao_base_dir}" "${APP_SOURCE_DIR}/${DAO_LIB_PATH}")
        execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink "${dao_rel_path}" "${dao_link}")
    endif()

    # add _services lib
    add_library(${app}_services)
    add_library(${app}::services ALIAS ${app}_services)
    target_include_directories(${app}_services PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(${app}_services PUBLIC ${app}_dao cflib_net)

    # configure code generation
    set(output)
    foreach(header ${RMI_SERVICE_HEADERS})
        # remove .h
        get_filename_component(dir  "${header}" DIRECTORY)
        get_filename_component(file "${header}" NAME_WLE )
        set(file "${dir}/${file}")

        list(APPEND output
            "${CMAKE_CURRENT_SOURCE_DIR}/${file}.h"
            "${CMAKE_CURRENT_SOURCE_DIR}/${file}.cpp"
        )

        target_sources(${app}_services
            PUBLIC  "${file}.h"
            PRIVATE "${file}.cpp"
        )
    endforeach()

    add_custom_command(
        OUTPUT ${output}
        COMMAND ${app} --export "${CMAKE_CURRENT_SOURCE_DIR}"
        DEPENDS ${app}
        VERBATIM
    )
endfunction()

# application
function(cf_app app)
    cmake_parse_arguments(ARG "ENABLE_EXCEPTIONS;ENABLE_SER;ENABLE_GIT_VERSION;CF_INTERN" "PCH;DAO" "DIRS;RESOURCES;OTHER_FILES" ${ARGN})

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
        target_link_libraries(${app}_dao PUBLIC cflib_dao)
        target_link_libraries(${app}_dao PUBLIC cflib_serialize)
        target_link_libraries(${app} PRIVATE ${app}_dao)
        set_target_properties(${app} PROPERTIES
            DAO_LIB_PATH ${ARG_DAO}
        )
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
        set(services)
        foreach(header ${headers})
            file(STRINGS "${header}" lines REGEX "SERIALIZE_CLASS")
            if(NOT lines)
                list(REMOVE_ITEM headers "${header}")
            endif()
            file(STRINGS "${header}" lines REGEX "RMIService")
            if(lines)
                list(APPEND services "${header}")
            endif()
        endforeach()
        if(services)
            set_target_properties(${target} PROPERTIES
                RMI_SERVICE_HEADERS "${services}"
            )
        endif()

        # Something to do?
        if(NOT headers)
            return()
        endif()

        set_target_properties(${target} PROPERTIES
            RMI_HEADERS "${headers}"
        )

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

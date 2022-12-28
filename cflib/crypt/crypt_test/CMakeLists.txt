find_package(Qt5 COMPONENTS Test REQUIRED)

set(ALL_TESTS
    tlscredentials_test
    tlsserver_test
    util_test
)

foreach(test ${ALL_TESTS})

    set(target ${PROJECT_NAME}_${test})

    add_executable(${target}
        ${test}.cpp
    )

    target_include_directories(${target}
        PRIVATE $<BUILD_INTERFACE:${CFLIB_SUPERPROJECT_SOURCE_DIR}>
    )

    target_link_libraries(${target}
        PRIVATE Qt5::Test
        PRIVATE cflib_crypt
        PRIVATE cflib_util
    )

    add_test(NAME ${target}
        COMMAND ${target}
    )

endforeach()
find_package(
    Qt5
    COMPONENTS Test
    REQUIRED)

include(SerializableGenerator)

set(ALL_TESTS dynamic_test gen_ber_test serializeber_test typeinfo_test util_test)

foreach(test ${ALL_TESTS})

    set(target ${PROJECT_NAME}_${test})

    add_executable(${target} ${test}.cpp)

    target_include_directories(${target} PRIVATE $<BUILD_INTERFACE:${CFLIB_SUPERPROJECT_SOURCE_DIR}>)

    target_link_libraries(
        ${target}
        PRIVATE Qt5::Test
        PRIVATE cflib::serialize
        PRIVATE cflib::util)

    add_test(NAME ${target} COMMAND ${target})

    add_dependencies(${target} ser) # The serializer is needed for the test

    generate_implementations_for(TARGET ${target})

endforeach()

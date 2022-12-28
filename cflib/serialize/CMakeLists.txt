project(cflib_serialize VERSION ${CFLIB_SUPERPROJECT_VERSION} LANGUAGES CXX)

find_package(Qt5 COMPONENTS Core REQUIRED)

add_library(${PROJECT_NAME} STATIC
    asn1dump.cpp
    serializetypeinfo.cpp
    tribool.cpp
    impl/registerclass.cpp
)

target_include_directories(${PROJECT_NAME}
    PRIVATE $<BUILD_INTERFACE:${CFLIB_SUPERPROJECT_SOURCE_DIR}>
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt5::Core
)

add_subdirectory(asn1dump)
add_subdirectory(ser)
add_subdirectory(serialize_test)
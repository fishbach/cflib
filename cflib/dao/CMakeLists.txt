project(
    cflib_dao
    VERSION ${CFLIB_SUPERPROJECT_VERSION}
    LANGUAGES CXX)

find_package(
    Qt5
    COMPONENTS Core
    REQUIRED)

include(SerializableGenerator)

add_library(${PROJECT_NAME} config.cpp configpsql.cpp version.cpp)
add_library(cflib::dao ALIAS ${PROJECT_NAME})

generate_implementations_for(TARGET ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PRIVATE $<BUILD_INTERFACE:${CFLIB_SUPERPROJECT_SOURCE_DIR}>)

target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Core)

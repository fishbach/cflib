project(
    webchat
    VERSION ${CFLIB_SUPERPROJECT_VERSION}
    LANGUAGES CXX)

find_package(
    Qt5
    COMPONENTS Core
    REQUIRED)

include(SerializableGenerator)

add_executable(${PROJECT_NAME} main.cpp infoservice.cpp)

generate_implementations_for(TARGET ${PROJECT_NAME})

target_include_directories(
    ${PROJECT_NAME}
    PRIVATE $<BUILD_INTERFACE:${CFLIB_SUPERPROJECT_SOURCE_DIR}>
    PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE Qt5::Core
    PRIVATE cflib::crypt
    PRIVATE cflib::net
    PRIVATE cflib::serialize
    PRIVATE cflib::util)

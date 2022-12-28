project(
    getssl
    VERSION ${CFLIB_SUPERPROJECT_VERSION}
    LANGUAGES CXX)

find_package(
    Qt5
    COMPONENTS Core Network
    REQUIRED)

add_executable(${PROJECT_NAME} main.cpp letsencrypt.cpp)

target_include_directories(${PROJECT_NAME} PRIVATE $<BUILD_INTERFACE:${CFLIB_SUPERPROJECT_SOURCE_DIR}>
                                                   $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE Qt5::Core
    PRIVATE Qt5::Network
    PRIVATE cflib::net
    PRIVATE cflib::crypt
    PRIVATE cflib::util)

project(
    webchatclient
    VERSION ${CFLIB_SUPERPROJECT_VERSION}
    LANGUAGES CXX)

find_package(
    Qt5
    COMPONENTS Core Gui WebSockets Widgets
    REQUIRED)

add_executable(${PROJECT_NAME} main.cpp chatwindow.cpp)

target_include_directories(
    ${PROJECT_NAME}
    PRIVATE $<BUILD_INTERFACE:${CFLIB_SUPERPROJECT_SOURCE_DIR}>
    PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE Qt5::Core
    PRIVATE Qt5::Gui
    PRIVATE Qt5::WebSockets
    PRIVATE Qt5::Widgets
    PRIVATE cflib::crypt
    PRIVATE cflib::net
    PRIVATE cflib::serialize
    PRIVATE cflib::util)

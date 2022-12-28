project(
    migrationmoped
    VERSION ${CFLIB_SUPERPROJECT_VERSION}
    LANGUAGES CXX)

find_package(
    Qt5
    COMPONENTS Core
    REQUIRED)

add_executable(${PROJECT_NAME} main.cpp)

target_include_directories(${PROJECT_NAME} PRIVATE $<BUILD_INTERFACE:${CFLIB_SUPERPROJECT_SOURCE_DIR}>)

target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE Qt5::Core
    PRIVATE cflib_db
    PRIVATE cflib_util)

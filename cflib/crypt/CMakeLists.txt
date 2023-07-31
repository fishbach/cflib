project(
    cflib_crypt
    VERSION ${CFLIB_SUPERPROJECT_VERSION}
    LANGUAGES CXX)

find_package(
    Qt5
    COMPONENTS Core
    REQUIRED)

find_package(PkgConfig REQUIRED)

pkg_check_modules(Botan2 REQUIRED IMPORTED_TARGET botan-2)

add_library(${PROJECT_NAME} tlsclient.cpp tlscredentials.cpp tlsserver.cpp tlssessions.cpp util.cpp)
add_library(cflib::crypt ALIAS ${PROJECT_NAME})

target_include_directories(
    ${PROJECT_NAME}
    PRIVATE $<BUILD_INTERFACE:${CFLIB_SUPERPROJECT_SOURCE_DIR}>
    INTERFACE $<BUILD_INTERFACE:${CFLIB_SUPERPROJECT_SOURCE_DIR}>)

target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE Qt5::Core
    PRIVATE PkgConfig::Botan2
    PRIVATE cflib::util)

add_subdirectory(crypt_test)
add_subdirectory(pwhash)

project(cflib_util VERSION ${CFLIB_SUPERPROJECT_VERSION} LANGUAGES CXX)

find_package(Qt5 COMPONENTS Core REQUIRED)
find_package(ZLIB REQUIRED)

add_library(${PROJECT_NAME}
    cmdline.cpp
    evtimer.cpp
    log.cpp
    libev.cpp
    log.cpp
    mailer.cpp
    test.cpp
    threadverify.cpp
    timer.cpp
    timeshifter.cpp
    unixsignal.cpp
    util.cpp
    impl/threadverifyimpl.cpp
)

target_include_directories(${PROJECT_NAME}
    PRIVATE $<BUILD_INTERFACE:${CFLIB_SUPERPROJECT_SOURCE_DIR}>
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt5::Core
    PRIVATE ZLIB::ZLIB
)

add_subdirectory(filefinder)
add_subdirectory(gitversion)
add_subdirectory(jscombiner)
add_subdirectory(util_test)

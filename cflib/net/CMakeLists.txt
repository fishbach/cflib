project(
    cflib_net
    VERSION ${CFLIB_SUPERPROJECT_VERSION}
    LANGUAGES CXX)

find_package(
    Qt5
    COMPONENTS Core
    REQUIRED)

include(SerializableGenerator)

add_library(
    ${PROJECT_NAME}
    impl/kafkaconnection.cpp
    impl/kafkaconnectorimpl.cpp
    impl/kafkafetchconnection.cpp
    impl/kafkagroupconnection.cpp
    impl/kafkametadataconnection.cpp
    impl/kafkaoffsetconnection.cpp
    impl/kafkaproduceconnection.cpp
    impl/requestparser.cpp
    impl/rmiserverbase.cpp
    impl/tcpconndata.cpp
    impl/tcpmanagerimpl.cpp
    impl/tlsthread.cpp
    impl/httpthread.cpp
    dns.cpp
    fileserver.cpp
    httpauth.cpp
    httpclient.cpp
    httprequest.cpp
    httpserver.cpp
    kafkaconnector.cpp
    logservice.cpp
    redirectserver.cpp
    request.cpp
    requestlog.cpp
    rmiservice.cpp
    rsig.cpp
    tcpconn.cpp
    tcpmanager.cpp
    websocketservice.cpp
    wscommmanager.cpp)
add_library(cflib::net ALIAS ${PROJECT_NAME})

generate_implementations_for(TARGET ${PROJECT_NAME})

target_include_directories(
    ${PROJECT_NAME}
    PRIVATE $<BUILD_INTERFACE:${CFLIB_SUPERPROJECT_SOURCE_DIR}>
    INTERFACE $<BUILD_INTERFACE:${CFLIB_SUPERPROJECT_SOURCE_DIR}>)

target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Core)

add_subdirectory(net_test)

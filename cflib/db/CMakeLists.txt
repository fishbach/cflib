project(
    cflib_db
    VERSION ${CFLIB_SUPERPROJECT_VERSION}
    LANGUAGES CXX)

find_package(
    Qt5
    COMPONENTS Core
    REQUIRED)
find_package(
    Qt5
    COMPONENTS Sql
    REQUIRED)
find_package(PostgreSQL REQUIRED)

add_library(${PROJECT_NAME} db.cpp dbconfig.cpp dbconfigpsql.cpp psql.cpp schema.cpp)

target_include_directories(${PROJECT_NAME} PRIVATE $<BUILD_INTERFACE:${CFLIB_SUPERPROJECT_SOURCE_DIR}>)

target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE Qt5::Core
    PRIVATE Qt5::Sql
    PRIVATE PostgreSQL::PostgreSQL)

add_subdirectory(db_test)
add_subdirectory(migrationmoped)

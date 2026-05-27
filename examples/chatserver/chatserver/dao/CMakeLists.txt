cf_lib(chatserver_dao
    PUBLIC
        cflib_serialize
    ENABLE_SER
)

target_include_directories(chatserver_dao PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../..)

cf_lib(chatserver_services
    PUBLIC
        chatserver_dao
        cflib_net
    REMOTE chatserver
)
target_include_directories(chatserver_services PUBLIC .)
add_library(ChatServer::services ALIAS chatserver_services)

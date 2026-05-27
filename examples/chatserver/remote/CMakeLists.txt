cf_lib(chatserver_services
    PUBLIC chatserver_dao
    REMOTE chatserver
)
add_library(ChatServer::services ALIAS chatserver_services)

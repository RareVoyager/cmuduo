

add_executable(log_example
    log.cc
)

target_link_libraries(log_example 
    PRIVATE
        muduo::base 
)


add_executable(inet_example
    inetaddress.cc
)
target_link_libraries(inet_example 
    PRIVATE
        muduo::base
        muduo::net 
)

add_executable(server testserver.cc)
target_link_libraries(server muduo_net muduo_base)
target_include_directories(server PRIVATE
    ${PROJECT_SOURCE_DIR}/muduo/base
    ${PROJECT_SOURCE_DIR}/muduo/net
)
add_executable(game game.cc)
target_link_libraries(game muduo_net muduo_base)

target_include_directories(game PRIVATE
    ${PROJECT_SOURCE_DIR}/muduo/base
    ${PROJECT_SOURCE_DIR}/muduo/net
)
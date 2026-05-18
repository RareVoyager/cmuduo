

add_executable(log_example
    log.cc
)

add_executable(inet_example
    inetaddress.cc
)

add_executable(server testserver.cc)

target_link_libraries(log_example 
    PRIVATE
        muduo::base 
)

target_link_libraries(server muduo_net muduo_base)

target_link_libraries(inet_example 
    PRIVATE
        muduo::base
        muduo::net 
)

target_include_directories(server PRIVATE
    ${PROJECT_SOURCE_DIR}/muduo/base
    ${PROJECT_SOURCE_DIR}/muduo/net
)
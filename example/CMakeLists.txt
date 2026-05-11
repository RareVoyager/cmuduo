

add_executable(log_example
    log.cc
)

add_executable(inet_example
    inetaddress.cc
)

target_link_libraries(log_example 
    PRIVATE
        muduo::base 
)

target_link_libraries(inet_example 
    PRIVATE
        muduo::base 
)
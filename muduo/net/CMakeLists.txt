file(GLOB_RECURSE MUDUO_NET_SOURCES
    CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cc
)

add_library(muduo_net
    ${MUDUO_NET_SOURCES}
)

target_include_directories(muduo_net
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(muduo_net
    PUBLIC
        muduo::base
)

target_compile_features(muduo_net
    PUBLIC
        cxx_std_11
)

add_library(muduo::net ALIAS muduo_net)
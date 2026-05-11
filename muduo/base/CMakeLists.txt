# 遍历 base/src 下的所有 .cc 文件
file(GLOB_RECURSE MUDUO_BASE_SOURCES
    CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cc
)

add_library(muduo_base
    ${MUDUO_BASE_SOURCES}
)

target_include_directories(muduo_base
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(muduo_base
    PUBLIC
        cxx_std_11
)

add_library(muduo::base ALIAS muduo_base)
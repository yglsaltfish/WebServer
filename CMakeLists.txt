cmake_minimum_required(VERSION 2.6)
set(CMAKE_C_COMPILER  "/usr/bin/clang")
set(CMAKE_CXX_COMPILER  "/usr/bin/clang++")

project(WebServer)

#add_library(block_queue SHARED log/block_queue.cc)


add_executable(${PROJECT_NAME}
    test/test.cc
)

#target_link_libraries(${PROJECT_NAME} block_queue)
target_include_directories(${PROJECT_NAME}
        PRIVATE
        ${PROJECT_SOURCE_DIR}/lock
        ${PROJECT_SOURCE_DIR}/log
        )



cmake_minimum_required(VERSION 2.6)
set(CMAKE_C_COMPILER  "/usr/bin/clang")
set(CMAKE_CXX_COMPILER  "/usr/bin/clang++")

project(WebServer)

#add_library(block_queue SHARED log/block_queue.cc)
add_compile_options(-o2)


add_executable(${PROJECT_NAME}
    test/test.cc
    log/log.cc
    CGImysql/sql_connection.cc
    timer/lst_timer.cc
    http/http_conn.cc
    Webserver/webserver.cc
    config/config.cc
)



target_include_directories(${PROJECT_NAME}
        PRIVATE
        ${PROJECT_SOURCE_DIR}/lock
        ${PROJECT_SOURCE_DIR}/log
        ${PROJECT_SOURCE_DIR}/CGImysql
        ${PROJECT_SOURCE_DIR}/threadpool
        ${PROJECT_SOURCE_DIR}/timer
        ${PROJECT_SOURCE_DIR}/http
        ${PROJECT_SOURCE_DIR}/Webserver
        ${PROJECT_SOURCE_DIR}/config
)



set(MYSQL_LIBS
    mysqlclient pthread z m rt atomic ssl crypto dl
)


target_link_libraries(${PROJECT_NAME}  ${MYSQL_LIBS})



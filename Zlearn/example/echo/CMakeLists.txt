add_executable(echo main.cc echo.cc)

target_link_libraries(echo muduo_base)
target_link_libraries(echo muduo_net)
target_link_libraries(echo pthread)
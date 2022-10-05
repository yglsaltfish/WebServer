add_executable(finger finger01.cc)

target_link_libraries(finger muduo_base)
target_link_libraries(finger muduo_net)
target_link_libraries(finger pthread)
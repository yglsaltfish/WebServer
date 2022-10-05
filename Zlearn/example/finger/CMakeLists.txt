add_executable(finger finger07.cc)


target_link_libraries(
    finger
    -Wl,--start-group
    muduo_base
    muduo_net
    pthread
    -Wl,--end-group
)


add_executable(Timestamp_test Timestamp_unittest.cc)

target_link_libraries(
    Timestamp_test
    -Wl,--start-group
    muduo_base
    muduo_net
    pthread
    -Wl,--end-group
)


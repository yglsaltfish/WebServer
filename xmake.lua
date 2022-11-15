target("WebServer")
    set_targetdir("./bin")
    set_kind("binary")
    add_files("*test/*.cpp")
    add_files("CGImysql/*.cc", "http/*.cc", "log/*.cc", "timer/*.cc", "config/*.cc" ,"Webserver/*.cc")
    add_headerfiles("CGImysql/*.h", "http/*.h", "lock/*.h", "log/*.h", "threadpool/*.h", "timer/*.h", "Webserver/*.cc")
    add_syslinks("pthread", "mysqlclient")  


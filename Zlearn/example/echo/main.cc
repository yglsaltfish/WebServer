#include "echo.h"

#include <muduo/base/Logging.h>
#include <muduo/net/EventLoop.h>

#include <unistd.h>

const int port = 10001;

int main()
{
    // 1.打印进程ID
    LOG_INFO << "pid = " << getpid();

    // 2.初始化EventLoop、InetAddress对象,
    muduo::net::EventLoop loop;
    const muduo::net::InetAddress listenAddr(2007);

    // 3.创建EchoServer, 启动服务
    EchoServer server(&loop, listenAddr);
    server.start();

    // 4.事件循环
    loop.loop();
}
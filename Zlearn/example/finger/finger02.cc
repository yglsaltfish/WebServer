#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpServer.h>

//接受新连接

int main()
{
    muduo::net::EventLoop loop;
    muduo::net::TcpServer server(&loop,muduo::net::InetAddress(2007),"Finger");
    server.start();
    loop.loop();
}
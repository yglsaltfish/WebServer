#include <muduo/net/EventLoop.h>

//拒绝连接

int main()
{
    muduo::net::EventLoop loop;
    loop.loop();
}
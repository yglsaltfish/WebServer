// finger01.cc
#include "muduo/net/EventLoop.h"

//using namespace muduo;
//using namespace muduo::net;

int main()
{
    muduo::net::EventLoop loop;
    loop.loop();
}
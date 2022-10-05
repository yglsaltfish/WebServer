#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpServer.h>

//断开连接

void onConnection(const muduo::net::TcpConnectionPtr& conn)
{
    if(conn->connected())
        conn->shutdown();
}

int main()
{
    muduo::net::EventLoop loop;
    muduo::net::TcpServer server(&loop, muduo::net::InetAddress(2007), "Finger");
    server.setConnectionCallback(onConnection);
    server.start();
    loop.loop();
}
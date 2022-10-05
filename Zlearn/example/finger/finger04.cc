#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpServer.h>

//读取用户，然后断开连接

void onMessage(const muduo::net::TcpConnectionPtr& conn,muduo::net::Buffer *buf,muduo::Timestamp receiveTime)
{
    if(buf->findCRLF())
        conn->shutdown();
}

int main()
{
    muduo::net::EventLoop loop;
    muduo::net::TcpServer server(&loop, muduo::net::InetAddress(2007), "Finger");
    server.setMessageCallback(onMessage);
    server.start();
    loop.loop();
}
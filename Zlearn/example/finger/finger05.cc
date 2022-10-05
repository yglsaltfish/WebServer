#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpServer.h>

//读取用户名，输出错误信息，断开连接

void onMessage(const muduo::net::TcpConnectionPtr& conn,muduo::net::Buffer *buf,muduo::Timestamp receiveTime)
{
    if(buf->findCRLF())
    {
        conn->send("No such user\r\n");
        conn->shutdown();
    }
        
}

int main()
{
    muduo::net::EventLoop loop;
    muduo::net::TcpServer server(&loop, muduo::net::InetAddress(2007), "Finger");
    server.setMessageCallback(onMessage);
    server.start();
    loop.loop();
}
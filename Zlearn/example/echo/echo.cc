#include "echo.h"
#include <muduo/base/Logging.h>

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

EchoServer::EchoServer(muduo::net::EventLoop *loop, const muduo::net::InetAddress &listenAddr):server_(loop,listenAddr,"EchoServer")
{
    server_.setConnectionCallback(std::bind(&EchoServer::onConnection,this,_1));
    server_.setMessageCallback(std::bind(&EchoServer::onMessage,this,_1,_2,_3));
}

void::EchoServer::start()
{
    server_.start();
}


void EchoServer::onConnection(const muduo::net::TcpConnectionPtr& conn)
{
    // perrAddress(): 返回对方地址(以InetAddress对象表示IP和port)
    // localAddress(): 返回本地地址(以InetAddress对象表示IP和port)
    // connected()：返回bool值, 表明目前连接是建立还是断开

    LOG_INFO<<"EchoServer - "<< conn->peerAddress().toIpPort()<<"->"<<conn->localAddress().toIpPort()<<" is "<<(conn->connected()?"up":"down");
}

void EchoServer::onMessage(const muduo::net::TcpConnectionPtr& conn,muduo::net::Buffer* buf,muduo::Timestamp time)
{
    muduo::string msg(buf->retrieveAllAsString());

    LOG_INFO << conn->name() << " echo " << msg.size() << " bytes, "<<"data received at : "<< time.toString();
    conn->send(msg);
}
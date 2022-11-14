#pragma once

#include <mysql/mysql.h>
#include <iostream>
#include <error.h>
#include <string.h>
#include <list>
#include "../lock/locker.h"
#include "../log/log.h"

using namespace std;

class connection_pool
{
public:
    MYSQL * GetConnection();        //获取数据库连接
    bool ReleaseConnection(MYSQL *con); //释放连接
    int GetFreeConn();              //获取连接
    void DestroyPool();             //销毁连接

    static connection_pool *GetInstance();
    void init(string url, string User,  string Password, string DataBaseName, int Port, int MaxConn, int close_log);

private:
    connection_pool();
    ~connection_pool();

    
    int m_MaxConn;
    int m_CurConn;
    int m_FreeConn;
    locker lock;
    list<MYSQL *> connList;
    sem reserve;


public:
    string m_url;
    string m_Port;
    string m_User;
    string m_Password;
    string m_DataBaseNmae;
    int m_close_log;


};

class ConnectionRAII
{
public:
    ConnectionRAII(MYSQL ** con, connection_pool *connpool);
    ~ConnectionRAII();


private:
    MYSQL * connRAII;
    connection_pool *pollRAII;
};
#include "sql_connection.h"

connection_pool::connection_pool()
{
    m_CurConn = 0;
    m_FreeConn = 0 ;
}

connection_pool::~connection_pool()
{
    DestroyPool();
}

MYSQL *connection_pool::GetConnection() //获取数据库连接
{
    MYSQL *con = NULL;
    if(0 == connList.size())
        return NULL;
    reserve.wait();
    lock.lock();
    con = connList.front();
    connList.pop_front();
    --m_FreeConn;
    ++m_CurConn;
    lock.unlock();
    return con;
}

bool connection_pool::ReleaseConnection(MYSQL * con) //释放连接
{
    if(NULL == con)
        return false;
    lock.lock();

    connList.push_back(con);
    ++m_FreeConn;
    --m_CurConn;
    reserve.post();
    return true;
}

int connection_pool::GetFreeConn() //获取连接
{
    return this->m_FreeConn;


}

void connection_pool::DestroyPool() //销毁连接
{
    lock.lock();
    if(connList.size() > 0)
    {
        for(auto &it : connList)
            mysql_close(it);
        m_CurConn = 0;
        m_FreeConn = 0;
        connList.clear();
    }
    lock.unlock();
}

connection_pool *connection_pool::GetInstance()
{
    static connection_pool connPool;
    return &connPool;
}

void connection_pool::init(string url, string User, string Password, string DataBaseName, int Port, int MaxConn, int close_log)
{
    m_url = url;
    m_Port = Port;
    m_User = User;
    m_Password = Password;
    m_DataBaseNmae = DataBaseName;
    m_close_log = close_log;

    for(int i = 0;i < MaxConn; i++)
    {
        MYSQL *con = NULL;
        con = mysql_init(con);

        if(con == nullptr)
        {
            LOG_ERROR("MySql Error");
            exit(1);
        }
        con = mysql_real_connect(con, m_url.c_str(), m_User.c_str(), m_Password.c_str(), m_DataBaseNmae.c_str(), Port, NULL, 0);
        if(con == NULL)
        {
            LOG_ERROR("MYSQL Error");
            exit(1);
        }
        connList.push_back(con);
        ++m_FreeConn;
    }
    reserve = sem(m_FreeConn);
    m_MaxConn = m_FreeConn;

}

ConnectionRAII::ConnectionRAII(MYSQL **SQL, connection_pool *connPool)
{
    *SQL = connPool->GetConnection();

    connRAII = *SQL;
    pollRAII = connPool;
}

ConnectionRAII::~ConnectionRAII()
{
    pollRAII->ReleaseConnection(connRAII);
}
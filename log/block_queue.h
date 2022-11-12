#pragma once

#include <iostream>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include "../lock/locker.h"

template<typename T>
class block_queue
{
public:
    block_queue(int num);
    ~block_queue();
    
    void clear();
    bool full();
    bool empty();
    bool front(T &value);
    bool back(T & value);
    int size();
    int max_size();
    bool push(const T&item);
    bool pop(T &item);

private:
    locker m_mutex;
    cond m_cond;
    T *m_array;
    int m_size;
    int m_max_size;
    int m_front;
    int m_back;
};

#include "block_queue.h"

template <typename T>
block_queue<T>::block_queue(int max_size)
{
    if (max_size <= 0)
        exit(-1);
    m_max_size = max_size;
    m_array = new T[max_size];
    m_size = 0;
    m_front = -1;
    m_back = -1;
}

template <typename T>
block_queue<T>::~block_queue()
{
    m_mutex.lock();
    if (m_array != nullptr)
        delete[] m_array;
    m_mutex.unlock();
}

template <typename T>
void block_queue<T>::clear()
{
    m_mutex.lock();
    m_size = 0;
    m_front = -1;
    m_back = -1;
    m_mutex.unlock();
}

template <typename T>
bool block_queue<T>::full()
{
    m_mutex.lock();
    if (m_size >= m_max_size)
    {
        m_mutex.unlock();
        return true;
    }
    m_mutex.unlock();
    return false;
}

template <typename T>
bool block_queue<T>::empty()
{
    m_mutex.lock();
    if (m_size == 0)
    {
        m_mutex.unlock();
        return true;
    }
    m_mutex.unlock();
    return false;
}

template <typename T>
bool block_queue<T>::front(T &value)
{
    m_mutex.lock();
    if (m_size == 0)
    {
        m_mutex.unlock();
        return false;
    }
    value = m_array[m_front];
    m_mutex.unlock();
    return true;
}
template <typename T>
bool block_queue<T>::back(T &value)
{
    m_mutex.lock();
    if (0 == m_size)
    {
        m_mutex.unlock();
        return false;
    }
    value = m_array[m_back];
    m_mutex.unlock();
    return true;
}

template <typename T>
int block_queue<T>::size()
{
    m_mutex.lock();
    int tmp = m_size;
    m_mutex.unlock();
    return tmp;
}

template <typename T>
int block_queue<T>::max_size()
{
    m_mutex.lock();
    int tmp = m_max_size;
    m_mutex.unlock();
    return tmp;
}

template <typename T>
bool block_queue<T>::push(const T &item)
{
    m_mutex.lock();
    if (m_size >= m_max_size)
    {

        m_cond.signal();
        m_mutex.unlock();
        return false;
    }

    m_back = (m_back + 1) % m_max_size;
    m_array[m_back] = item;

    m_size++;

    m_cond.signal();
    m_mutex.unlock();
    return true;
}

template <typename T>
bool block_queue<T>::pop(T &item)
{

    m_mutex.lock();
    while (m_size <= 0)
    {
        if (!m_cond.wait(m_mutex.get()))
        {
            m_mutex.unlock();
            return false;
        }
    }

    m_front = (m_front + 1) % m_max_size;
    item = m_array[m_front];
    m_size--;
    m_mutex.unlock();
    return true;
}

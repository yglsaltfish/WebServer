#ifndef BASE_CURRENTTHREAD_H
#define BASE_CURRENTTHREAD_H

#include "base/noncopyable.h"
#include "base/Types.h"



namespace Zlearn
{
namespace CurrentThread
{
/*
__thread 关键字: 如果想要每个线程都有一份独立的数据，使用__thread关键字修饰数据。
只能用于修饰POD类型(plain old data)的数据
*/
 extern __thread int t_cachedTId;  // 线程的真实ID
 extern __thread char t_tidString[32];  // string 类型表示tid
 extern __thread int t_tidStringLength; //string 类型tid 的长度
 extern __thread const char* t_threadName; //线程的名字
 void cacheTid();

 inline int tid()
 {
    if (__builtin_expect(t_cachedTId==0, 0))
    {
      cacheTid(); 
    }
    return t_cachedTId;
 }

 inline const char* tidString() //记录日志
 {
    return t_tidString;
 }

 inline int tidStringLength() //记录日志
 {
    return t_tidStringLength;
 }
 
 inline const char *name()
 {
     return t_threadName;
 }

 bool isMainThread();

 void sleepUsec(int64_t usec);//测试用

 string stackTrace(bool demangle);
}

}

#endif // BASE_CURRENTTHREAD_H
# 1. 记录我实现WebServer
reactor/procatar模式均有。
我是一步一步实现的，按照`lock -> threadpool -> webserver -> http 信息处理 -> 日志处理 -> 数据库池连接-> 处理非活跃连接` 这个顺序一步一步实现的

# 2 实现
## 2.1 I/O 模型
* 同步IO：内核向应用程序通知的是就绪事件
  * 阻塞IO：调用者调用了某个函数，等待这个函数返回，期间什么也不做，不停的去检查这个函数有没有返回，必须等这个函数返回才能进行下一步动作
  * 非阻塞IO：非阻塞等待，每隔一段时间就去检测IO事件是否就绪。没有就绪就可以做其他事。非阻塞I/O执行系统调用总是立即返回，不管事件是否已经发生，若事件没有发生，则返回-1，此时可以根据errno区分这两种情况，对于accept，recv和send，事件未发生时，errno通常被设置成eagain
  * 信号驱动IO：linux用套接口进行信号驱动IO，安装一个信号处理函数，进程继续运行并不阻塞，当IO时间就绪，进程收到SIGIO信号。然后处理IO事件。
  * IO复用：linux用select/poll函数实现IO复用模型，这两个函数也会使进程阻塞，但是和阻塞IO所不同的是这两个函数可以同时阻塞多个IO操作。而且可以同时对多个读操作、写操作的IO函数进行检测。知道有数据可读或可写时，才真正调用IO操作函数
* 异步IO：linux中，可以调用aio_read函数告诉内核描述字缓冲区指针和缓冲区的大小、文件偏移及通知的方式，然后立即返回，当内核将数据拷贝到缓冲区后，再通知应用程序。


## 2.1 reactor 模式如下实现：
* 主线程：
    1. 监听和建立客户端的连接；
    2. 接收客户端的请求，创建一个任务，并把该任务放入任务队列；
    3. 通知分发线程。
* 分发线程
    1. 查看任务队列，看是否有请求任务？没有任务则继续睡觉，否则把任务取出来，然后分发给线程池；
    2. 线程池有空闲的线程，则把该任务交给空闲的线程处理，否则等待。
* 工作线程
    1. 执行任务
    2. 销毁任务

1、主线程往epoll 内核事件表中注册socket上的读就绪事件。

2、主线程调用epoll_wait等待socket上有数据可读。

3、当socket 上有数据可读时，epoll_wait通知主线程。主线程则将socket可读事件放入请求队列。

4、睡眠在请求队列上的某个工作线程被唤醒，它从socket读取数据，并处理客户请求，然后往epoll内核事件表中注册该socket上的写就绪事件。

5、主线程调用epoll_wait等待socket可写。

6、当socket可写时，epoll_wait通知主线程。主线程将socket可写事件放入请求队列。

7、睡眠在请求队列上的某个工作线程被唤醒，它往socket上写入服务器处理客户请求的结果。

## 2.2 procator
使用异步I/O模型（以aio_read和 aio_write为例）实现的Proactor模式的工作流程是:

1、主线程调用aio_read函数向内核注册socket上的读完成事件，并告诉内核用户读缓冲区的位置，以及读操作完成时如何通知应用程序（这里以信号为例，详情请参考sigevent的man手册)。

2、主线程继续处理其他逻辑。

3、当socket 上的数据被读入用户缓冲区后，内核将向应用程序发送一个信号，以通知应用程序数据已经可用。

4、应用程序预先定义好的信号处理函数选择一个工作线程来处理客户请求。工作线程处理完客户请求之后，调用aio_write函数向内核注册socket 上的写完成事件，并告诉内核用户写缓冲区的位置，以及写操作完成时如何通知应用程序（仍然以信号为例)。

5、主线程继续处理其他逻辑。

6、当用户缓冲区的数据被写人socket之后，内核将向应用程序发送一个信号，以通知应用程序数据已经发送完毕。

7、应用程序预先定义好的信号处理函数选择一-个工作线程来做善后处理，比如决定是否关闭socket。


procatar 模式可以使用reactor模式模拟的：

1、主线程往epoll内核事件表中注册socket 上的读就绪事件。

2、主线程调用epoll_wait等待socket上有数据可读。

3、当socket 上有数据可读时，epoll_wait通知主线程。主线程从socket循环读取数据，直到没有更多数据可读，然后将读取到的数据封装成一个请求对象并插人请求队列。

4、睡眠在请求队列上的某个工作线程被唤醒，它获得请求对象并处理客户请求，然后往epoll 内核事件表中注册socket上的写就绪事件

5、主线程调用epoll_wait等待socket可写。

6）当socket可写时，epoll_wait通知主线程。主线程往socket上写人服务器处理客户请求的结果。


总的来说：
* reactor模式中，主线程(I/O处理单元)只负责监听文件描述符上是否有事件发生，有的话立即通知工作线程(逻辑单元)，读写数据、接受新连接及处理客户请求均在工作线程中完成。通常由同步I/O实现。
* proactor模式中，主线程和内核负责处理读写数据、接受新连接等I/O操作，工作线程仅负责业务逻辑，如处理客户请求。通常由异步I/O实现。


## 2.3 多线程相关实现
如果要用多线程，需要使用一些使得线程同步的机制

## 2.3.1 信号量
信号量：信号量是一种特殊的变量，它只能取自然数值并且只支持两种操作：等待(P)和信号(V).假设有信号量SV，对其的P、V操作如下：
* P：如果SV的值大于0，则将其减一；若SV的值为0，则挂起执行
* V：如果有其他进行因为等待SV而挂起，则唤醒；若没有，则将SV值加一

如下就是信号量的一些api:
* sem_init函数用于初始化一个未命名的信号量
* sem_destory函数用于销毁信号量
* sem_wait函数将以原子操作方式将信号量减一,信号量为0时,sem_wait阻塞
* sem_post函数以原子操作方式将信号量加一,信号量大于0时,唤醒调用sem_post的线程

## 2.3.2 互斥量
互斥锁：可以保护关键代码段，以确保独占式访问.当进入关键代码段，获得互斥锁将其加锁；离开关键代码段，唤醒等待该互斥锁的线程。

以下就是相关api
* pthread_mutex_init函数用于初始化互斥锁
* pthread_mutex_destory函数用于销毁互斥锁
* pthread_mutex_lock函数以原子操作方式给互斥锁加锁
* pthread_mutex_unlock函数以原子操作方式给互斥锁解锁

## 2.3.3 条件变量
条件变量：条件变量提供了一种线程间的通知机制,当某个共享数据达到某个值时,唤醒等待这个共享数据的线程.

以下就是相关api：
* pthread_cond_init函数用于初始化条件变量
* pthread_cond_destory函数销毁条件变量
* pthread_cond_broadcast函数以广播的方式唤醒所有等待目标条件变量的线程
* pthread_cond_wait函数用于等待目标条件变量。该函数调用时需要传入 mutex参数(加锁的互斥锁) ,函数执行时，先把调用线程放入条件变量的请求队列，然后将互斥锁mutex解锁，当函数成功返回为0时，互斥锁会再次被锁上. 也就是说函数内部会有一次解锁和加锁操作。

## 2.4 WebServer实现
主要使用套接字，套接字有一些使用的比较多的api和参数，在这里写一下：

### 2.4.1 epoll相关

#### 相关函数
* `epoll_create() -> int`：创建一个epoll句柄，该函数返回一个非负整数作为epoll句柄，如果返回-1则表示创建失败。

* `epoll_ctl(int epfd, int op, int fd, struct epoll_event *event) -> int`:
  * `epfd`：为epoll_creat的句柄
  * `op`：表示动作，用3个宏来表示：
    * `EPOLL_CTL_ADD` (注册新的fd到epfd)，
    * `EPOLL_CTL_MOD` (修改已经注册的fd的监听事件)，
    * `EPOLL_CTL_DEL` (从epfd删除一个fd)；
  * `event`：告诉内核需要监听的事件
    * EPOLLIN：表示对应的文件描述符可以读（包括对端SOCKET正常关闭）
    * `EPOLLOUT`：表示对应的文件描述符可以写
    * `EPOLLPRI`：表示对应的文件描述符有紧急的数据可读（这里应该表示有带外数据到来）
    * `EPOLLERR`：表示对应的文件描述符发生错误
    * `EPOLLHUP`：表示对应的文件描述符被挂断；
    * `EPOLLET`：将EPOLL设为边缘触发(Edge Triggered)模式，这是相对于水平触发(Level Triggered)而言的
    * `EPOLLONESHOT`：只监听一次事件，当监听完这次事件之后，如果还需要继续监听这个socket的话，需要再次把这个socket加入到EPOLL队列里
* `epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout) -> int`:
  * `maxevents`: 表示events事件大小
  * `timeout`：超时时间，`-1：阻塞； 0：立即返回，非阻塞；>0 : 指定时间`

#### 相关重要参数
ET、LT、EPOLLONESHOT
* LT水平触发模式
  * epoll_wait检测到文件描述符有事件发生，则将其通知给应用程序，应用程序可以不立即处理该事件。
  * 当下一次调用epoll_wait时，epoll_wait还会再次向应用程序报告此事件，直至被处理
* ET边缘触发模式
  * epoll_wait检测到文件描述符有事件发生，则将其通知给应用程序，应用程序必须立即处理该事件
  * 必须要一次性将数据读取完，使用非阻塞I/O，读取到出现eagain
* EPOLLONESHOT
  * 解决问题：一个线程读取某个socket上的数据后开始处理数据，在处理过程中该socket上又有新数据可读，此时另一个线程被唤醒读取，此时出现两个线程处理同一个socket
  * 通过epoll_ctl对该文件描述符注册epolloneshot事件，一个线程处理socket时，其他线程将无法处理，当该线程处理完后，需要通过epoll_ctl重置epolloneshot事件

### 2.4.2 管道相关
在这部分实现中，我还用到了管道。
* `int socketpair(int domain, int type, int protocol, int sv[2]);` 
  * domain 表示套接字的域（地址族），通常是 AF_UNIX 或 AF_LOCAL。
  * type 表示套接字的类型，通常是 SOCK_STREAM 或 SOCK_DGRAM。
  * protocol 表示套接字使用的协议，通常为 0（默认）。
  * sv 是一个长度为 2 的整型数组，用于存放创建的一对套接字的文件描述符。

在这里管道主要用来接受信号，由于之后要使用定时器来关闭不常使用的连接，定时器发送的信号就用管道来接受。


## 2.5 HTTP信息处理相关

### 2.5.1 报文格式

* 请求报文 ：请求行（request line）、请求头部（header）、空行和请求数据四个部分组成。
```html
GET /562f25980001b1b106000338.jpg HTTP/1.1
Host:xxxx.com
User-Agent:Mozilla/5.0 (Windows NT 10.0; WOW64)
AppleWebKit/537.36 (KHTML, like Gecko) Chrome/51.0.2704.106 Safari/537.36
Accept:image/webp,image/*,*/*;q=0.8
Referer:http://www.imooc.com/
Accept-Encoding:gzip, deflate, sdch
Accept-Language:zh-CN,zh;q=0.8
空行
请求数据为空


POST / HTTP1.1
Host:www.xxxxxxxx.com
User-Agent:Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1; SV1; .NET CLR 2.0.50727; .NET CLR 3.0.04506.648; .NET CLR 3.5.21022)
Content-Length:40
Connection: Keep-Alive
空行
name=Professional%20Ajax&publisher=Wiley
```

* 响应报文：HTTP响应也由四个部分组成，分别是：状态行、消息报头、空行和响应正文。

### 2.5.2 状态码
HTTP有5种类型的状态码，具体的：
* 1xx：指示信息--表示请求已接收，继续处理。
* 2xx：成功--表示请求正常处理完毕。
  * 200 OK：客户端请求被正常处理。
  * 206 Partial content：客户端进行了范围请求。
* 3xx：重定向--要完成请求必须进行更进一步的操作
  * 301 Moved Permanently：永久重定向，该资源已被永久移动到新位置，将来任何对该资源的访问都要使用本响应返回的若干个URI之一。
  * 302 Found：临时重定向，请求的资源现在临时从不同的URI中获得
* 4xx：客户端错误--请求有语法错误，服务器无法处理请求。
  * 400 Bad Request：请求报文存在语法错误。
  * 403 Forbidden：请求被服务器拒绝。
  * 404 Not Found：请求不存在，服务器上找不到请求的资源
* 5xx：服务器端错误--服务器处理请求出错。
  * 500 Internal Server Error：服务器在执行请求时出现错误。


### 2.5.3 有限状态机(DFA)
有限状态机，是一种抽象的理论模型，它能够把有限个变量描述的状态变化过程，以可构造可验证的方式呈现出来。
在这里使用有限状态机是因为分类讨论代码会写的比较复杂，并且极容易丢失细节和处理错误。
处理HTTP请求过程分为主状态机和从状态机：
主状态机有三种状态，表示要解析的东西：
* CHECK_STATE_REQUESTLINE，解析请求行
* CHECK_STATE_HEADER，解析请求头
* CHECK_STATE_CONTENT，解析消息体，仅用于解析POST请求

主状态机的逻辑：
主状态机初始状态是CHECK_STATE_REQUESTLINE，通过调用从状态机来驱动主状态机，在主状态机进行解析前，从状态机已经将每一行的末尾\r\n符号改为\0\0，以便于主状态机直接取出对应字符串进行处理。

* CHECK_STATE_REQUESTLINE
  * 主状态机的初始状态，调用parse_request_line函数解析请求行
  * 解析函数从m_read_buf中解析HTTP请求行，获得请求方法、目标URL及HTTP版本号
  * 解析完成后主状态机的状态变为CHECK_STATE_HEADER

解析完请求行后，主状态机继续分析请求头。
* CHECK_STATE_HEADER
  * 调用parse_headers函数解析请求头部信息
  * 判断是空行还是请求头，若是空行，进而判断content-length是否为0，如果不是0，表明是POST请求，则状态转移到CHECK_STATE_CONTENT，否则说明是GET请求，则报文解析结束。
  * 若解析的是请求头部字段，则主要分析connection字段，content-length字段，其他字段可以直接跳过。
  * connection字段判断是keep-alive还是close，决定是长连接还是短连接
  * content-length字段，这里用于读取post请求的消息体长度

从状态机有三种状态，表示解析的状态：
* LINE_OK，完整读取一行
* LINE_BAD，报文语法有误
* LINE_OPEN，读取的行不完整



### 2.5.4 `munmap()`、`mmap()`
 mmap()系统调用使得进程之间通过映射同一个普通文件实现共享内存。普通文件被映射到进程地址空间后，进程可以向访问普通内存一样对文件进行访问，不必再调用read()，write（）等操作。
在这里是为了提高访问速度，于是就把普通文件映射到逻辑地址。


## 2.6 日志处理
使用单例模式，这里我使用的单例模式做的日志系统。参考的《Effective C++》中的单例模式实现，这种方式不需要加锁和解锁。
```c++
class Singleton {
public:
    static Singleton& getInstance() {
        static Singleton instance;
        return instance;
    }
private:
    Singleton() {} // 禁止通过构造函数创建实例
    Singleton(const Singleton& other) = delete; // 禁止拷贝构造函数
    Singleton& operator=(const Singleton& other) = delete; // 禁止赋值运算符
};
```

由于使用的生产者-消费者模型，使用的前文提到的条件变量做支撑。

## 2.7 数据库池
在一个高并发的应用程序中，如果每次请求都建立一次数据库连接，会带来很大的性能问题，因为每次连接都需要系统创建数据库连接，完成数据库操作，然后系统断开数据库连接等操作，这些都需要占用系统资源和时间。而且频繁地建立和关闭连接还会对数据库服务器造成额外的压力。

在程序初始化的时候，集中创建多个数据库连接，并把他们集中管理，供程序使用，可以保证较快的数据库读写速度，更加安全可靠。

使用RAII技巧管理创建、销毁连接池；通过信号量来处理多线程争夺数据库连接的同步机制。
### 2.7.1 获取、释放连接
* 获取：当线程数量大于数据库连接数量时，使用信号量进行同步，每次取出连接，信号量原子减1，释放连接原子加1，若连接池内没有连接了，则阻塞等待。
* 销毁：通过迭代器遍历连接池链表，关闭对应数据库连接，清空链表并重置空闲连接和现有连接数量。


## 2.8 用定时器处理不活跃的连接
LINUX下有三种定时方式：
* socket选项SO_RECVTIMEO和SO_SNDTIMEO
* SIGALRM信号
* I/O复用系统调用的超时参数
我使用的`SIGALRM`信号方式，利用`alarm`函数周期性地触发`SIGALRM`信号，信号处理函数利用管道通知主循环，主循环接收到该信号后对升序链表上所有定时器进行处理，若该段时间内没有交换数据，则将该连接关闭，释放所占用的资源。
具体用到
* #define SIGALRM  14     //由alarm系统调用产生timer时钟信号
* #define SIGTERM  15     //终端发送的终止信号

### 2.8.1 信号处理机制
* 信号的接收
  * 接收信号的任务是由内核代理的，当内核接收到信号后，会将其放到对应进程的信号队列中，同时向进程发送一个中断，使其陷入内核态。注意，此时信号还只是在队列中，对进程来说暂时是不知道有信号到来的。

* 信号的检测
  * 进程从内核态返回到用户态前进行信号检测
  * 进程在内核态中，从睡眠状态被唤醒的时候进行信号检测
  * 进程陷入内核态后，有两种场景会对信号进行检测：
  * 当发现有新信号时，便会进入下一步，信号的处理。

* 信号的处理
  * ( 内核 )信号处理函数是运行在用户态的，调用处理函数前，内核会将当前内核栈的内容备份拷贝到用户栈上，并且修改指令寄存器（eip）将其指向信号处理函数。
  * ( 用户 )接下来进程返回到用户态中，执行相应的信号处理函数。
  * ( 内核 )信号处理函数执行完成后，还需要返回内核态，检查是否还有其它信号未处理。
  * ( 用户 )如果所有信号都处理完成，就会将内核栈恢复（从用户栈的备份拷贝回来），同时恢复指令寄存器（eip）将其指向中断前的运行位置，最后回到用户态继续执行进程。

### 2.8.2 超时设置
使用升序链表来处理超时连接
* 浏览器与服务器连接时，创建该连接对应的定时器，并将该定时器添加到链表上
* 处理异常事件时，执行定时事件，服务器关闭连接，从链表上移除对应定时器
* 处理定时信号时，将定时标志设置为true
* 处理读事件时，若某连接上发生读事件，将对应定时器向后移动，否则，执行定时事件
* 处理写事件时，若服务器通过某连接给浏览器发送数据，将对应定时器向后移动，否则，执行定时事件

# 3 调试
我是在ubuntu上用vscode进行开发的，在调试过程中发现了一些问题。可以在命令行中使用以下代码开启管理员模式进而调试。
* OS VERSION : Ubuntu 20.04.2 LTS
* VScode Version : 1.73.1
```
sudo code --no-sandbox --disable-gpu-sandbox --user-data-dir=/root/.vscode/
```

# 4 参考文献
1. https://github.com/qinguoyi/TinyWebServer
2. Linux高性能服务器编程，游双著.
3. chatGpt带给我的帮助



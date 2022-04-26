#ifndef SERVER_H
#define SERVER_H

#include<vector>
#include"../threadpool/threadpool.h"
#include<sys/epoll.h>
#include<sys/fcntl.h>
#include<sys/types.h>
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<sys/msg.h>
#include<signal.h>
#include<string.h>
#include<queue>
#include<sys/ipc.h>
#include"../uncopyable/uncopyable.h"
#include"../ipc/fifo.h"
#include"../ipc/msg.h"




#define BUF_SIZE 100
#define EPOLL_SIZE 1000

const int MAX_FD = 65536;           //最大文件描述符
const int MAX_EVENT_NUMBER = 10000; //最大事件数


class Server : public uncopyable
{
public:
    Server(char* port,char* publisherhost,char* brokerhoust)
    {
        m_msg = new Publisher();
        m_msg->init(publisherhost,brokerhoust);
        user = new MyThreadpoolData[MAX_FD];
        m_thread_pool_read = new threadpool<MyThreadpoolData>;
        m_thread_pool_write = new threadpool<MyThreadpoolData>;
        m_port = port;
    }
    ~Server();
private:
    MyThreadpoolData *user; //存客户端的描述符
    threadpool<MyThreadpoolData> *m_thread_pool_read;//线程池
    threadpool<MyThreadpoolData> *m_thread_pool_write;//线程池

    char * m_port;      //端口
    int serve_sock;     //服务器的套接子描述符
    struct sockaddr_in server_addr;     //bind

    /*
    m_msg：消息对象
    */
public:
    Publisher *m_msg;
private:   
    /*
    管道相关：
    m_pipe :管道对象
    fifo_treead：接受管道的线程
    fifoFunc:管道线程的回调函数
    Mkfifo:有两个任务 1 解包，2 加入epoll 下次触发可写
    */
    Pipe_m *m_pipe;
    pthread_t fifo_thread;
    static void* fifoFunc(void*);
    bool Mkfifo(int epollfd);

    epoll_event  ep_events[MAX_EVENT_NUMBER];   //接受epoll_wait
    int epollfd;    //epoll描述符

public:
    bool Listen();
    void eventLoop();
    bool Accept();
    void dealwithread(int fd);
    void dealwithwrite(int fd);
};

#endif
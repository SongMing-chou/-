#ifndef THREADPOOL_H
#define THREADPOOL_H

#include<list>
#include<cstdio>
#include<exception>
#include"../lock/locker.h"
#include<string>
#include"../threadpooldata/mythreadpooldata.h"

/**
 * @brief 线程池类
 * 
 * @tparam T 
 */

template <typename T>
class threadpool 
{
private:
    int m_max_requests;
    int m_max_thread_number;
    pthread_t *m_threads;
    std::list<T*>m_work_queue;
    locker m_queue_loker;
    sem m_queue_sem;
public:
    threadpool();
    ~threadpool();
    bool append(T*request,int state);
    void run();
private:
    static void*worker(void*arg);
};

#endif
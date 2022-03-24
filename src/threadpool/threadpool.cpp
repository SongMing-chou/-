#include"threadpool.h"
#include<unistd.h>

template<typename T>
threadpool<T>::threadpool()
                :m_max_thread_number(8),m_max_requests(10000),m_threads(NULL)
{
    if(m_max_thread_number <= 0 && m_max_requests <= 0) 
    {
        throw std::exception();
    }
    m_threads = new pthread_t[m_max_thread_number];
    if(!m_threads) 
    {
        throw std::exception();
    }
    for(int i = 0;i < m_max_thread_number;++i) 
    {
        if(pthread_create(m_threads + i,NULL,worker,this) != 0) 
        {
            delete[] m_threads;
            throw std::exception();
        }
        if(pthread_detach(m_threads[i])) 
        {
            delete[] m_threads;
            throw std::exception();
        }
    }
    printf("thread poll init sucess!!!\n");
}

template<typename T>
threadpool<T>::~threadpool() 
{
    delete[] m_threads;
}
/**
 * @brief 将任务添加到线程池的阻塞队列
 * 
 * @tparam T 
 * @param request 任务
 * @param state 状态区分读写
 * @return true 成功返回
 * @return false 调用失败，队列已满
 */
template<typename T>
bool threadpool<T>::append(T*request,int state) 
{
    m_queue_loker.lock();
    /*如果工作队列已经满了,解锁返回，将任务丢弃*/
    if(m_work_queue.size() >= m_max_requests) 
    {
        m_queue_loker.unlock();
        return false;
    }
    request->m_state = state;
    m_work_queue.push_back(request);
    //唤醒
    m_queue_sem.post();
    m_queue_loker.unlock();//记得解锁
    return true;
}

template<typename T>
void *threadpool<T>::worker(void*arg) 
{
    threadpool *pool = (threadpool*)arg;
    pool->run();
    return pool;
}

/**
 * @brief 任务循环，先获取信号量(休眠)，再取得锁
 * 
 * @tparam T 
 */
template <typename T>
void threadpool<T>::run() 
{
    // printf("threa is init and runing!!!\n");
    while(true) 
    {
        //等待信号量（休眠）
        m_queue_sem.wait();
        //对队列操作要加锁
        m_queue_loker.lock();
        // printf("get work!!!\n");
        if(m_work_queue.empty()) 
        {
            m_queue_loker.unlock();
            continue;
        }

        T* request = m_work_queue.front();
        m_work_queue.pop_front();
        m_queue_loker.unlock();

        if(!request) 
        {
            continue;
        }
        if(request->m_state == 0) 
        {
            if(request->read_once())
                request->process();
        }else if(request->m_state == 1) 
        {
            request->m_write();
        }
        //do samething !!!
    }
}

template class threadpool<MyThreadpoolData>;


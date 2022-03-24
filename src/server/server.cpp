#include"server.h"
#include<unordered_set>

using namespace std;



//记录客户端的文件描述符
unordered_set<int> set;

Server:: ~Server()
{
    delete[] user;
    delete m_pipe;
    delete m_thread_pool_read;
    delete m_thread_pool_write;
    for(auto i = set.begin();i != set.end();i++) 
    {
        close(*i);
    }
};

/**
 * @brief 创建管道并将其加入epoll监听
 * 
 * @param epollfd 
 * @return true 
 * @return false 
 */
bool Server::Mkfifo(int epollfd) 
{
    epoll_event event;
    m_pipe = new Pipe_m(O_RDONLY,O_NONBLOCK,"./choufifo");
    m_pipe->openPipe();
    int fifo_fd = m_pipe->getFd();

    event.data.fd = fifo_fd;
    event.events = EPOLLIN | EPOLLET;
    epoll_ctl(epollfd,EPOLL_CTL_ADD,fifo_fd, &event);
    SetnoBlocking(fifo_fd);//一定要记得设置成非阻塞状态
}


/**
 * @brief 解析管道数据，解析客户数据并添加可写事件
 * 
 * @param argv 
 * @return void* 
 */
void* Server::fifoFunc(void*argv) 
{
    Server *s = (Server*)argv;
    char buf[10000];  //数据
    int n = 0;
    //注意 et模式no_block
    while(1) 
    {
        struct PipeData m_headdata;
        int head = 0;
        //一定要读到完整的包头
        while(head < sizeof(m_headdata)) 
        {

           // n = read(s->m_pipe->getFd(),&m_headdata+head,sizeof(m_headdata) - head);
            n = s->m_pipe->readPipe(&m_headdata+head,sizeof(m_headdata) - head);
            // printf("n === %d!!\n",n);
            //如果包头一点没读 可以退出
            // if(n == 0) {
            //     empty  = true;
            //     break;
            // }
            // sleep(2);
            if(n < 0) 
            {
                continue;
            }
            head += n;
        }

        int fd = m_headdata.fd;
        strncpy(s->user[fd].sendfile_name,m_headdata.Data,sizeof(m_headdata.Data));
        // s->user[fd].sendfile_name = m_headdata.Data;

        //这些是调试不要的
        // memcpy( s->user[fd].sendfile_name,m_headdata.file_name,m_headdata.LEN + 1);
        // printf("get new file name == %s\n",s->user[fd].sendfile_name);
        // if(access(s->user[fd].sendfile_name,0) < 0){
        //     printf("file is not exit!!\n");
        //     continue;
        // }
        // printf("sendfile_name revc pipe == %s fd ==%d \n",s->user[fd].sendfile_name,fd);

        modFd(s->epollfd,fd,EPOLLOUT);//注册可读事件
    }
}
/**
 * @brief 服务器listen的基本步骤，创建socket epoll
 * 
 * @return true 成功返回
 * @return false 失败返回false
 */
bool Server::Listen() 
{
    serve_sock = socket(PF_INET, SOCK_STREAM, 0);
    if(serve_sock < 0) 
    {
        printf("create server_sock error!!\n");
        return false;
    }
    //优雅关闭连接
    // if (0 == m_OPT_LINGER)
    // {
        // struct linger tmp = {0, 1};
        // setsockopt(serve_sock, SOL_SOCKET, SO_LINGER, &tmp, sizeof(tmp));
    // }
    // else if (1 == m_OPT_LINGER)
    // {
        // struct linger tmp = {1, 1};
        // setsockopt(m_listenfd, SOL_SOCKET, SO_LINGER, &tmp, sizeof(tmp));
    // }
    //服务器的基本步骤
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(atoi(m_port));

    if(bind(serve_sock, (struct sockaddr*)&server_addr, sizeof(server_addr) ) < 0) 
    {
        printf("bind error!!!!\n");
        return false;
    }


    if(listen(serve_sock,1000) <  0 ) 
    {
        printf("listen error!!!!\n");
        return false;
    }

    epollfd = epoll_create(EPOLL_SIZE);
    if(epollfd <  0) 
    {
        printf("create epoll error !!!\n");
        return false;
    }

    MyThreadpoolData::m_epollfd = epollfd;

    /*将管道放进epoll进行监听*/
    Mkfifo(epollfd);

    addFdToEpoll(epollfd, serve_sock, false);
    if(pthread_create(&fifo_thread,NULL,fifoFunc,this)  < 0)printf("create error!!\n");
    // printf("listen!!\n");
    return true;
}


/**
 * @brief epollwait循环，监听新连接 读写事件 管道可读事件
 * 
 */
void Server::eventLoop() 
{
    bool ok  =  false;
    while(!ok) 
    {
        // printf("epoll waiting !!!\n");
        int event_cnt = epoll_wait(epollfd, ep_events, MAX_EVENT_NUMBER, -1);
        if(event_cnt < 0&&errno != EINTR) 
        {
            printf("%d epoll faliure!!!\n",event_cnt);
            break;
        }
        // printf("get epoll event number ==  %d  \n",event_cnt);
        for(int i = 0; i < event_cnt; i++) 
        {
            /*epoll结构体中的文件描述符
            有个高级的做法是利用union中的void*和多态实现
            暂时未实现
            */
            int sockfd = ep_events[i].data.fd;
            /*处理新到的客户连接*/
            if(sockfd  == serve_sock) 
            {
                Accept();
                // printf("get conff \n");
            }else if(sockfd == m_pipe->getFd()) 
            {
                // printf("fifo data conming!!!\n");
                /*
                唤醒线程
                对于管道要单独处理
                这里要交给一个特殊的线程来解析
                */
            }
            else if(ep_events[i].events & EPOLLIN) 
            {
                dealwithread(sockfd);
            }
            else if(ep_events[i].events & EPOLLOUT) 
            {
                user[sockfd].write_one = 1;
                dealwithwrite(sockfd);
            }
        }

    }
}

/**
 * @brief 循环接收非阻塞的服务器套接字
 * 
 * @return true 
 * @return false 
 */
bool Server::Accept() 
{
    struct sockaddr_in client_address;
    socklen_t client_addrlength = sizeof(client_address);
    /*因为开启非阻塞 所以要循环接收*/
    while(1) 
    {
        int connfd = accept(serve_sock, (struct sockaddr*)&client_address, &client_addrlength);
        if(connfd < 0) 
        {
            /* printf("accapt done!!!!!\n")*/
            return true;
        }
        // printf("init clineting fd == %d  \n",connfd);
        user[connfd].init(connfd,m_msg);

    }
    // printf("level the func accept!!\n");
    return true;
}

/**
 * @brief 将任务加入写线程池
 * 
 * @param fd usr指针数组的偏移量
 */
void Server::dealwithread(int fd) 
{
    // printf("go to the dealwithread func!!!\n");

    if(m_thread_pool_read->append(user + fd,0))
    {
        // printf("add to threqadpool\n");
    }
    else {
        // printf("add to threqadpool error\n");
    }

}
/**
 * @brief 将任务加入写线程
 * 
 * @param fd  usr指针数组的偏移量
 */
void Server::dealwithwrite(int fd) 
{
    // printf("go to the dealwithwrite func!!!\n");
    if(m_thread_pool_write->append(user + fd, 1)) 
    {
        // printf("add to threadpool\n");
    } else {
        // printf("add to threadpool error!!\n");
    }

}

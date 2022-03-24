#include"filecntl.h"

/**
 * @brief 将文件设置成非阻塞
 * 
 * @param fd 文件描述符
 * @return int 返回旧文件描述符属性
 */
int SetnoBlocking(int fd) 
{
    int old_option = fcntl(fd,F_GETFL);
    int new_option = old_option | O_NONBLOCK;

    fcntl(fd,F_SETFL,new_option);

    return old_option;
}
/**
 * @brief    
    将内核事件表注册读事件，ET模式，X选择开启EPOLLONESHOT
    EPOLLIN         需要读取数据的情况；
    EPOLLOUT        输出缓冲为空，可以立即发送数据的情况；
    EPOLLPRI        收到OOB数据的情况；
    EPOLLRDHUP      断开连接或者半关闭，这种在边缘触发方式下非常有用；
    EPOLLERR        发生错误的情况；
    EPOLLET         以边缘触发的方式得到事件通知；
    EPOOLLONESHOT   发生一次事件之后，相应的文件描述符不再收到事件通知，因此需要再次设置
 * 
 * @param epollfd 
 * @param fd 
 * @param one_shot 
 */
void addFdToEpoll(int epollfd,int fd, bool one_shot) 
{
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
    if(one_shot) 
    {
        event.events |= EPOLLONESHOT;
    }
    /*
    EPOLL_CTL_ADD   将文件描述符注册到epoll例程中；
    EPOLL_CTL_DEL   从epoll例程中删除文件描述符；
    EPOLL_CTL_MOD   更改注册的文件描述符的关注时间发生的情况。
    */
    epoll_ctl(epollfd,EPOLL_CTL_ADD,fd, &event);
    SetnoBlocking(fd);//一定要记得设置成非阻塞状态
}

/**
 * @brief 从内核删除时间表描述符
 * 
 * @param epollfd 
 * @param fd 
 */
void reMoveFd(int epollfd,  int fd) 
{
    epoll_ctl(epollfd,EPOLL_CTL_DEL,fd,0);
    close(fd);
}

/**
 * @brief 将事件重置为EPOOLONESHOT
 * 
 * @param epollfd 
 * @param fd 
 * @param ev 
 */
void modFd(int epollfd, int fd, int ev) 
{
    epoll_event event;
    event.data.fd = fd;
    event.events = ev | EPOLLET | EPOLLONESHOT | EPOLLRDHUP;
    epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &event);
}
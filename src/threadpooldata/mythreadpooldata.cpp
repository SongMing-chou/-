#include"mythreadpooldata.h"
#include<sstream>
#include<vector>


int MyThreadpoolData::m_epollfd = -1;//在server中重新赋值

/**
 * @brief 一开始注册读写事件，然后可读posses之后开启可写事件，当写完或者没成功也要注册读事件。
 * 
 * @param sockfd 用来绑定任务的客户fd
 * @param msg   消息队列channel对象
 * @return int 暂无返回类型
 */
int MyThreadpoolData::init(int sockfd,Publisher *msg) 
{
    m_sockfd = sockfd;
    m_msg = msg;
    // printf("add the clinet socke into epoll\n");
    addFdToEpoll(m_epollfd,m_sockfd,true);
}

/**
 * @brief 处理读事件，et模式循环读
 * 
 * @return true 正常读完  errno为EAGAIN或者EWOULDBLOCK
 * @return false 无数据可读
 */
bool MyThreadpoolData::read_once() 
{
    int bytes_read = 0;
    while(true) 
    {
        bytes_read = recv(m_sockfd, m_read_index + m_read_buf, READ_BUFFER_SIZE - m_read_index, 0);
        if(bytes_read  == -1) 
        {
            if(errno == EAGAIN ||errno == EWOULDBLOCK) 
            {
                modFd(m_epollfd, m_sockfd, EPOLLIN);//要记得注册读事件
                break;
            }
            return false;
        }
        else if( bytes_read  == 0) 
        {
            return false;
        }
        m_read_index += bytes_read;
        
    }
    return true;
}
/**
 * @brief 处理写事件 
 * 当写缓冲区满了的时候，需要再次注册写事件，等待可写时间触发
 * @return true 
 * @return false 
 */
bool MyThreadpoolData::m_write() 
{
    /*是不是第一次发送，第一次要先读文件*/
    if(write_one == 1) 
    {
        int sendfd = open(sendfile_name,0666);
        if(sendfd < 0) 
        {
            printf("open sendfile error sendfile_name = %s!!!\n",sendfile_name);
        }
        m_file_read_index =  read(sendfd,m_write_buf,sizeof(m_write_buf));
        close(sendfd);
        write_one = 0;
    }
    // printf("take the masege == %d going to write!!\n",m_file_read_index);
    int bytes_send = 0;
    while(1) 
    {
        if(m_file_read_index == 0) break;
        bytes_send = write(m_sockfd, m_write_index + m_write_buf, m_file_read_index);
        m_file_read_index-=bytes_send;
        m_write_index += bytes_send;
        printf("send %d byte to client !!\n",bytes_send);
        if(bytes_send < 0) 
        {
            /*缓存区满了*/
            if(bytes_send == -1&&(errno ==EAGAIN || errno == EWOULDBLOCK) ) 
            {
                /*要记得注册写事件*/
                modFd(m_epollfd, m_sockfd, EPOLLOUT);
                break;
            }
            break;
        }
    }
}
/**
 * @brief 逻辑处理，输出重定向，编译用户代码 发送消息
 * 
 * @return true 
 * @return false 
 */
bool MyThreadpoolData::process() 
{

    // printf("go to the func process!!\n");
    //文件名就是客户端套接字字符
    char clientfd[10];
    sprintf(clientfd,"%d",m_sockfd);
    strncat(savefile_name,clientfd,sizeof(clientfd) - 1);

    char c[128];char* end = ".cpp";
    int  i = 0;int j = 0;
    for(;i < strlen(savefile_name);i++) 
    {
        c[i] = savefile_name[i];
    }
    for(;j < strlen(end);j++) 
    {
        c[i+j] = end[j];
    }
    c[i+j] = '\0';
    int savefd = open(c,O_RDWR |  O_TRUNC | O_CREAT | 0666);//, 0666
    if(savefd <= 0 ) 
    {
        return false;
    }
    /*写入文件*/
    int wr = write(savefd, m_read_buf, m_read_index);
    close(savefd);

    /*组装二进制报文*/
    struct MsgData send_M;
    struct BMsgData data_M;
    memset(&data_M,0,sizeof(data_M));
    strncpy(data_M.filename,savefile_name,strlen(savefile_name));
    data_M.fd = m_sockfd;
    memcpy(send_M.mtext,&data_M,sizeof(data_M));
    send_M.mtype = 1;

    //组装报文 垃圾写法 以更正
    // char * t = "file_name: ";
    // int len_t =  strlen(t);
    // strncpy(send_M.mtext,t,len_t);

    // int len_file = strlen(savefile_name);
    // strncpy(send_M.mtext + len_t,savefile_name,len_file);
    // send_M.mtext[len_file+len_t ] = '\r';
    // send_M.mtext[len_file+len_t+ 1] = '\n';

    // char * tt = "client fd: ";
    // int len_tt = strlen(tt);
    // strncpy(send_M.mtext + len_t + len_file + 2,tt,len_tt);

    // int len_c = strlen(clientfd);
    // strncpy(send_M.mtext + len_t + len_file + +len_tt+ 2,clientfd,len_c);
    // send_M.mtext[len_t + len_file + len_c + len_tt + 2] = '\n';
    // send_M.mtext[len_t + len_file + len_c + len_tt + 3] = '\0';

    char makec[128] = "g++ -o ";
    char *x = savefile_name;
    int len_xx = strlen(x);

    int len_end = strlen(end);
    strncpy(makec + 7,x,len_xx);
    makec[ 7 +len_xx] = ' ';
    strncpy(makec+8+len_xx,x,len_xx);
    strncpy(makec+8+len_xx+len_xx,end,len_end);


    char * outpath_end = "_out.txt";
    char outPath[64];
    memset(outPath,0,sizeof(outPath));
    strncpy(outPath ,savefile_name,strlen(savefile_name));
    strncpy(outPath + strlen(savefile_name),outpath_end,strlen(outpath_end));
    savefd = open(outPath,O_RDWR |O_TRUNC| O_CREAT | 0666);//, 0666

    dup2(savefd,STDERR_FILENO);

    system(makec);
    close(savefd);
    // printf("%s\n",makec);

    m_msg->pToMsgsendQ(send_M);

   
    // printf("push to q:\n");
    // for(int i = 0;i < sizeof(send_M.mtext);i++)printf("%c",send_M.mtext[i]);
    // printf("\n");
    //msgsnd(msgid,(void*)&send_M,strlen(send_M.mtext),0);
    // printf("m_sockfd == %d  send_m.id == %d\n",m_sockfd,send_M.ID);
  
    return true;
}

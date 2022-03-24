#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<signal.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<stdint.h>
#include<string>
#include<string.h>
#include"../lock/locker.h"
#include"../uncopyable/uncopyable.h"


/**
 * @brief 管道数据
 * 
 * @param  SRC_PID: 源PID
 * @param  DST_PID: 目的PID
 * @param  fd: 客户端fd句柄
 * @param  LEN: 信息长度
 * @param  Data: 信息
 */
struct PipeData 
{
    uint16_t SRC_PID;
    uint16_t DST_PID;
    int fd;
    uint16_t LEN;
    char Data[64] = {0};
};

/**
 * @brief 管道控制
 * @param mode 打开方式 读写
 * @param block 阻塞方式 阻塞非阻塞
 * @param s 管道文件
 */
class Pipe_m : public uncopyable
{
    int m_fd;               //文件描述符
    int m_mode;             //打开方式
    int m_block;            //阻塞非阻塞
    char m_pipe_name[64];   //管道文件名
public:
    Pipe_m(int mode,int block,char* s) :m_mode(mode),m_block(block)
    {
        memset(m_pipe_name,0,sizeof(m_pipe_name));
        strncpy(m_pipe_name,s,strlen(s));
    };
    
    void openPipe() 
    {
        if(access(m_pipe_name,0) < 0) {
            int n = mkfifo(m_pipe_name,0666);
            if(n < 0) {
                printf("create fifo fail!!!\n");
            }else {
                printf("create fifo sucess!!\n");
            }
        } else {
            printf("file is aready!!\n");
        }
        m_fd = open(m_pipe_name,m_mode | m_block);
        printf("fifo fd == %d\n",m_fd);
        if(m_fd < 0) {
            printf("open fifo fali!!!\n");
        }
    }
    int getFd() 
    {
        return m_fd;
    }
    int readPipe(void*m_buf,size_t m_bytes_) 
    {
        int n = read(m_fd,m_buf,m_bytes_);
        return n;
    }

    int writePipe(void*m_buf,size_t m_bytes_) 
    {
        int n = write(m_fd,m_buf,m_bytes_);
        return n;
    }
};

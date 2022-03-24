#ifndef MYTHREADPOOLDATA_H
#define MYTHREADPOOLDATA_H

#include<cstdio>
#include<string>
#include<fstream>
#include<sys/socket.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/epoll.h>
#include<string.h>
#include<sys/stat.h>
#include<stdio.h>
#include<stdarg.h>
#include<sys/msg.h>
#include<signal.h>
#include<stdlib.h>
#include<sys/fcntl.h>

#include"../ipc/msg.h"
#include"../filecntl/filecntl.h"


static const int FILENAME_LEN = 200;
static const int READ_BUFFER_SIZE = 2048;
static const int WRITE_BUFFER_SIZE = 2048;


class MyThreadpoolData 
{
public:
public:
    MyThreadpoolData(){};
    ~MyThreadpoolData(){};

public:

    int init(int sockfd,Publisher *msg);  /*要在server中调用，注册客户端fd*/
    Publisher* m_msg;           /*Publisher的句柄*/

    bool read_once ();  /*读操作*/
    bool m_write();     /*写操作*/
    bool process();     /*逻辑处理*/

    int write_one = 0;  /*第一次写*/

    int m_state = -1;       /*区分是都还是写 0 读*/
    int m_sockfd;           /*该数据的套接字描述符*/
    static int m_epollfd;   /*复制一份epollfd，逻辑处理完之后用来注册写事件*/
    char sendfile_name[64]; /*发送文件的名字*/

private:

    char m_read_buf[READ_BUFFER_SIZE];  /*接受缓存*/
    int m_read_index = 0;               /*已经接收字节数*/
    char savefile_name[64] = "/home/chou/Desktop/MYPROJECT/src/test/";/*保存文件的名字*/

    char m_write_buf[WRITE_BUFFER_SIZE];    /*发送缓存*/
    int m_write_index = 0;                  /*已经写的字节数*/

    int m_file_read_index;      /*发送*/

};


#endif

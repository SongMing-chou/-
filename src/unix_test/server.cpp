#include<sys/socket.h>
#include<fcntl.h>
#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<assert.h>
#include<string.h>
#include"ipc_temp.h"

static const int CONTROL_LEN = CMSG_LEN(sizeof(int));

int recv_fd(int fd) 
{
    struct iovec iov[1];
    struct msghdr msg;
    char buf[0];

    iov[0].iov_base = buf;
    iov[0].iov_len = 1;
    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;

    cmsghdr cm;
    msg.msg_control = &cm;
    msg.msg_controllen = CONTROL_LEN;

    recvmsg(fd,&msg,0);

    int fd_to_read = *(int *)CMSG_DATA(&cm);
    return fd_to_read;
}
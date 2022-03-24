#include<sys/socket.h>
#include<fcntl.h>
#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<assert.h>
#include"ipc_temp.h"
#include<string.h>

static const int CONTROL_LEN = CMSG_LEN(sizeof(int));

void send_fd(int fd,int fd_to_send) 
{

//    分散/聚集io 支持原子性 
//     struct iovec
//   {
//     void *iov_base;	/* Pointer to data.  */
//     size_t iov_len;	/* Length of data.  */
//   };
    struct iovec iov[1];
    struct msghdr msg;
    char buf[0];

    iov[0].iov_base = buf;
    iov[0].iov_len  = 1;

    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;

    /*辅助数据*/
    cmsghdr cm;
    cm.cmsg_len = CONTROL_LEN;
    cm.cmsg_level = SOL_SOCKET;
    cm.cmsg_type = SCM_RIGHTS;
    *(int *)CMSG_DATA(&cm) = fd_to_send;
    msg.msg_control = &cm;
    msg.msg_controllen = CONTROL_LEN;

    sendmsg(fd,&msg,0);

}

int main() {
    p_msg msg("./ipc_temp.h",0);
    
}
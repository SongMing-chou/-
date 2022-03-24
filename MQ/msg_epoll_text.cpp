#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <sys/stat.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/uio.h>
#include<sys/msg.h>
#include<sys/ipc.h>
#include<sys/sem.h>
#include<stdint.h>


// #define timetext
#ifdef timetext
//5秒一定时
void sig_handler(int sig ) {
    if(sig == SIGALRM)
        printf("timer up!!!\n");
    alarm(5);
}
#endif
int main() {
    // signal(SIGALRM,sig_handler);
    // alarm(5);
    // while(1) ;
    int epollfd = epoll_create1(10);

    struct epoll_event ev, events[20];

    key_t KEY = ftok("../src/chou_key",'b');
    int msgque_id = msgget(KEY,066|IPC_CREAT);

    printf("msgid == %d \n",msgque_id);


    ev.data.fd = msgque_id;
    ev.events = EPOLLIN;
    if(epoll_ctl(epollfd,EPOLL_CTL_ADD,msgque_id,&ev) == -1)printf("error!!!\n");

    while(1) {
        int c = epoll_wait(epollfd,events,20,-1);
        if(c > 0) {
            printf("get!!!\n");
        }
    }
    return 0;
}
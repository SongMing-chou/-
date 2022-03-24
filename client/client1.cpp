#include<sys/socket.h>
#include<stdio.h>
#include<string.h>
#include<sys/types.h>
#include<sys/pci.h>
#include<unistd.h>
#include<sys/fcntl.h>
#include<arpa/inet.h>
#include<stdlib.h>
#include<pthread.h>
#include<sys/epoll.h>
#include<errno.h>



void * func_con(void *argv) {
    int n = 1000;
    int epoll_fd;
    int *connfd = new int(3000);
    epoll_event fd[n];
    if((epoll_fd = epoll_create(n)) < 0) {
        printf("create error!!\n");
    }
        struct sockaddr_in server_addr;
        // char *ip = "120.77.87.178";
        //    char *ip = "125.217.249.87";
           char *ip = "211.66.15.194";


        char* port = "9006";
        // printf("socket sucess!!\n");
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = inet_addr(ip);
        server_addr.sin_port = htons(atoi(port));

    for(int  i = 0;i < n;i++) {
        // sleep(1);
        connfd[i] = socket(AF_INET,SOCK_STREAM,0);
        int nn = connect(connfd[i],(struct sockaddr*)&server_addr,sizeof(server_addr));
        if(nn< 0) {
            printf("%d == error!!connect error!! errno == %d\n",i,errno);
        }else{
            printf("%d == success!!\n",i);
        }
    }

    // printf("done");
    for(int i = 0;i < n;i++) {
        char buf[1000];
        int fdd = open("send0.txt",O_RDONLY);
        if(fdd< 0) {
            printf("open error!! errno == %d\n",errno);
        }
        int  o = read(fdd,&buf,1000);
        write(connfd[i],&buf,o);
        close(fdd);
        epoll_event even;
        even.data.fd = connfd[i];
        even.events = EPOLLIN | EPOLLET;
        epoll_ctl(epoll_fd,EPOLL_CTL_ADD,connfd[i],&even);
    }



    // sleep(10);

//     for(int  i = n;i < 2*n;i++) {
//         // sleep(1);
//         connfd[i] = socket(AF_INET,SOCK_STREAM,0);
//         int nn = connect(connfd[i],(struct sockaddr*)&server_addr,sizeof(server_addr));
//         if(nn< 0) {
//             printf("%d == error!!\n",i);
//         }else{
//             printf("%d == success!!\n",i);
//         }
//     }
//     // printf("done");
//     for(int i = n;i < 2*n;i++) {
//         char buf[1000];
//         int fdd = open("send0.txt",O_RDONLY);
//         if(fdd< 0) {
//             printf("open error!!\n");
//         }
//         int  o = read(fdd,&buf,1000);
//         write(connfd[i],&buf,o);
//         close(fdd);
//         epoll_event even;
//         even.data.fd = connfd[i];
//         even.events = EPOLLIN | EPOLLET;
//         epoll_ctl(epoll_fd,EPOLL_CTL_ADD,connfd[i],&even);
//     }



//     // sleep(10);

//    for(int  i = 2*n;i < 3*n;i++) {
//         // sleep(1);
//         connfd[i] = socket(AF_INET,SOCK_STREAM,0);
//         int nn = connect(connfd[i],(struct sockaddr*)&server_addr,sizeof(server_addr));
//         if(nn< 0) {
//             printf("%d == error!!\n",i);
//         }else{
//             printf("%d == success!!\n",i);
//         }
//     }
//     // printf("done");
//     for(int i = 2*n;i < 3*n;i++) {
//         char buf[1000];
//         int fdd = open("send0.txt",O_RDONLY);
//         if(fdd< 0) {

//             printf("open error  ==  %d!!\n",errno);
//            // EAGAIN
//         }
//         int  o = read(fdd,&buf,1000);
//         write(connfd[i],&buf,o);
//         close(fdd);
//         epoll_event even;
//         even.data.fd = connfd[i];
//         even.events = EPOLLIN | EPOLLET;
//         epoll_ctl(epoll_fd,EPOLL_CTL_ADD,connfd[i],&even);
//     }

    // sleep(100);
    while(1) {
        int num = epoll_wait(epoll_fd,fd,sizeof(fd),0);
        for(int  i = 0;i < num;i++) {
            int fdd = fd[i].data.fd;
            printf("client ==%d\n",fdd);
        }
    }
}
int main() {
    pthread_t id;
    int n = pthread_create(&id,NULL,func_con,NULL);
    while(1);
    return 0;
}
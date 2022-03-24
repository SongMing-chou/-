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


void * func_con(void *argv) {
    // char *ip = "120.77.87.178";
    char *ip = "125.217.249.87";
    char* port = "9006";
    int connfd;
    char buf[1000];

    int * ar = (int*)argv;

    struct sockaddr_in server_addr;

    connfd = socket(AF_INET,SOCK_STREAM,0);
    // printf("socket sucess!!\n");
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip);
    server_addr.sin_port = htons(atoi(port));

    int n = connect(connfd,(struct sockaddr*)&server_addr,sizeof(server_addr));
    if(n == -1) {
        printf("connect error!!!\n");
    }else{
        printf("connfd sucess!!\n");
    }
    int fd = 0;
    // if(*ar%2 == 0)
        fd = open("send0.txt",O_RDONLY);
    // else{
    //     // fd = open("send1.txt",O_RDONLY);
    // }

    int  o = read(fd,&buf,1000);
    write(connfd,&buf,o);
    // printf("send to server :\n");
    // printf("%s\n",buf);
    memset(buf,0,sizeof(buf));
    read(connfd,buf,299);
    printf("rev  from server: \n");

    for(int i = 0;i < 299;i++) {
        printf("%c",buf[i]);
    }
    printf("\n");

    // printf("%s\n",buf);
    close(fd);
    close(connfd);
    printf("client == %d\n",*ar);
}
int main() {
    int n = 10;
    pthread_t a[n];
    int c[n];
    for(int  i = 0;i < n;i++)c[i] = i;
    for(int  i = 0;i < n;i++) {
       if(0!=pthread_create(&a[i],NULL,func_con,&c[i])){
           printf("error!!!\n");
       }
    }
    sleep(800);
    return 0;
}
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


struct RuquestHead 
{
    uint32_t op;
    uint32_t who;
    char host[20];
};
int main() {
    char data[28];
    memset(data,0,sizeof(data));
    RuquestHead c{65,66,"127.0.0.1"};
    // printf("%d %d %s %d\n",c.op,c.who,c.host,sizeof(c));
    memcpy(data,&c,sizeof(c));
    RuquestHead b;
    int fd = open("text.txt",O_WRONLY);
    for(int i = 0;i < 28;i++)printf("%c",data[i]);
    printf("\n");

    printf("%s\n",data);
    

    memset(b.host,0,sizeof(b.host));
    memcpy(&b,data,sizeof(b));//

    write(fd,&b,sizeof(b));
    close(fd);
    printf("%d %d %s\n",b.op,b.who,b.host);//b.host
    return 0;
}
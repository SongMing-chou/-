#include"msg.h"
#include<iostream>
using namespace std;

int main() 
{
    Publisher *c = new Publisher();
    c->init("127.0.0.2","127.0.0.1");
    int sendfd = open("send.txt",O_RDONLY);
    if(sendfd < 0) 
    {
        printf("open error!!!\n");
    }
    char msg[254];
    memset(msg,0,sizeof(msg));
    read(sendfd,msg,sizeof(msg));

    /*组装二进制报文*/
    struct MsgData send_M;
    struct BMsgData data_M;
    memset(&send_M,0,sizeof(send_M));
    memset(&data_M,0,sizeof(data_M));

    memcpy(data_M.filename,msg,sizeof(msg));
    memcpy(send_M.mtext,&data_M,sizeof(data_M));
    send_M.mtype = 1;

    for(int i = 0;i < 10000000;i++)
    {
        c->pToMsgsendQ(send_M);
    }

    char cc;
    while(cin>>cc) {
        if(cc=='q'||cc=='Q')break;
        sleep(10);
    }
    delete c;
    return 0;
}
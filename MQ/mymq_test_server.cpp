#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/msg.h>
#include<signal.h>
#include<string.h>
#include<sys/ipc.h>
#include<sys/sem.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<errno.h>
#include<stdint.h>
//消息类型
struct msgbuf1 {
    long  mtype;
    char mtext[128];
    int ID;
};

class p_msg {
public:
    key_t key;
    int msgid;
    struct msgbuf1 buf;
    p_msg(char* c,long level) {
        key  = ftok(c,'a');
        msgid = msgget(key,IPC_CREAT|0777);
        if(msgid < 0) {
            printf("msgget faill!!\n");
        }else {
            printf("msgid  ==  %d\n",msgid);
        }
        buf.mtype = level;
        // printf("construct errno == %d\n",errno);
    }
    //清零
    void msg_memset() {
        memset(buf.mtext,0,sizeof(buf.mtext));
    }

    //接收 level == n的消息类型
    void msg_msgrcv(long n,int block) {
        int ev = 0;
        if(block) {
            ev |= IPC_NOWAIT;
        }
        msgrcv(msgid,(void*)&buf,sizeof(buf.mtext),n,ev);
        return ;
    }
    //发送 level == n的消息类型
    void msg_msgsnd(int n,int block,char *c) {
        memset(&buf,0,sizeof(buf));
        int ev = 0;
        strncpy(buf.mtext,c,strlen(c));
        if(block) {
            ev |= IPC_NOWAIT;
        }
        buf.mtype =(long ) n;

        // printf("errno  == %d\n",errno);
        // buf.mtext[11] = '\0';
        msgsnd(msgid,&buf,strlen(buf.mtext),0);
        // printf("msgid == %d\n%s\n errno  ==  %d\n buf.size() == %d\n",msgid,buf.mtext,errno,strlen(buf.mtext));

        return ;
    }
};

int main() {

    char *c  = "0 127.0.0.2";
    char* v = "0hi client";
    p_msg madd("locker.h",0);
    madd.msg_msgsnd(1,false,c);
    madd.msg_msgrcv(2,false);
    printf("%s\n",madd.buf.mtext);
    madd.buf.mtype = 255;//转到消息频道
    int  t = 10;
    while(t-- > 0) {
        // scanf("%c",&c);
        memset(madd.buf.mtext,0,sizeof(madd.buf.mtext));
        strncpy(madd.buf.mtext,v,strlen(v));
        madd.msg_msgsnd(255,false,v);
        // sleep(2);
    }
    while(1);

}
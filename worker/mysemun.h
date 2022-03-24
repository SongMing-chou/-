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
#include<stdint.h>

//信号量
union semun {
    int val;
    struct  semid_ds *buf;      //用于SETVAL命令
    unsigned short *array;      //用于于IPC——STAT和IPC——SET命令
    struct seminfo *__buf;      //用于IPC——INFO命令
};

class p_sem {
    int semid;
    key_t key;
    int n = 0;
public:
    //信号集 数目
    p_sem(int n,char* s) {
        this->n = n;
        key = ftok(s,'b');
        if(key == -1) {
            printf("get key error!!\n");
        }
        semid = semget(key,n,IPC_CREAT|0666);
        if(semid < 0) {
            printf("semget error!!!\n");
            semctl(semid, 0, IPC_RMID);
        }
    }

    //初始化i个信号量，初始值为k
    void init_sem(int k) {
        union semun myun;
        for(int  i = 0;i < n;i++) {
            myun.val = k;
            semctl(semid,i,SETVAL,myun);
        }
    }

    //执行pv操作
    void pv(int num,int op) {
        struct sembuf buf;
        buf.sem_num = num;
        buf.sem_op  = op;
        buf.sem_flg = 0;
        semop(semid,&buf,1);
    }
};



/**
 * 已经不用了的一个类
 * 
*/
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
    p_msg(char* c,long level) 
    {
        key  = ftok(c,'b');
        msgid = msgget(key,IPC_CREAT|0666);
        if(msgid < 0) {
            printf("msgget faill!!\n");
        }else{
            printf("msgid == %d\n",msgid);
        }
        buf.mtype = level;
    }
    //清零
    void msg_memset() 
    {
        memset(buf.mtext,0,sizeof(buf.mtext));
    }

    //接收 level == n的消息类型
    void msg_msgrcv(long n,int block) 
    {
        int ev = 0;
        if(block) {
            ev |= IPC_NOWAIT;
        }
        msgrcv(msgid,(void*)&buf,sizeof(buf.mtext),n,ev);
        return ;
    }
    //发送 level == n的消息类型
    void msg_msgsnd(int n,int block) 
    {
        int ev;
        if(block) {
            ev |= IPC_NOWAIT;
        }
        msgsnd(msgid,(void*)&buf,strlen(buf.mtext),ev);
        return ;
    }
};

//管道的数据
struct myFifoData {
    uint16_t SRC_PID;       //源PID
    uint16_t DST_PID;       //目的PID
    int fd;            //回执的客户端fd
    uint16_t LEN;           //长度
    char file_name[64] = {0};    //文件名
};

class p_fifo {
    int fd;
    int m_mstat;
public:
    p_fifo() {

    }
    void init(char *cc,int m_stat) {
        if(access(cc,0) < 0) {
            int n = mkfifo(cc,0666);
            if(n < 0) {
                printf("create fifo fail!!!\n");
            }else {
                printf("create fifo sucess!!\n");
            }
        } else {
            printf("file is aready!!\n");
        }
        fd = open(cc,m_stat);
        m_mstat = m_stat;
        printf("fifo fd == %d\n",fd);
        if(fd < 0) {
            printf("open fifo fali!!!\n");
        }
    }
    bool p_fifo_write(myFifoData*cc,int n) {
        if((m_mstat & O_WRONLY) != 0 || (m_mstat & O_RDWR) != 0) {
            write(fd,cc,n);
            return true;
        }
        return false;
    }
    bool p_fifo_read(myFifoData*cc,int n) {
        if((m_mstat & O_RDONLY)!=0 || (m_mstat & O_RDWR)!= 0 ) {
            read(fd,cc,n);
            return true;
        }
        return false;
    }

};


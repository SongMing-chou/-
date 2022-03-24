#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/msg.h>
#include<signal.h>
#include<string.h>
#include<sys/ipc.h>
#include<sys/sem.h>
#include"mysemun.h"


// //消息类型
// struct msgbuf1 {
//     long  mtype;
//     char mtext[128] = "pp";
//     int ID;
// };



int main() {
    char* cc = "../src/chou_key";
    char *oo = "../src/choufifo";
    
    //初始化管道
    p_fifo m_fifo;
    m_fifo.init(oo,O_RDWR);
    
    //消息
    p_msg m_msg(cc,1);

    //初始化信号量
    p_sem m_sem(1,cc);
    m_sem.init_sem(1);
    while(1) {

        struct myFifoData Data;
        memset(Data.file_name,0,sizeof(Data.file_name));
        // printf("sizeof == %d\n",sizeof(Data.file_name));
            //   int c = getpid();
        // printf("pid == %d\n",c);
        m_msg.msg_memset();
        m_sem.pv(0,-1);//信号量
        // printf("waiting mesge!!!\n");
            m_msg.msg_msgrcv(m_msg.buf.mtype,false);
        m_sem.pv(0,1);//信号量

        // printf("data from server:\n");
        // printf("****************************\n");
        bool namef = false;
        int j = 0,k = 0;
        bool fdf = false;
        char fdd[10];
  
        //提取消息
        for(int  i = 0;i < 128;i++) {
            if(m_msg.buf.mtext[i] == '\r' ||m_msg.buf.mtext[i] == '\n') {
                fdf = true;
                continue;
            }
            else if( m_msg.buf.mtext[i]=='\0')break;
            else if(m_msg.buf.mtext[i] == ' '&&!namef) {
                namef = true;
            }else if(!fdf&&namef && m_msg.buf.mtext[i]!=' ') {
                Data.file_name[j] = m_msg.buf.mtext[i];
                j++;
            }else if(!fdf&&namef&&m_msg.buf.mtext[i]==' ') {
                fdf = true;
            }else if(fdf) {
                fdd[k] =  m_msg.buf.mtext[i];
                k++;
            }
        }
        for(int i = 0;i< 128;i++) {
            if(m_msg.buf.mtext[i] != '\0') {
                printf("%c",m_msg.buf.mtext[i]);
            }else{
                break;
            }
        }
        // printf("\n");
        // printf("****************************\n");
        // printf("data file name == %s \n",Data.file_name);
        m_fifo.p_fifo_write(&Data,sizeof(Data));
        int sendf = open(Data.file_name,0666);
        if(sendf < 0) {
            printf("open sendf faile!!\n");
        }else{
            printf("open sendf sucess!!!\n"); 
            close(sendf);  
        }
     
    }
    return 0;
}
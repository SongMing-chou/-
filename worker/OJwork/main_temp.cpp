#include <stdio.h>
#include <stdbool.h>
#include "./source/source.h"
#include<stdlib.h>
#include "./monitor/monitor.h"
#include"../mysemun.h"
#include<time.h>


static void getAndSetConfig(int argc, char *argv[], struct ExecveConfig *config);
static bool checkConfig(const struct ExecveConfig *const config);
static void printResult(const struct ExecveResult *const result);
static void showUsage(void);

int main(int argc, char *argv[])
{


    char* cc = "../../src/chou_key";
    // char* back = "../../src/chou_back";
    char *oo = "../../src/choufifo";
    
    //初始化管道
    p_fifo m_fifo;
    m_fifo.init(oo,O_WRONLY);
    
    //消息
    p_msg m_msg(cc,1);

    // p_msg m_back(back,1);

    //初始化信号量
    p_sem m_sem(1,cc);
    m_sem.init_sem(1);

    struct ExecveConfig config;
    struct ExecveResult result;
    struct myFifoData Data;

    config.stderrPath = "err.txt";
    config.stdinPath  = "in.txt";

  
    while(1) 
    {

    memset(Data.file_name,0,sizeof(Data.file_name));
    // printf("sizeof == %d\n",sizeof(Data.file_name));
    m_msg.msg_memset();


    m_sem.pv(0,-1);//信号量
        printf("waiting mesge!!!\n");
    m_msg.msg_msgrcv(m_msg.buf.mtype,false);
    m_sem.pv(0,1);//信号量

    // printf("data from server:\n");
    // printf("****************************\n");
    bool namef = false;
    int j = 0,k = 0;
    bool fdf = false;
    char fdd[20];
    memset(fdd,0,sizeof(fdd));
  
    //提取消息
    for(int  i = 0;i < 128;i++) 
    {
        if(m_msg.buf.mtext[i] == '\r' ||m_msg.buf.mtext[i] == '\n') 
        {
            fdf = true;
            continue;
        }
        else if( m_msg.buf.mtext[i]=='\0')break;
        else if(m_msg.buf.mtext[i] == ' '&&!namef) 
        {
            namef = true;
        }
        else if(!fdf&&namef && m_msg.buf.mtext[i]!=' ') 
        {
            Data.file_name[j] = m_msg.buf.mtext[i];
            j++;
        }else if(!fdf&&namef&&m_msg.buf.mtext[i]==' ') 
        {
            fdf = true;
        }else if(fdf) {
            fdd[k] =  m_msg.buf.mtext[i];
            k++;
        }
    }
    bool ok  = false;int clientfd = 0;
    for(int i = 0;i < strlen(fdd);i++) 
    {
        if(fdd[i] == ':') 
        {
            ok = true;
            i++;
        }
        else if(ok) 
        {
            // printf("fdd[i] == %c\n",fdd[i]);
            clientfd  = clientfd*10+fdd[i] - '0';
        }
    }
    Data.fd = clientfd;

    // printf("fdd == %s  strlen == %d\n",fdd,strlen(fdd));
    // printf("fd == %d\n",clientfd);
    // for(int i = 0;i< 128;i++) {
    //     if(m_msg.buf.mtext[i] != '\0') {
    //         printf("%c",m_msg.buf.mtext[i]);
    //     }else{
    //         break;
    //     }
    // }

    initializeConfig(&config);
    initializeResult(&result);

    config.execvePath = Data.file_name;//获取文件
    char outPath[64] ;
    strncpy(outPath,config.execvePath,strlen(config.execvePath));
    char * t = "_out.txt";
    strncpy(outPath + strlen(config.execvePath),t,strlen(t));
    outPath[strlen(config.execvePath)+strlen(t)] = '\0';
   
    config.stdoutPath = outPath;
    // config.stdoutPath = "/home/user/MYPROJECT/src/filetest/out.txt";

    if(checkConfig(&config)) 
    {
        // printf("check over!!,start runing!!!\n");
        startMonitor(&config, &result);
    }
    else 
    {
        result.condition = CONFIG_ERROR;//设置错误
    }

    
    for(int  i = 0;i < strlen(outPath);i++)
        Data.file_name[i] = outPath[i];

    m_fifo.p_fifo_write(&Data,sizeof(Data));
    // int sendf = open(Data.file_name,0666);
    printResult(&result);

    }
    return 0;
}
bool checkConfig(const struct ExecveConfig *const config)
{
    if(config->cpuTimeLimit < 0
       || config->realTimeLimit < 0
       || config->memoryLimit < 1024
       || config->wallMemoryLimit < 1024
       || config->outputSizeLimit < 0
       || config->execvePath == NULL
       || config->execvePath[0] == '\0')
        return false;
    else
        return true;
}
inline void printResult(const struct ExecveResult *const result) 
{
    // 此处的stdout将被调用者处理 应该以json字符串形式表示
   // printf("cuptimecosts == %d memorycost == %d conditionnum == %d\n", result->cpuTimeCost, result->memoryCost, result->condition);
}

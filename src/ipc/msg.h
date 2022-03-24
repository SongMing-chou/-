#ifndef MSG_H
#define MSG_H

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
#include<sys/msg.h>
#include<queue>
#include<pthread.h>
#include<unordered_set>
#include<unordered_map>
#include<sys/socket.h>
#include<arpa/inet.h>


#include"../lock/locker.h"
#include"../uncopyable/uncopyable.h"


using namespace std;



/**
 * @brief 请求channel暂时固定为127.0.0.1  请求报文只含有请求头部，数据报文暂时没有用到头部 之后可能会增加
 *        ACK功能未实现
 * 
 * 向broker申请成功后 每个publisher拥有一个私有channel,broker使用publisher私有channel接收publisher的消息。
 * 
 * sonsumer发送请求报文给broker ，broker解析其感兴趣的publisher,如果有消息返回消息，如果没消息，返回hello。
 * consumer现在支持一条一条消息拉取，暂未实现broker推送多条消息
 * 
 */



/**
 * @brief 消息队列数据报文(只是载体，不是真实报文)
 * @param mtype 消息类型
 * @param mtext 消息数据
 * @param ID    暂时未用到
 */
struct MsgData
{
    long mtype;
    char mtext[254];
    int ID;
};
/**
 * @brief 消息真实数据，好处是不用字符串拼接，坏处就是要浪费额外空间
 * @param filename 文件名字
 * @param fd socket
 */
struct BMsgData
{
    char filename[64];
    int fd ;
};

/**
 * @brief 报文头状态
 * @param REQUESTmsg 请求消息
 * @param REGISTERED 申请注册
 * @param ACK        ack
 * @param RESPONSE   应答报文
 */
enum HEADSTATUS 
{
    REQUESTmsg = 0,
    REGISTERED,
    ACK,
    RESPONSE,
};
/**
 * @brief       请求报文头
 * @param op    报文头状态
 * @param who   Publisher = 0  consumer = 1
 * @param host  host地址
 * @param ID    确认消息的序号
 */
struct RuquestHead 
{
    HEADSTATUS op;
    uint32_t who;
    uint32_t phost;
    uint32_t chost;
    uint32_t ID;
};

/**
* @brief  MSG错误码的定义
* @param  MSGEXIST        Publisher host已存在 该状态用于Publisher
* @param  BROKERER        broker未启动
* @param  CREATETHREADER  创建线程失败
* @param  BROKREJET       broker 单方面拒绝或不理会
* @param  PUBLISHERER     publiser不存在 该状态用于拒绝Consumer
* @param  SUCCEED         成功
*/
enum msgErrno {
    MSGEXIST = 0, 
    BROKERER,     
    CREATETHREADER,
    BROKREJET,
    PUBLISHERER,
    SUCCEED,
};
/**
 * @brief 基类
 * 
 */
class Msg_m : public uncopyable
{
private:
    /*接收 msg_type 消息类型到msgrevdQue中 阻塞*/
    virtual void pToMsgrcvQ(long msg_type,MsgData buf){return ;};
    /*从接收队列头部取出消息*/
    virtual bool Revc(int block,MsgData &buf){return true;};
    /*将消息压入待发送的消息队列中*/
    virtual void pToMsgsendQ(MsgData buf){return ;};
    /*从待发送消息队列中 中发送消息*/
    virtual bool Send(int block,MsgData buf){return true;} ;

public:
    key_t key;
    int msgid;
    long m_level;

    void getHead(RuquestHead &head,char*consumerhost, char*publisherhost,
                HEADSTATUS op,uint32_t who,uint32_t ID);
    /*销毁一个消息*/
    void destroymsg(int mid)
    {
        char t[20] = "ipcrm -q ";
        memset(t+9,0,11);
        sprintf(t+9,"%d",mid);
        printf("destroymsg == %s\n",t);
        system(t);
    }
};

/**
 * @brief 生产者发送到broker
 * 
 * 只向使用者提供init() pToMsgsndQ() exit() 接口
 * 如果长时间为调用 exit() broker会将其会话移除
 */
class Publisher :public Msg_m
{

    int broker_msgid;
    locker send_lock;
    sem send_sem;
    pthread_t send_threadid;
    std::queue<MsgData> msgSendQue;/*待发送队列*/
    void sendRun();
    /*这个线程一直在从队列中提取消息(如果有的话) 并push到broker*/
    static void *Pusher(void*argv)
    {
        Publisher* s = (Publisher*)argv;
        s->sendRun();
    }
    bool Create();
    bool Send(int block,MsgData buf);

public:
    Publisher()//char *c ,int level
    {
        // pthread_create(&send_threadid,NULL,Pusher,this);
    }
    ~Publisher() 
    {
        destroymsg(msgid);
    }
    msgErrno init(char* publisher_host,char *broker_host);
    void pToMsgsendQ(MsgData buf);

    
    bool exit();
};

/**
 * @brief 消费者从broker拉取消息
 * 
 */
class Consumer :public Msg_m
{
    locker rev_lock;
    sem rev_sem;
    pthread_t rev_threadid;
    /*推模型队列 设置最大值 表示消费者最多消费多少*/
    std::queue<MsgData> msgrevdQue;
    static void *revcWork(void*argv)
    {
        Consumer* s = (Consumer*)argv;
        s->revcRun();
    }
    void revcRun();
    /*以上拉模型暂未实现*/


    int broker_msgid;
public:
    Consumer()//char *c ,int level
    {
        // pthread_create(&rev_threadid,NULL,revcWork,this);
    }

    ~Consumer()
    {
        destroymsg(msgid);
    }
    msgErrno init(char* publisher_host,char* conmer_host,char *broker_host );
    void pToMsgrcvQ(long msg_type,MsgData buf);

    msgErrno getMsg(MsgData&buf);


    char data[255];  /*将消息提取出来*/
    void takeData(MsgData&buf);


    /*未实现ACK功能*/
    bool sendAck();
    bool Revc(int block,MsgData&buf);


};


class Broker:public Msg_m
{
    /*管理方案 应该是可以参考智能指针的引用计数的管理方法*/
    unordered_map<uint32_t,unordered_set<string> >publisher;/*管理pubilsher 现在还没实现 将用来解散consumer*/
    unordered_map<uint32_t,string>consumer;                 /*管理consumer ？好像现在还不需要*/


    unordered_map<uint32_t,int> msgidD;     /*记录全局信号host--id*/
    unordered_map<int,locker> mlocker;      /*每个publisher队列持有的锁*/
    unordered_map<int,sem> msem;            /*每个publisher队列持有的信号量*/
    unordered_map<uint32_t,queue<MsgData>>PublisherQ; /*每个publisher持有的队列*/

    /*管理每个 publisher 的消息*/
    locker qlock;   /*锁队列*/
    sem qsem;
 

    // unordered_map<string,queue<MsgData>>ConsumerQ; /*现在还不需要*/
    pthread_t requestPth,recvPth;
    static void*requesTask(void*argv);
    static void*recvTask(void*argv);
    int msgidrev;
public:
    Broker(char *c = "127.0.0.1",char*d = "127.0.0.126")
    {
        key = ftok(c,'a');
        msgid = msgget(key,IPC_CREAT |0666);
        if(msgid < 0) 
        {
            printf("send msgget error!!!\n");
        }else {
            printf("msgid == %d\n",msgid);
        }
        /*用于类似DHCP协议的通道，暂未使用*/
        // int rkey = ftok(c,'a');
        // msgidrev = msgget(rkey,IPC_CREAT |0666);
        // if(msgidrev < 0) 
        // {
        //     printf("recv msgget error!!!\n");
        // }
    }

    void requesRun();
    void recvRun();

    void init();

    /*尚未实现的部分*/
    void start();
    bool revcAck();

};
#endif

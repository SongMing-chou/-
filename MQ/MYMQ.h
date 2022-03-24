#include<iostream>
#include<unistd.h>
#include"../worker/mysemun.h"
#include<vector>
#include"locker.h"
#include<queue>
#include<time.h>
#include<sys/epoll.h>
#include<fcntl.h>
#include<sys/types.h>
#include<stdlib.h>
#include<stdio.h>
#include<signal.h>
#include<errno.h>
#include<unordered_set>
#include<unordered_map>
#include<pthread.h>




using namespace std;

//MBlock_Queue
class MBlock_Queue {

    //消息类型
    struct msgbuf1 {
        long  mtype;
        char mtext[128];
        int ID;
    };

    int m_size;
    int cur_size = 0;   //当前大小  acksize肯定小于cur_size,当ack之后cur_size才减一
    int ack_size = 0;   //需要ack的大小   
    std::vector<msgbuf1> mQ;

    locker q_lock;      //维护队列

    int GET_INDEX   = 0;//下一个收取下标
    int SEND_INDEX  = 0;//下一个发送下标
    int ACK_INDEX   = 0;//下一个ack下标

    enum STATUS {
        GET = 0,    //已收到
        SEND = 1,   //已发出
        ACK = 2,    //空位置
    };

    std::vector<STATUS> BITS;     //状态标记

    //表示重传次数以及重传下标
    struct reSend_Times {
        int clientID;
        int index;
        int times;
    };
    //自定义排序，用于最小堆
    class cmp {
    public:
        bool operator()(pair<time_t,reSend_Times>&x,pair<time_t,reSend_Times>&y) {
            return x.first > y.first;
        }
    };
    time_t timeout = 10;
    //发出去的时候 最小堆启用超时重传，重传4次失败，这样就是没发送 状态重新置为GET
    priority_queue<pair<time_t,reSend_Times>,vector<pair<time_t, reSend_Times> >,cmp>timer_Q;
    
    //定时器
    timer_t timerid;
    struct sigevent sev;
    struct itimerspec its;
    //信号
    sigset_t mask;
    struct sigaction sa;
    long long time_out  = 10;


    //三个接口 给MQ队列接收 根据监听的io类型来 调用
    bool getMQ(msgbuf1&value) ;
    bool sendMQ(msgbuf1&value,int client,int &index) ;

    bool ackMQ(int index) ;

    //信号处理函数
    static void SigHandler(int sig,siginfo_t*si,void * us) {
        if(sig == SIGALRM) {
            MBlock_Queue *temp = (MBlock_Queue*)si->_sifields._timer.si_sigval.sival_ptr;
            // printf("time out !!! flash queue!!!\n");
            temp->reSend();
        }
    }
    //重传函数
    void reSend();

public:
    key_t key;
    int msgid;

    // int Master = 0;
    MBlock_Queue(char *cc,int msize) ;

    // enum {
    //     CADD = 0,
    //     CSEND,
    //     CRCV,
    // };
    //退出或加入channel  为0
    //请求发送channel    为255


    unordered_set<int> set;//标记已连接用户
    unordered_map<int,int> map;//记录上一条消息 
    
    // vector<int> ms_client;      //保存消息用户id,也就是消息ID

    //已处理为 0
    //处理为为 1
    //未处理为 2
    // vector<int> request_mask;   //消息掩码，表示请求消息或者有消息到来的

    static void *Listen(void*arg);
    static void *Wroker(void*arg);

    pthread_t listen_id,work_id;

    void RUN();

    locker mask_lock;
};

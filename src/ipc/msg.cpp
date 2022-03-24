#include"msg.h"
#include<sys/socket.h>
#include<fcntl.h>
#include<stdio.h>

#include<unistd.h>
#include<stdlib.h>

/**
 * @brief 组装报文头
 * 
 * @param head 报文头
 * @param host host地址
 * @param op   状态
 * @param who  publisher or consumer
 * @param ID   确认消息序列号
 */
void Msg_m::getHead(RuquestHead &head,char*consumerhost, char*publisherhost,
                HEADSTATUS op,uint32_t who,uint32_t ID)
{
    if(consumerhost!=NULL)
        head.chost = inet_addr(consumerhost);
    if(publisherhost!=NULL)
        head.phost = inet_addr(publisherhost);

    head.ID = ID;
    head.op = op;
    head.who = who;

}

// void Msg_m::getPost(MsgData revc,MsgData &get)
// {

// }

/**
 * @brief 在发送线程中执行的函数 有消息就会发送
 *
 */
void Publisher::sendRun()
{
    while(1)
    {
        send_sem.wait();
        send_lock.lock();
        if(msgSendQue.empty())
        {
            send_lock.unlock();
            continue;
        }
        MsgData request = msgSendQue.front();
        msgSendQue.pop();
        send_lock.unlock();
        Send(0,request);
    }
}
/**
 * @brief 将消息压入待发送的消息队列中
 *        使用者使用该接口 向publiser交付一个消息
 * @param buf 传入的消息
 */
void Publisher::pToMsgsendQ(MsgData buf)
{
    send_lock.lock();
    // printf("msg push to q!!\n");
    msgSendQue.push(buf);
    // BMsgData t;
    // memcpy(&t,buf.mtext,sizeof(t));
    // printf("%s  %d \n",t.filename,t.fd);
    //唤醒
    send_sem.post();
    send_lock.unlock();
    return ;
}
/**
 * @brief  从待发送消息队列中 中发送消息
 *
 * @param block 发送消息的方式 阻塞非阻塞
 * @param buf
 * @return true 成功时返回true
 * @return false 失败返回false
 */
bool Publisher::Send(int block,MsgData buf)
{
    msgsnd(msgid,(void*)&buf,sizeof(buf.mtext),block);
    return true;
}
/**
 * @brief 向broker注册一个publisher
 * 
 * @param publisher_host publisher host
 * @param broker_host    broker host
 * @return true     成功
 * @return false    失败
 */
msgErrno Publisher::init(char* publisher_host,char *broker_host = "127.0.0.1") 
{ 
    /*
    ping 一下broker IPC_EXCL 检查broker是否在监听
    */
    int bkey = ftok(broker_host,'a');
    broker_msgid = msgget(bkey,IPC_CREAT | IPC_EXCL | 0666);
    // if(broker_msgid >= 0 ) 
    if(false)
    {
       
        /*broker尚未监听，释放资源*/
        destroymsg(broker_msgid);
        printf("broker is sleep!!!,plaese try again later!!\n");
        return BROKERER;
    }
    broker_msgid = msgget(bkey,IPC_CREAT | 0666);
    /*
    检测消息是否可用
    */
    key = ftok(publisher_host,'a');
    msgid  = msgget(key,IPC_CREAT | IPC_EXCL | 0666);
    if(msgid < 0) {
        printf("msgget faill!!\n");
        return MSGEXIST;
    }
    /*
    要向broker注册消息所有权
    */
    MsgData request;
    RuquestHead head;
    /*合成并发送一个注册报文*/
    getHead(head,NULL,publisher_host,REGISTERED,0,0);
    printf("get head!!\n");
    memset(request.mtext,0,sizeof(request.mtext));
    request.mtype = 1;
    memcpy(request.mtext,&head,sizeof(head));
    msgsnd(broker_msgid,(void*)&request,sizeof(request.mtext),0);

    printf("broker msgid == %d\n",broker_msgid);

    /**
     * 接收返回消息 这里可能要注册一个定时器打断
     */
    memset(request.mtext,0,sizeof(request.mtext));
    memset(&head,0,sizeof(head));
    head.ID = -1;
    msgrcv(msgid,(void*)&request,sizeof(request.mtext),request.mtype,0);
    printf("get!!\n");
    memcpy(&head,request.mtext,sizeof(head));

    if(head.ID == -1) 
    {
        /*被拒绝 释放消息*/
        destroymsg(msgid);
        printf("broker reject!!!,plaese try again later!!\n");
        return BROKREJET;
    }
    if(!Create()) 
    {
        /*释放消息*/
        // exit();
        destroymsg(msgid);
        return CREATETHREADER;
    }
    return SUCCEED;
}
/**
 * @brief 创建publiser线程
 * 
 * @return true 成功
 * @return false 失败
 */
bool Publisher::Create() 
{
    if( 0 > pthread_create(&send_threadid,NULL,Pusher,this)) 
    {
        return false;
    }
    return true;
}



/**
 * @brief 在接收线程中执行的函数 一直在接收
 *
 * 未实现
 */
void Consumer::revcRun()
{

}

/**
 * @brief 接收 msg_type 消息类型到msgrevdQue中阻塞
 * 
 * 未实现，未使用
 * @param msg_type
 * @param buf
 */
void Consumer::pToMsgrcvQ(long msg_type,MsgData buf)
{
    memset(buf.mtext,0,sizeof(buf.mtext));
    msgrcv(msgid,(void*)&buf,sizeof(buf.mtext),msg_type,0);

    rev_lock.lock();
    msgrevdQue.push(buf);
    rev_sem.post();
    rev_lock.unlock();

    return ;
}
/**
 * @brief 从接收队列头部取出消息
 * 未实现未使用
 * @param block
 * @param buf
 * @return true
 * @return false
 */
bool Consumer::Revc(int block,MsgData&buf)
{
    // rev_lock.lock();
    // if(msgrevdQue.empty()) {
    //     rev_lock.unlock();
    //     return false;
    // }
    // buf = msgrevdQue.front();
    // msgrevdQue.pop();

    // rev_lock.unlock();
    return true;
}

/**
 * @brief 在这里应该要向broker申请一个Consumer 并申请感兴趣的publisher
 * 
 * @return true 
 * @return false 
 */
msgErrno Consumer::init(char* publisher_host,char* consumer_host,char *broker_host = "127.0.0.1") 
{
    /*
    ping一下 publisher是否可用
    */
    int pkey = ftok(publisher_host,'a');
    int pmsgid  = msgget(pkey,IPC_CREAT | IPC_EXCL | 0666);
    if(pmsgid >= 0) 
    {
        destroymsg(pmsgid);
        printf("publisher sleep!!\n");
        return PUBLISHERER;
    }
    /*
    ping 一下broker IPC_EXCL 检查broker是否在监听
    */
    int bkey = ftok(broker_host,'a');
    broker_msgid = msgget(bkey,IPC_CREAT | IPC_EXCL | 0666);
    if(broker_msgid >= 0) 
    {
        /*broker尚未监听，释放资源*/
        destroymsg(broker_msgid);
        printf("broker is sleep!!!,plaese try again later!!\n");
        return BROKERER;
    }
    broker_msgid = msgget(bkey,IPC_CREAT  | 0666);
    /*创建consumer 消息通道*/
    key = ftok(consumer_host,'a');
    msgid = msgget(key,IPC_CREAT| IPC_EXCL | 0666);
    if(msgid < 0) 
    {
        printf("conmer megget error!!\n");
    }
     /*
    要向broker注册消息所有权
    */
    MsgData request;
    RuquestHead head;
    /*合成一个注册报文*/
    getHead(head,consumer_host,publisher_host,REGISTERED,1,0);
    request.mtype = 1;
    memset(request.mtext,0,sizeof(request.mtext));
    memcpy(request.mtext,&head,sizeof(head));
    printf("brokermsgid == %d\n",broker_msgid);
    msgsnd(broker_msgid,(void*)&request,sizeof(request.mtext),0);
    // for(int i = 0;i < sizeof(head);i++)printf("%c",request.mtext[i]);
    // printf("\n");
    /**
     * 接收返回消息 这里可能要注册一个定时器打断
     */
    memset(request.mtext,0,sizeof(request.mtext));
    memset(&head,0,sizeof(head));
    head.ID = -1;
    msgrcv(msgid,(void*)&request,sizeof(request.mtext),request.mtype,0);
    memcpy(&head,request.mtext,sizeof(head));

    if(head.ID == -1) 
    {
        /*被拒绝 释放消息*/
        destroymsg(msgid);
        printf("broker reject!!!,plaese try again later!!\n");
        return BROKREJET;
    }
    return SUCCEED;
}
/**
 * @brief 申请一个消息
 * 
 * @return true 
 * @return false 
 */
msgErrno Consumer::getMsg(MsgData&request) 
{
    RuquestHead head;
    memset(&head,0,sizeof(head));
    /*发送一个请求报文*/
    getHead(head,"127.0.0.3","127.0.0.2",REQUESTmsg,1,0);
    request.mtype = 1;
    memset(request.mtext,0,sizeof(request.mtext));
    memcpy(request.mtext,&head,sizeof(head));
    msgsnd(broker_msgid,(void*)&request,sizeof(request.mtext),0);
    // printf("wait msg!!\n");

    /*接收消息*/
    memset(request.mtext,0,sizeof(request.mtext));
    memset(&head,0,sizeof(head));
    head.ID = -1;
    msgrcv(msgid,(void*)&request,sizeof(request.mtext),request.mtype,0);

    takeData(request);
}

/**
 * @brief 解析出消息
 * 
 */
void Consumer::takeData(MsgData&buf)
{
    memset(data,0,sizeof(data));
    memcpy(data,buf.mtext,sizeof(buf.mtext));
}







/**
 * @brief 创建接收线程 创建请求线程
 * 
 */
void Broker::init()
{
    if(0 > pthread_create(&requestPth,NULL,requesTask,this)) 
    {
        printf("create requestask error!!!\n");
    }
    if(0 > pthread_create(&recvPth,NULL,recvTask,this)) 
    {
        printf("create recvTask error!!!\n");
    }
}
/**
 * @brief request线程回调函数
 * 
 * @param argv 
 * @return void* 
 */
void* Broker::requesTask(void*argv) 
{
    Broker* t = (Broker*)argv;
    t->requesRun();
}
/**
 * @brief recv线程回调函数
 * 
 * @param argv 
 * @return void* 
 */
void* Broker::recvTask(void*argv)
{
    Broker * t = (Broker*)argv;
    t->recvRun();
}
/**
 * @brief 循环接收请求报文 阻塞
 * 
 */
void Broker::requesRun() 
{
    
    
    RuquestHead head;
    MsgData request;
    while(1) 
    {
        /*设置环境*/
        memset(&request.mtext,0,sizeof(request.mtext));
        memset(&head,0,sizeof(head));
        request.mtype = 1;

        msgrcv(msgid,(void*)&request,sizeof(request.mtext),request.mtype,0);

        /*解报*/
        memcpy(&head,request.mtext,sizeof(head));
        memset(&request.mtext,0,sizeof(request.mtext));
        uint32_t consumer = head.chost,publisher = head.phost;
        /*consumer请求消息*/
        if(head.op == REQUESTmsg)
        {   /*坏请求，暂时不理睬 后续增加报文拒绝功能*/
            if(head.chost == NULL)continue;

            /*临界区*/
            qsem.wait();
            qlock.lock();
            while(PublisherQ.empty()) 
            {
                /*返回空消息*/
                memset(&request.mtext,0,sizeof(request.mtext));
                msgsnd(msgidD[consumer],&request,sizeof(request.mtext),0);
                qlock.unlock();
                continue;
            }
            request = PublisherQ[publisher].front();
            request.mtype = 1;
            PublisherQ[publisher].pop();
            qlock.unlock();

            /*返回消息*/
            msgsnd(msgidD[consumer],&request,sizeof(request.mtext),0);

        }/*注册请求*/ 
        else if(head.op == REGISTERED)
        {
            /*consumer*/
            if(head.who == 1) 
            {
                /*解析host,转成char以获取key*/
                char host[20];
                memset(host,0,sizeof(host));
                /*测试发现我机器是小端的 要反过来*/
                sprintf(host,"%d.%d.%d.%d\0",
                            (consumer )   &0x000000ff,
                            (consumer>>8) &0x000000ff,
                            (consumer>>16)&0x000000ff,
                            (consumer>>24)&0x000000ff);

                int t = ftok(host,'a');
                int id = msgget(t,0666|IPC_CREAT);
                printf("%s id == %d\n",host,id);
                if(id < 0) 
                {
                    printf("consumer error!! t == %d\n",id);
                }
                else 
                {
                    msgidD[consumer] = id;
                    memset(&head,0,sizeof(head) );
                    head.ID = 1;
                    memset(&request,0,sizeof(request));
                    request.mtype = 1;
                    memcpy(request.mtext,&head,sizeof(head));
                    msgsnd(id,(void*)&request,sizeof(request.mtext),0);
                    printf("send!!\n");
                }
            }
            /*publisher*/
            else if(head.who == 0)
            {
                /*解析host,转成char以获取key*/
                char host[20];
                memset(host,0,sizeof(host));
                /*测试发现我机器是小端的 要反过来*/
                sprintf(host,"%d.%d.%d.%d\0",
                            (publisher    )&0x000000ff,
                            (publisher>>8 )&0x000000ff,
                            (publisher>>16)&0x000000ff,
                            (publisher>>24)&0x000000ff);


                int t = ftok(host,'a');printf("%s\n",host);
                int id = msgget(t,0666);
                if(id < 0) 
                {
                    printf("publisher error!!\n");
                }
                else 
                {
                    printf("get publisher id ==%d\n",id);
                    msgidD[publisher] = id;
                    memset(&head,0,sizeof(head) );
                    head.ID = 1;
                    memset(&request,0,sizeof(request));
                    request.mtype = 1;
                    memcpy(request.mtext,&head,sizeof(head));
                    msgsnd(id,(void*)&request,sizeof(request.mtext),0);
                    printf("send!!\n");
                }
            }
        /*ACK报文*/
        }else if(head.op == ACK) 
        {
        
        /*未知错误*/
        }else {

        }
    }
}
/**
 * @brief 循环接收publisher的推送 阻塞
 * 
 */
void Broker::recvRun()
{
    MsgData request;
    uint32_t publisher = inet_addr("127.0.0.2");
    while(1)
    {
        while(msgidD.find(publisher) == msgidD.end())
        {
            continue;
        }
        msgrcv(msgidD[publisher],(void*)&request,sizeof(request.mtext),request.mtype,0);

        qlock.lock();
        {
            PublisherQ[publisher].push(request);
           // printf("%s\n",request.mtext);
            qsem.post();
        }
        qlock.unlock();

        memset(request.mtext,0,sizeof(request.mtext));
    }
}
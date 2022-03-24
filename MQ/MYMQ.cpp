#include"MYMQ.h"


MBlock_Queue::MBlock_Queue(char * cc,int msize = 10000) {
        mQ.resize(msize);
        BITS.resize(msize,ACK);
        m_size = msize;
        GET_INDEX  = -1;
        SEND_INDEX = -1;
        ACK_INDEX  = -1;


        key = ftok(cc,'a');
        if(( msgid = msgget(key,IPC_CREAT | 0666)) < 0) {
            printf("msg create error!!\n");
        }

        // printf("msgid ==  %d \n",msgid);
        //定时器+信号初始化
        //信号
        sa.sa_flags = SA_SIGINFO;
        sa.sa_sigaction = MBlock_Queue::SigHandler;
        sigemptyset(&sa.sa_mask);

        if(sigaction(SIGALRM,&sa,NULL) == -1) {
            printf("signalactionerror!!!\n");
        }
        //设置环境
        sev.sigev_notify = SIGEV_SIGNAL;
        sev.sigev_signo = SIGALRM;
        sev.sigev_value.sival_ptr = this;

        // sigemptyset(&mask);
        // sigaddset( &mask,SIGALRM);
        // sigprocmask(SIG_SETMASK,&mask,NULL);//屏蔽给 所有msak信号

        //定时器
        its.it_value.tv_sec = time_out / 10;
        its.it_value.tv_nsec = time_out % 1000000000;
        its.it_interval.tv_nsec = its.it_value.tv_nsec;
        its.it_interval.tv_sec = its.it_value.tv_sec;

        if(timer_create(CLOCK_REALTIME,&sev,&timerid) == -1) {
            printf("timer create error!! %d \n",errno);
        }

        if(timer_settime(timerid,0,&its,NULL) == -1){
            printf("settime error!!\n");
        }
}
//获取消息
bool MBlock_Queue::getMQ(msgbuf1&value) {
     if(cur_size >= m_size) {
            return false;
        }
        q_lock.lock();
        int GET_INDEX = (GET_INDEX + 1)%m_size;
        while(BITS[GET_INDEX] != ACK) {
            //找到下一个可用的接收位置
            GET_INDEX = (GET_INDEX + 1)%m_size;
        }
        mQ[GET_INDEX] = value;//放入该位置
        BITS[GET_INDEX] = GET;//别忘了改变状态，状态改变为 以收取
        cur_size++;
        q_lock.unlock();
        return true;
}
//发送消息
bool MBlock_Queue::sendMQ(msgbuf1&value,int client,int &index) {
    if(cur_size  - ack_size <= 0 || cur_size <= 0) {// && ack_size <= 0
            //无消息可发送
        return false;
    }

    q_lock.lock();
    SEND_INDEX = (SEND_INDEX + 1)%m_size;
    while(BITS[SEND_INDEX] != GET) {
        printf("no msg to send!!\n");
        SEND_INDEX = (SEND_INDEX + 1)%m_size;
    }
    value = mQ[SEND_INDEX];
    mQ[SEND_INDEX].mtype = client;
    BITS[SEND_INDEX] = SEND;//状态改为 已发出
    cur_size--;
    ack_size++;
    index = SEND_INDEX;
    //加入最小根堆
    reSend_Times t;
    t.index = SEND_INDEX;
    t.times = 0;
    t.clientID = client;//标记发给谁的
    time_t cur = time(NULL) + timeout;
    timer_Q.push(make_pair(cur,t));
    printf("send to client %d and push back Q\n",client);
    q_lock.unlock();
    return true;
}
//ack消息
bool MBlock_Queue::ackMQ(int index) {
    q_lock.lock();
    if(BITS[index] != SEND) {
        q_lock.unlock();
        printf("something error!!!\n");
        return false;
    }
    BITS[index] = ACK;
    ack_size--;
    q_lock.unlock();
    return true;
}
//重传消息
void MBlock_Queue::reSend() {
    if(timer_Q.empty()) {
        return ;
    }
    // printf("resend !!!!\n");
    time_t cur = time(NULL);
    vector<pair<time_t,reSend_Times>> timeout_item;
    while(!timer_Q.empty() && timer_Q.top().first < cur) {
        auto c = timer_Q.top().second.index;
        q_lock.lock();
        printf("resending!!!\n");
            //已经ack
        if(BITS[c] == ACK) {
            printf("ooo!!\n");
            timer_Q.pop();
            q_lock.unlock();
            continue;
        }
        q_lock.unlock();
        //重传
        auto item = timer_Q.top();
         //4次以上 失败处理 放入任务队列
        if(item.second.times > 4) {
            q_lock.lock();
            BITS[item.second.index] = GET;
            continue;
            q_lock.unlock();
        }
        timer_Q.pop();
        //重发
        msgsnd(msgid,&mQ[map[item.second.clientID]],sizeof(mQ[map[item.second.clientID]].mtext),IPC_NOWAIT);
        printf("resend id == %d!!!\n",mQ[map[item.second.clientID]].mtype);
        // send(item.second.clientID);
        time_t cur_time = time(NULL);
        item.first = cur_time;
        // timer_Q.push(item);
        timeout_item.push_back(item);
    }
    for(auto &c : timeout_item)timer_Q.push(c);
    return ;
}

/*
报文第一个字节表示连接还是断开

监听连接报文 频道为1

id 为 2的为主站，其余的都为从站
后面后面是地址
0 127.0.0.2\0 主站ID为2加入
1 127.0.0.2\0 主站ID为2退出
*/
//监听新加入的链接
void *MBlock_Queue::Listen(void*arg) {
    printf("Listening !!!\n");
    MBlock_Queue* o = (MBlock_Queue*)arg;
    printf("msgid == %d\n",o->msgid);
    msgbuf1 temp;
    while(1) {
        temp.mtype = 1;
        memset (temp.mtext,0,sizeof(temp.mtext));
        msgrcv(o->msgid,&temp,sizeof(temp.mtext),temp.mtype,0);
        // printf("conming !!!erron ==  %d \n",errno);
        printf("recv == %s\n",temp.mtext);

        // EAGAIN
        char op = temp.mtext[0];
        if(op!='0' && op !='1')continue;
        char * host = temp.mtext+1;
        printf("host len == %d\n",strlen(host));
        int take = 3,id = 0;
        for(int i = 0;i < strlen(host);i++) {
            if(take == 0) {
                id = id*10+host[i] - '0';
            }else if(host[i] == '.')take--;
        }
        memset(temp.mtext,0,sizeof(temp.mtext));
        if(op == '0') {
            if(o->set.find(id) == o->set.end()) {
                o->set.insert(id);
                char* t = "Wellcome add to the channel!!!\n";
                strncpy(temp.mtext,t,strlen(t));
                temp.mtype = id;
                printf("wellcomd to join channel client id == %d!!\n",id);
                msgsnd(o->msgid,(void *)&temp,sizeof(temp.mtext),IPC_NOWAIT);//回执
                printf("send back!!\n");
            }
            else {
                printf("id is exsit!!!\n");
            }
        }else{
            if(o->set.find(id) == o->set.end()) {
                printf("id is invali!!!\n");
                continue;
            }
            o->set.erase(id);
            printf("client id == %d disconnect!!!\n",id);
        }
    }
}


/*
请求频道 255 带上自己的id 

报文第一个字节表示请求发送或请求接收

0  主站ID为1有消息要接收
1 127.0.0.12\0 主站ID为12请求一条消息
2 为ack一条消息
*/
//处理任务 发送和接受
void *MBlock_Queue::Wroker(void*arg) {
    printf("wroking !!!\n");
    MBlock_Queue* o = (MBlock_Queue*)arg;
    msgbuf1 temp;
    while(1) {
        temp.mtype = 255;
        memset (temp.mtext,0,sizeof(temp.mtext));
        int no = msgrcv(o->msgid,(void*)&temp,sizeof(temp.mtext),temp.mtype,IPC_NOWAIT);
        if(no == -1) {
            // printf("no msg sleep!!\n");
            // sleep(2);
            continue;
        }
        char op = temp.mtext[0];
        if(op == '0') {
            //接收消息
            for(int i = 0;i < strlen(temp.mtext) - 1;i++) {
                temp.mtext[i] = temp.mtext[i+1];
            }
            temp.mtext[strlen(temp.mtext) - 1] = '\0';
            printf("get msg!!\n");
            o->getMQ(temp);
        }
        else if( op =='1') {
            char * host = temp.mtext+1;
            int take = 3,id = 0;
            for(int i = 0;i < strlen(host);i++) {
                if(take == 0) {
                    id = id*10+host[i] - '0';
                }else if(host[i] == '.')take--;
            }
            memset(temp.mtext,0,sizeof(temp.mtext));
            //发送消息
            int index = 0;
            if(!o->sendMQ(temp,id,index))continue;
            o->map[id] = index;//
            temp.mtype = id;
            printf("locker!!!\n");
            msgsnd(o->msgid,&temp,sizeof(temp.mtext),IPC_NOWAIT);

        }else if(op == '2') {
            char * host = temp.mtext+1;
            int take = 3,id = 0;
            for(int i = 0;i < strlen(host);i++) {
                if(take == 0) {
                    id = id*10+host[i] - '0';
                }else if(host[i] == '.')take--;
            }
            memset(temp.mtext,0,sizeof(temp.mtext));
            if(o->map.find(id) != o->map.end()) {
                o->ackMQ(o->map[id]);
                printf("ack msg frome %d \n",id);
            }
        }
    }
}

void MBlock_Queue::RUN() {
    pid_t l_id,w_id;
    if(pthread_create(&listen_id,NULL,&Listen,this) < 0) {
        printf("create l_id error!!\n");
    } 
    if(pthread_create(&work_id,NULL,&Wroker,this) < 0) {
        printf("create worker thread error!!\n");
    }
}

int main() {
    MBlock_Queue c("locker.h");
    sleep(1);
    c.RUN();
    while(1);
    return 0;
}
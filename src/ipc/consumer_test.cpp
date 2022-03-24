#include"msg.h"
#include<iostream>
#include<time.h>

using namespace std;

int main()
{
    Consumer *c = new Consumer();

    c->init("127.0.0.2","127.0.0.3","127.0.0.1");
    struct MsgData rev_M;
    memset(&rev_M,0,sizeof(rev_M));
    rev_M.mtype = 1;

    struct BMsgData data_M;
    memset(&data_M,0,sizeof(data_M));



/*1 kw 条消息跑了 65.973265974561s cpu时间 开打印 */
/*1 kw 条消息跑了 23.253923255182s cpu时间 关打印 时间大概40s */

    clock_t start,end;
    start = clock();
    for(int i = 0;i < 10000000;i++)
    {
        c->getMsg(rev_M);
        // printf("%s\n",rev_M.mtext);
        memset(rev_M.mtext,0,sizeof(rev_M));
    }
    end = clock();
    cout<<"cost: "<<(double)(end - start)/CLOCKS_PER_SEC<<end;

    char cc;
    while(cin>>cc) {
        if(cc=='q'||cc=='Q')break;
        sleep(10);
    }
    delete c;
    return 0;
}
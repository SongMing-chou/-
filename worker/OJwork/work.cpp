#include <stdio.h>
#include <stdbool.h>
#include "./source/source.h"
#include<stdlib.h>
#include "./monitor/monitor.h"
#include"../mysemun.h"
#include<time.h>
#include<pthread.h>
#include<queue>
#include"locker.h"


class worker {
    pthread_t make_thread,work_thread;
public: 
    worker() {
        if(pthread_create(&make_thread,NULL,&make,this)<0) {
            printf("create make thread error!!\n");
        }
        if(pthread_create(&make_thread,NULL,&work,this) < 0) {
            printf("create worker thread error!!\n");
        }

    }
    void getAndSetConfig(int argc, char *argv[], struct ExecveConfig *config);
    bool checkConfig(const struct ExecveConfig *const config);


    locker q_lock;

    void init();
    void makeer();
    void worker();
    std::queue<char*>q;

    static void *make(void*argv);
    static void *work(void*argv);
};
#include<time.h>
#include<fcntl.h>
#include<sys/types.h>
#include<unistd.h>
#include<stdlib.h>
#include<stdio.h>
#include<signal.h>
#include<sys/types.h>
#include<errno.h>

 



//类功能，每个对象中内置一个定时器，初始化类之后就一直在定时，定时时间到了就执行某一函数
class a {
   
    timer_t timerid;
    struct sigevent sev;
    struct itimerspec its;

    sigset_t mask;
    struct sigaction sa;

    long long time_out  = 100;

public:
    int num ;
    a(int val):num(val) {
        
        //信号
        sa.sa_flags = SA_SIGINFO;
        sa.sa_sigaction = a::func;
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
        its.it_value.tv_sec = time_out / 100;
        its.it_value.tv_nsec = 10000;
        its.it_interval.tv_nsec = its.it_value.tv_nsec;
        its.it_interval.tv_sec = its.it_value.tv_sec;

        if(timer_create(CLOCK_REALTIME,&sev,&timerid) == -1) {
            printf("timer create error!! %d \n",errno);
        }

        if(timer_settime(timerid,0,&its,NULL) == -1){
            printf("settime error!!\n");
            // sleep(1);
            
        }

    }
    static void func(int sig,siginfo_t*si,void * us) {
         a* temp = (a*) si->_sifields._timer.si_sigval.sival_ptr;
        if(sig == SIGALRM) {
            // sleep(5);
            // printf("alrm!!!! aa == %d\n",temp->num);
            temp->mprintf();
        }else{
            temp->mprintf();
        }
    }
    void mprintf() {
        sleep(1);
        printf("num == %d\n",num);
    }
    
};

//sig_ign 忽略该信号
/*
  } siginfo_t __SI_ALIGNMENT;
 struct sigaction  
 {
  void     (*sa_handler)(int) sa_handler;
  void     (*sa_sigaction)(int, siginfo_t *, void *) __sigaction_handler;
  sigset_t  sa_mask;
  int       sa_flags;
  void     (*sa_restorer)(void);
 };
当 sa_flags 成员的值

包含了 SA_SIGINFO 标志时，系统将使用 sa_sigaction 函数作为信号处理函数，
否则使用 sa_handler 作为信号处理

*/
 int main() {
     a aa(1);
    //  sleep(1);
    //  a bb(2);
     printf("ooo\n");
     while(1){
         printf("ooooo!!!\n");
         sleep(1);
     }
 }
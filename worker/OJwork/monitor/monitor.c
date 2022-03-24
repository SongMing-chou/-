#include<unistd.h>
#include<sys/time.h>
#include<pthread.h>
#include<sys/wait.h>
#include<signal.h>
#include<stdbool.h>
#include<stdio.h>
#include<string.h>
#include"monitor.h"
#include<stdlib.h>

#include"../executant/executant.h"

struct RealTimeKillerConfig {
    pid_t pid;
    unsigned readTimeLimit;
};


//killer函数
void *reallTimeKiller(void *RealTimeKillerConfig);

//根据进程消耗资源，推出情况设置执行结果
static enum RUNNING_CONDITION setRunningCondition(
        const int status,const struct ExecveConfig*,const struct ExecveResult*);

//顾名思义
static unsigned long getMillisecond(const struct timeval val);

// static int k = 0; 
// static long long sum = 0,start = 0,end = 0;

void startMonitor(const struct ExecveConfig*const config,struct ExecveResult*const result) {
    struct timeval startTime, endTime;
    // system();
    if(0 != getuid()) {
        printf("fail ai root user!!\n");
        result->condition = UNABLE_TO_SET_UID;
        return ;
    }

    // 重定向文件
    // redrection(config->stdinPath,config->stdoutPath,config->stderrPath);
    
    gettimeofday(&startTime,NULL);

    pid_t childPid = fork();

    if(childPid < 0) {
        result->condition = UNABLE_TO_EXECVE;
        return;
    }

    if(0 == childPid) {
        startExecutant(config);
    }else{
        pthread_t killerThreadId;
        struct RealTimeKillerConfig realTimeKillerConfig;
        realTimeKillerConfig.pid = childPid;
        realTimeKillerConfig.readTimeLimit = config->realTimeLimit;

        const int ret = pthread_create(&killerThreadId,NULL,reallTimeKiller,(void*)&realTimeKillerConfig);

        if(ret != 0) {
            printf("create killer fail!!\n");
            result->condition = UNABLE_TO_LIMIT_REAL_TIME;
            kill(childPid,SIGKILL);
            return;
        }

        int status;
        struct rusage costs;//统计消耗资源
        wait4(childPid,&status,WSTOPPED,&costs);//用来设置状态
        gettimeofday(&endTime,NULL);
        pthread_cancel(killerThreadId);

        //设置结果
        result->cpuTimeCost = getMillisecond(costs.ru_utime);
        result->realTimeCost = getMillisecond(endTime) - getMillisecond(startTime);
        result->memoryCost = costs.ru_maxrss;
        result->condition = setRunningCondition(status,config,result);
    }
}


void* reallTimeKiller(void*realTimeKillerConfig) {
    struct RealTimeKillerConfig *config = ( struct RealTimeKillerConfig *)realTimeKillerConfig;
    // printf("killer is rungning!!\n");
    sleep(config->readTimeLimit/1000);
    // sleep(10);
    // printf("kill!!!!\n");
    kill(config->pid,SIGKILL);
    return NULL;

}

//换算时间
inline unsigned long getMillisecond(const struct timeval val) {
    return val.tv_sec*1000 + val.tv_usec/1000;
}

// WIFEXITED(status)	
//              若为正常终止子进程返回的状态，则为真
//              对于这种情况可执行WEXITSTATUS(status)取子进程传送给exit、_exit、_Exit参数的低8位
// WEXITSTATUS(status)	如果WIFEXITED返回真，则此宏返回子进程的退出码
// WIFSIGNALED(status)	
//              若为异常终止子进程返回的状态（子进程接到了一个没有捕捉的信号而终止），则为真
//              对于 这种情况，可执行WTERMSIG(status) 取使子进程终止的信号编号
//              另外，有些实现（非Signle UNIX Specification）定义宏WCOREDUMP(status)，若已产生终止进程的core文件，则它返回真
// WTERMSIG(status)	如果WIFSIGNALED返回真，则该宏返回一个信号值
// WIFSTOPPED(status)	
//              若为当前暂停子进程的返回的状态，则为真
//              对于这种情况，可执行WSTOPSIG(status)获取使子进程暂停的信号编号
// WSTOPSIG(status)	如果WIFSTOPPED返回真，则其返回一个信号值
// WIFCONTINUED(status)	若在作业控制暂停后已经继续的子进程返回了状态，则为真（POSIX.1的XSI扩展;仅用于waitpid）

enum RUNNING_CONDITION setRunningCondition(
    const int status,const struct ExecveConfig *config,const struct ExecveResult*result)
{
    if(WIFEXITED(status)) {
        if(0 == WEXITSTATUS(status)) {

            bool isMemoryExceeded = (unsigned long long)(result->memoryCost) > config->memoryLimit;

            if(isMemoryExceeded) {
                return MEMORY_LIMIT_EXCEED;
            }

            //消耗时间
            int isCpuTimeExceeded = config->cpuTimeLimit < result->cpuTimeCost;
            int isRealTimeExceeded = config->realTimeLimit < result->realTimeCost;

            if(isCpuTimeExceeded || isRealTimeExceeded) {
                return TIME_LIMIT_EXCEED;
            }

            return SUCCESS;
        }
        return (enum RUNNING_CONDITION)WEXITSTATUS(status);
    }

    // 异常终止
    if (WIFSIGNALED(status)) {
        if (WTERMSIG(status) == SIGXCPU) {
            return TIME_LIMIT_EXCEED;
        }
        if (WTERMSIG(status) == SIGSEGV) {
            return SEGMENTATION_FAULT;
        }
        if (WTERMSIG(status) == SIGKILL) {
            // 经测试 cpu的时间超限也会出现在此处
            if (config->cpuTimeLimit < result->cpuTimeCost) {
                return TIME_LIMIT_EXCEED;
            }
            return RUNTIME_ERROR;
        }
        if (WTERMSIG(status) == SIGXFSZ) {
            return OUTPUT_LIMIT_EXCEED;
        }
        return RUNTIME_ERROR;
    }
    return UNKNOWN_ERROR;
}
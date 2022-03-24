#include"executant.h"
#include<sys/stat.h>
#include<fcntl.h>
#include<sys/resource.h>
#include<seccomp.h>
#include<unistd.h>
#include<sys/types.h>
#include<stdio.h>
#include<stdlib.h>
#include<arpa/inet.h>
#include<errno.h>


//重定向三大输出文件 标准输入 输出 错误文件
 void redrection(const char* const inputPath,const char*const outputPath,
                        const char*stderrPath);

//内存 时间 输出大小的限制
static void setResourceLimitation(const struct ExecveConfig* const config);

//禁止除读写文件外的系统调用
static void forbidSyscall(const struct ExecveConfig *const config );



//执行代码
void startExecutant(const struct ExecveConfig *const config) {

    //重定向文件
    redrection(config->stdinPath,config->stdoutPath,config->stderrPath);
    // int savefd = open(config->stdoutPath,O_RDWR |  O_TRUNC | O_CREAT | 0666);//, 0666
    // if(savefd <= 0 ) {
    //     printf("open savefd error!!!\n");
    //     return ;
    // }
    // close(savefd);
    //限制资源
    setResourceLimitation(config);

    //设置uid
    if(config->userId != USER_ID_DEFAULT) {
        if(setuid(config->userId) == -1) {
            // printf("set uid error !!!\n");
            _exit(UNABLE_TO_SET_UID);
        }
    }

    //禁止读写外的系统调用
    forbidSyscall(config);

    char *env[] = {"PATH=/bin", NULL};
    execve(config->execvePath, NULL, env);
    _exit(0);
}

//重定向文件 dup函数
void redrection(const char*inputPath,const char* outputPath,const char*stderrPath) {
    //输入文件
    if(NULL != inputPath) {
        const int inputNo = open(inputPath,O_RDONLY);

        if(-1 == inputNo) {
            _exit(UNABLE_TO_GET_INPUT);
        }
        dup2(inputNo,STDIN_FILENO);
    }

    //输出重定向
    if(NULL != outputPath) {
        const int outputNo = open(outputPath,O_WRONLY);
        if(-1 == outputNo) {
            _exit(UNABLE_TO_MAKE_OUTPUT);
        }
        dup2(outputNo,STDOUT_FILENO);
    }

    //error重定向
    if(NULL != stderrPath) {
        const int errputNo = open(stderrPath,O_WRONLY);
        if (-1 == errputNo)
        {
            _exit(UNABLE_TO_MAKE_OUTPUT);
        }
        dup2(errputNo,STDERR_FILENO);
        
    }
}


//限制资源
void setResourceLimitation(const struct ExecveConfig*const config) {
    struct rlimit limit;

    //限制内存（kb->b)
    limit.rlim_cur = config->memoryLimit*1024;
    limit.rlim_max = config->wallMemoryLimit*1024;
    if(0 != setrlimit(RLIMIT_AS,&limit)) {
        _exit(UNABLE_TO_LIMIT_MEM);
    }

    //限制cpu时间(毫秒-》秒)
    limit.rlim_cur = limit.rlim_max = config->cpuTimeLimit/1000 + 1;
    if(0 != setrlimit(RLIMIT_CPU,&limit)) {
        _exit(UNABLE_TO_LIMIT_CPU_TIME);
    }

    //限制输出规模
    limit.rlim_cur = limit.rlim_max = config->outputSizeLimit;
    if(0 != setrlimit(RLIMIT_FSIZE,&limit)) {
        _exit(UNABLE_TO_LIMIT_OUTPUT);
    }

    //堆栈
    limit.rlim_cur = limit.rlim_max = RLIM_INFINITY;
    if(setrlimit(RLIMIT_STACK,&limit)) {
        _exit(UNABLE_TO_LIMIT_STACK);
    }
}

inline void forbidSyscall(const struct ExecveConfig * const config) {
    const static int FROBIDDEN_LIST[] = {
        SCMP_SYS(fork),
        SCMP_SYS(clone),
        SCMP_SYS(vfork)
    };

    //初始化过滤器
    scmp_filter_ctx ctx;
    ctx = seccomp_init(SCMP_ACT_ALLOW);

    if (NULL == ctx) {
        printf("seccomp_init error!!!\n");
        _exit(UNABLE_TO_SECCOMP);
    }

    //添加禁止规则
    const size_t len = sizeof FROBIDDEN_LIST /sizeof(int);
    for(size_t i = 0;i < len;i++) {
        if(0 != seccomp_rule_add(ctx,SCMP_ACT_KILL,FROBIDDEN_LIST[i],0)) {
            printf("seccom_rule_add error rule == %d erron  == %d!!\n",i,errno);
            
            _exit(UNABLE_TO_SECCOMP);
        }
    }

    //仅允许此进程EXECVE提交程序
    const int ret = seccomp_rule_add(ctx,SCMP_ACT_KILL,SCMP_SYS(execve),
            1,SCMP_A0(SCMP_CMP_NE,(scmp_datum_t)(config->execvePath) ));
    
    if(ret < 0) {
        printf("seccomp_rule_add error!! execve error!!\n");
        _exit(UNABLE_TO_SECCOMP);
    }

    seccomp_load(ctx);
}

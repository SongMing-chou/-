#pragma once
#ifndef SOURCH_H
#define SOURCH_H

#include <unistd.h>
#include <sys/resource.h>
#include<stdio.h>


/*运行出错*/
enum RUNNING_CONDITION{
    SUCCESS = 0,                /*运行成功*/
    RUNTIME_ERROR,              /*运行时错误*/
    TIME_LIMIT_EXCEED,          /*超时*/
    MEMORY_LIMIT_EXCEED,        /*爆内存*/
    SEGMENTATION_FAULT,         /*段错误*/
    UNKNOWN_ERROR, //5          /*未知错误*/
    OUTPUT_LIMIT_EXCEED,        /*爆输出*/

    /*初始化出错*/
    UNABLE_TO_GET_INPUT,        /*无法输入*/
    UNABLE_TO_MAKE_OUTPUT,      /*无法输出*/
    UNABLE_TO_EXECVE,           /*无法执行程序*/ 
    UNABLE_TO_SET_UID,//10      /*无法以低权限用户执行*/
    UNABLE_TO_LIMIT_MEM,        /*无法限制内存*/
    UNABLE_TO_LIMIT_CPU_TIME,   /*无法限制cpu时间*/
    UNABLE_TO_LIMIT_OUTPUT,     /*无法限制输出*/
    UNABLE_TO_LIMIT_STACK,      /*无法限制堆栈*/
    UNABLE_TO_SECCOMP,  //15    /*无法限制系统调用*/
    UNABLE_TO_LIMIT_REAL_TIME,  /*无法限制真实时间*/
    CONFIG_ERROR                /*设置错误*/
};


/*超资源 出错*/
enum LIMIT_DEFAULT{
    CPU_TIME_LIMIT_DEFAULT = 4000,          /*4000ms*/
    REAL_TIME_LIMIE_DEFAULT = 10000,        /*10000ms*/
    MEMORY_LIMIT_DEFAULT = 1024*64,         /*64MiB*/
    WALL_MEMORY_LIMIT_DEFAULT = 1024*64*4,  /*256MiB*/
    OUTPUT_SIZE_LIMIT_DEFAULT = 1024*512,   /*512KiB*/
    USER_ID_DEFAULT = -1                    /*root*/
};

/*
 RLIMIT_AS：进程虚拟内存（地址空间，Address Space）的最大字节长度。该限制会影响brk、mmap和mremap等。
 RLIMIT_CORE：core文件的最大字节长度。超出这个大小的core文件会被截短。指定0则表示不产生core文件。
 RLIMIT_CPU：CPU时间的使用限制（秒）。进程达到软限制时，会收到一个SIGXCPU信号（默认会终止进程。但进程可以捕获该信号）。
如果进程继续消耗CPU时间，它会每秒收到一个SIGXCPU信号，直到达到硬限制，并接收到SIGKILL信号（不同的实现在此处可能会有差别）。
 RLIMIT_DATA：进程数据段（初始化数据节、未初始化数据节和堆）的最大字节长度。该限制会影响brk和sbrk等。
 RLIMIT_FSIZE：进程所能创建的文件的最大字节长度。
 RLIMIT_LOCKS：进程可创建的flock锁和fcntl租借锁的总数（租借锁是Linux特有的：fcntl可通过F_SETLEASE命令对文件加读或写的租借锁。
当另一个进程尝试打开或截短该文件而产生冲突时，内核会通过信号通知持有租借锁的进程。后者应当对此作出响应，如flush缓冲区或移除租借锁等）。
 RLIMIT_MEMLOCK：进程使用mlock能够锁定在RAM中的最大字节长度（防止被换出到交换分区。内存的锁定和解锁以页为单位）。该限制会影响mlock、mlockall和mmap等。
 RLIMIT_MSGQUEUE：调用进程的实际用户所能分配的Posix消息队列的最大字节长度。
 RLIMIT_NOFILE：进程所能打开（如使用open/pipe/socket）的文件描述符的最大值加1。注意，进程间的文件描述符是独立的。超出该限制会抛出EMFILE错误。
 RLIMIT_NPROC：调用进程的实际用户所能创建进程（在Linux上，更准确的说法是线程）的最大数目。超出该限制时，fork会失败并抛出EAGAIN错误。
 RLIMIT_STACK：进程的栈的最大字节长度。超出该限制会收到SIGSEGV信号。

struct rlimit {
    rlim_t rlim_cur;  // 软限制。任何进程可将它设置为0~rlim_max
    rlim_t rlim_max;  // 硬限制（软限制的上限）
};

*/
struct ExecveConfig
{
    rlim_t cpuTimeLimit;        /*单位ms*/
    rlim_t realTimeLimit;       /*单位ms*/
    rlim_t memoryLimit;         /*单位kB*/
    rlim_t wallMemoryLimit;     /*单位kB*/
    rlim_t outputSizeLimit;     /*单位kB*/
    uid_t userId;               /*以该userid对应用户执行提交程序*/
    char *execvePath;           //
    char *stdinPath;            //
    char *stdoutPath;           //
    char *stderrPath;           //
};

//执行消耗
struct ExecveResult{
    rlim_t cpuTimeCost;                 /*单位毫秒*/
    rlim_t realTimeCost;                /*单位毫秒*/
    rlim_t memoryCost;                  /*单位kB*/
    enum RUNNING_CONDITION condition;   /**/
};

void initializeConfig(struct ExecveConfig *const config);
void initializeResult(struct ExecveResult *const result);
#endif
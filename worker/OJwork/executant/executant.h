#include"../source/source.h"
#include<linux/seccomp.h>


//开始执行
void startExecutant(const struct ExecveConfig* const config);

//重定向三大输出文件 标准输入 输出 错误文件
 void redrection(const char* const inputPath,const char*const outputPath,
                        const char*stderrPath);